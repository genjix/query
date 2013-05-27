#include "node_impl.hpp"

#include <future>

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

void output_to_file(std::ofstream& file, log_level level,
    const std::string& domain, const std::string& body)
{
    if (body.empty())
        return;
    file << level_repr(level);
    if (!domain.empty())
        file << " [" << domain << "]";
    file << ": " << body << std::endl;
}
void output_cerr_and_file(std::ofstream& file, log_level level,
    const std::string& domain, const std::string& body)
{
    if (body.empty())
        return;
    std::ostringstream output;
    output << level_repr(level);
    if (!domain.empty())
        output << " [" << domain << "]";
    output << ": " << body;
    std::cerr << output.str() << std::endl;
    file << output.str() << std::endl;
}

node_impl::node_impl()
  : network_pool_(1), disk_pool_(6), mem_pool_(1), publish_pool_(2),
    hosts_(network_pool_),
    handshake_(network_pool_),
    network_(network_pool_),
    protocol_(network_pool_, hosts_, handshake_, network_),
    chain_(disk_pool_),
    poller_(mem_pool_, chain_),
    txpool_(mem_pool_, chain_),
    session_(mem_pool_, {
        handshake_, protocol_, chain_, poller_, txpool_})
{
}

bool node_impl::start(config_map_type& config)
{
    outfile_.open(config["output-file"]);
    errfile_.open(config["error-file"].c_str());
    log_debug().set_output_function(
        std::bind(output_to_file, std::ref(outfile_), _1, _2, _3));
    log_info().set_output_function(
        std::bind(output_to_file, std::ref(outfile_), _1, _2, _3));
    log_warning().set_output_function(
        std::bind(output_to_file, std::ref(errfile_), _1, _2, _3));
    log_error().set_output_function(
        std::bind(output_cerr_and_file, std::ref(errfile_), _1, _2, _3));
    log_fatal().set_output_function(
        std::bind(output_cerr_and_file, std::ref(errfile_), _1, _2, _3));
    protocol_.subscribe_channel(
        std::bind(&node_impl::monitor_tx, this, _1));
    // Start blockchain.
    std::promise<std::error_code> ec_chain;
    auto blockchain_started =
        [&](const std::error_code& ec)
        {
            ec_chain.set_value(ec);
        };
    chain_.start(config["database"], blockchain_started);
    // Query the error_code and wait for startup completion.
    std::error_code ec = ec_chain.get_future().get();
    if (ec)
    {
        log_error() << "Couldn't start blockchain: " << ec.message();
        return false;
    }
    // Transaction pool
    txpool_.start();
    // Start session
    std::promise<std::error_code> ec_session;
    auto session_started =
        [&](const std::error_code& ec)
        {
            ec_session.set_value(ec);
        };
    session_.start(session_started);
    // Query the error_code and wait for startup completion.
    ec = ec_session.get_future().get();
    if (ec)
    {
        log_error() << "Unable to start session: " << ec.message();
        return false;
    }
    // Ready to begin publishing new blocks and txs.
    publish_.start(config);
    chain_.subscribe_reorganize(
        std::bind(&node_impl::reorganize,
            this, _1, _2, _3, _4));
    return true;
}

bool node_impl::stop()
{
    std::promise<std::error_code> ec_promise;
    auto session_stop =
        [&](const std::error_code& ec)
        {
            ec_promise.set_value(ec);
        };
    session_.stop(session_stop);
    std::error_code ec = ec_promise.get_future().get();
    if (ec)
    {
        log_error() << "Problem stopping session: " << ec.message();
        return false;
    }
    network_pool_.stop();
    disk_pool_.stop();
    mem_pool_.stop();
    network_pool_.join();
    disk_pool_.join();
    mem_pool_.join();
    chain_.stop();
    return true;
}

blockchain& node_impl::blockchain()
{
    return chain_;
}
transaction_pool& node_impl::transaction_pool()
{
    return txpool_;
}
protocol& node_impl::protocol()
{
    return protocol_;
}

void node_impl::reorganize(const std::error_code& ec,
    size_t fork_point,
    const bc::blockchain::block_list& new_blocks,
    const bc::blockchain::block_list& replaced_blocks)
{
    // Don't bother publishing blocks when in the initial blockchain download.
    if (fork_point > 235866)
        for (size_t i = 0; i < new_blocks.size(); ++i)
        {
            size_t depth = fork_point + i + 1;
            const block_type& blk = *new_blocks[i];
            publish_pool_.service().post(
                std::bind(&publisher::send_blk, &publish_, depth, blk));
        }
    chain_.subscribe_reorganize(
        std::bind(&node_impl::reorganize,
            this, _1, _2, _3, _4));
}

void node_impl::monitor_tx(channel_ptr node)
{
    node->subscribe_transaction(
        std::bind(&node_impl::recv_transaction, this, _1, _2, node));
    protocol_.subscribe_channel(
        std::bind(&node_impl::monitor_tx, this, _1));
}

void node_impl::recv_transaction(const std::error_code& ec,
    const transaction_type& tx, channel_ptr node)
{
    if (ec)
    {
        log_error() << "recv_transaction: " << ec.message();
        return;
    }
    auto handle_confirm = [](const std::error_code& ec)
        {
            if (ec)
                log_warning() << "Confirm error: " << ec.message();
        };
    txpool_.store(tx, handle_confirm,
        std::bind(&node_impl::handle_mempool_store, this, _1, _2, tx, node));
    node->subscribe_transaction(
        std::bind(&node_impl::recv_transaction, this, _1, _2, node));
}

void node_impl::handle_mempool_store(
    const std::error_code& ec, const index_list& unconfirmed,
    const transaction_type& tx, channel_ptr node)
{
    log_info() << "Accepted transaction: " << hash_transaction(tx);
    publish_pool_.service().post(
        std::bind(&publisher::send_tx, &publish_, tx));
}


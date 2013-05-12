#include "node_impl.hpp"

#include <future>

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

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
}

node_impl::node_impl()
  : network_pool_(1), disk_pool_(1), mem_pool_(1),
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
    //protocol_.subscribe_channel(monitor_tx);
    // Start blockchain.
    std::promise<std::error_code> ec_chain;
    auto blockchain_started =
        [&](const std::error_code& ec)
        {
            ec_chain.set_value(ec);
        };
    chain_.start(config["database"], blockchain_started);
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
    // Query the error_codes and wait for startup completion.
    std::error_code ec = ec_chain.get_future().get();
    if (ec)
    {
        log_error() << "Couldn't start blockchain: " << ec.message();
        return false;
    }
    ec = ec_session.get_future().get();
    if (ec)
    {
        log_error() << "Unable to start session: " << ec.message();
        return false;
    }
    publish_.start(config);
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


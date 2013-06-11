#ifndef QUERY_NODE_IMPL_HPP
#define QUERY_NODE_IMPL_HPP

#include <bitcoin/bitcoin.hpp>

#include "config.hpp"
#include "publisher.hpp"

class node_impl
{
public:
    node_impl();
    bool start(config_map_type& config);
    bool stop();

    bc::blockchain& blockchain();
    bc::transaction_pool& transaction_pool();
    bc::protocol& protocol();

private:
    void reorganize(const std::error_code& ec,
        size_t fork_point,
        const bc::blockchain::block_list& new_blocks,
        const bc::blockchain::block_list& replaced_blocks);

    void monitor_tx(const std::error_code& ec, bc::channel_ptr node);
    void recv_transaction(const std::error_code& ec,
        const bc::transaction_type& tx, bc::channel_ptr node);
    void handle_mempool_store(
        const std::error_code& ec, const bc::index_list& unconfirmed,
        const bc::transaction_type& tx, bc::channel_ptr node);

    std::ofstream outfile_, errfile_;
    bc::threadpool network_pool_, disk_pool_, mem_pool_, publish_pool_;
    // Services
    bc::hosts hosts_;
    bc::handshake handshake_;
    bc::network network_;
    bc::protocol protocol_;
    bc::leveldb_blockchain chain_;
    bc::poller poller_;
    bc::transaction_pool txpool_;
    bc::session session_;
    // Publisher
    publisher publish_;
};

#endif


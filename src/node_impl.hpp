#ifndef QUERY_NODE_IMPL_HPP
#define QUERY_NODE_IMPL_HPP

#include <bitcoin/bitcoin.hpp>

#include "config.hpp"
#include "publisher.hpp"

class node_impl
{
public:
    node_impl(config_map_type& config);
    bool start();
    bool stop();

    bc::blockchain& blockchain();
    bc::transaction_pool& transaction_pool();
    bc::protocol& protocol();

private:
    std::ofstream outfile_, errfile_;
    bc::threadpool network_pool_, disk_pool_, mem_pool_;
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


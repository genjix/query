#ifndef QUERY_PUBLISHER_HPP
#define QUERY_PUBLISHER_HPP

#include <zmq.hpp>
#include <bitcoin/bitcoin.hpp>

#include "config.hpp"

class publisher
{
public:
    publisher();
    void start(config_map_type& config);
    bool send(uint32_t depth, const bc::block_type& blk);
    bool send(const bc::transaction_type& tx);

private:
    zmq::context_t context_;
    zmq::socket_t socket_block_, socket_tx_;
};

#endif


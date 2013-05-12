#include "publisher.hpp"

publisher::publisher(config_map_type& config)
  : context_(1),
    socket_block_(context_, ZMQ_PUB), socket_tx_(context_, ZMQ_PUB)
{
    std::string bind_addr = "tcp://*:";
    socket_block_.bind((bind_addr + config["block pub port"]).c_str());
    socket_tx_.bind((bind_addr + config["tx pub port"]).c_str());
}

bool send_raw(const bc::data_chunk& raw,
    zmq::socket_t& socket, bool send_more=false)
{
    zmq::message_t message(raw.size());
    memcpy(message.data(), raw.data(), raw.size());
    return socket.send(message, send_more ? ZMQ_PUB : 0);
}

bool publisher::send(uint32_t depth, const bc::block_type& blk)
{
    bc::data_chunk raw_depth = bc::uncast_type(depth);
    BITCOIN_ASSERT(raw_depth.size() == 4);
    if (!send_raw(raw_depth, socket_block_, true))
        return false;
    bc::data_chunk raw_block(bc::satoshi_raw_size(blk));
    bc::satoshi_save(blk, raw_block.begin());
    return send_raw(raw_block, socket_block_);
}

bool publisher::send(const bc::transaction_type& tx)
{
    bc::data_chunk raw_tx(bc::satoshi_raw_size(tx));
    bc::satoshi_save(tx, raw_tx.begin());
    return send_raw(raw_tx, socket_tx_);
}


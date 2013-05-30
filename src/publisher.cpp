#include "publisher.hpp"

#define LOG_PUBLISHER "publisher"

publisher::publisher()
  : context_(1),
    socket_block_(context_, ZMQ_PUB), socket_tx_(context_, ZMQ_PUB)
{
}

void publisher::start(config_map_type& config)
{
    std::string bind_addr = "tcp://*:";
    socket_block_.bind((bind_addr + config["block-publish-port"]).c_str());
    socket_tx_.bind((bind_addr + config["tx-publish-port"]).c_str());
}

bool send_raw(const bc::data_chunk& raw,
    zmq::socket_t& socket, bool send_more=false)
{
    zmq::message_t message(raw.size());
    memcpy(message.data(), raw.data(), raw.size());
    return socket.send(message, send_more ? ZMQ_SNDMORE : 0);
}

bool publisher::send_blk(uint32_t depth, const bc::block_type& blk)
{
    bc::data_chunk raw_depth = bc::uncast_type(depth);
    BITCOIN_ASSERT(raw_depth.size() == 4);
    bool success = send_raw(raw_depth, socket_block_, true);
    if (!success)
        bc::log_warning(LOG_PUBLISHER) << "Problem publishing block depth.";
    bc::data_chunk raw_block(bc::satoshi_raw_size(blk));
    bc::satoshi_save(blk, raw_block.begin());
    if (!send_raw(raw_block, socket_block_))
    {
        bc::log_warning(LOG_PUBLISHER) << "Problem publishing block data.";
        return false;
    }
    return true;
}

bool publisher::send_tx(const bc::transaction_type& tx)
{
    bc::data_chunk raw_tx(bc::satoshi_raw_size(tx));
    bc::satoshi_save(tx, raw_tx.begin());
    if (!send_raw(raw_tx, socket_tx_))
        bc::log_warning(LOG_PUBLISHER) << "Problem publishing tx data.";
}


#ifndef QUERY_SERVICE_HPP
#define QUERY_SERVICE_HPP

#include <bitcoin/bitcoin.hpp>

#include "thrift/QueryService.h"
#include "node_impl.hpp"
#include "sync_blockchain.hpp"
#include "sync_transaction_pool.hpp"

class query_service_handler
  : public QueryServiceIf
{
public:
    typedef std::function<void ()> stop_function_type;

    query_service_handler(config_map_type& config, node_impl& node);
    bool stopped() const;
    void wait();

    bool stop(const std::string& secret);
    // blockchain methods
    void block_header_by_depth(BlockHeader& blk, const int32_t depth);
    void block_header_by_hash(BlockHeader& blk, const std::string& hash);
    void block_transaction_hashes_by_depth(
        HashList& tx_hashes, const int32_t depth);
    void block_transaction_hashes_by_hash(
        HashList& tx_hashes, const std::string& hash);
    int32_t block_depth(const std::string& hash);
    int32_t last_depth();
    void transaction(Transaction& tx, const std::string& hash);
    void transaction_index(
        TransactionIndex& tx_index, const std::string& hash);
    void spend(InputPoint& inpoint, const OutputPoint& outpoint);
    void outputs(OutputPointList& outpoints, const std::string& address);
    // blockchain (composed) methods
    void history(History& history, const std::string& address);
    void output_values(OutputValues& values, const OutputPointList& outpoints);
    // transaction pool methods
    void transaction_pool_transaction(
        Transaction& tx, const std::string& hash);
    // protocol methods
    bool broadcast_transaction(const std::string& tx_data);

private:
    sync_blockchain chain_;
    sync_transaction_pool txpool_;
    bc::protocol& protocol_;
    const std::string stop_secret_;
    bool stopped_ = false;
};

void start_thrift_server(config_map_type& config, node_impl& node);

#endif


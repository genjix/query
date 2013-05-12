#ifndef QUERY_SYNC_TRANSACTION_POOL_HPP
#define QUERY_SYNC_TRANSACTION_POOL_HPP

#include <bitcoin/bitcoin.hpp>

class sync_transaction_pool
{
public:
    sync_transaction_pool(bc::transaction_pool& txpool);
    bc::transaction_type get(
        const bc::hash_digest& tx_hash, std::error_code& ec) const;
private:
    bc::transaction_pool& txpool_;
};

#endif


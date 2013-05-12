#include "sync_transaction_pool.hpp"

#include "sync_get_impl.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

sync_transaction_pool::sync_transaction_pool(bc::transaction_pool& txpool)
  : txpool_(txpool)
{
}

bc::transaction_type sync_transaction_pool::get(
    const bc::hash_digest& tx_hash, std::error_code& ec) const
{
    return sync_get_impl<bc::transaction_type>(
        std::bind(&bc::transaction_pool::fetch, &txpool_, _1, _2),
        tx_hash, ec);
}


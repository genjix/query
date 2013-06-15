#include "sync_blockchain.hpp"

#include <future>

#include "sync_get_impl.hpp"

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

sync_blockchain::sync_blockchain(blockchain& chain)
  : chain_(chain)
{
}

template <typename IndexType>
block_type block_header_impl(blockchain& chain,
    IndexType index, std::error_code& ec)
{
    block_type block_header;
    std::promise<bool> promise;
    auto handle_block_header =
        [&ec, &block_header, &promise]
            (const std::error_code& cec, const block_type& blk)
        {
            ec = cec;
            block_header = blk;
            promise.set_value(true);
        };
    chain.fetch_block_header(index, handle_block_header);
    bool success = promise.get_future().get();
    BITCOIN_ASSERT(success);
    return block_header;
}

block_type sync_blockchain::block_header(size_t depth) const
{
    std::error_code discard_ec;
    return block_header(depth, discard_ec);
}
block_type sync_blockchain::block_header(size_t depth,
    std::error_code& ec) const
{
    return block_header_impl(chain_, depth, ec);
}

block_type sync_blockchain::block_header(
    const hash_digest& block_hash) const
{
    std::error_code discard_ec;
    return block_header(block_hash, discard_ec);
}
block_type sync_blockchain::block_header(const hash_digest& block_hash,
    std::error_code& ec) const
{
    return block_header_impl(chain_, block_hash, ec);
}

template <typename IndexType>
inventory_list block_tx_hashes_impl(blockchain& chain,
    IndexType index, std::error_code& ec)
{
    inventory_list hashes;
    std::promise<bool> promise;
    auto handle_tx_hashes =
        [&ec, &hashes, &promise]
            (const std::error_code& cec, const inventory_list& chashes)
        {
            ec = cec;
            hashes = chashes;
            promise.set_value(true);
        };
    chain.fetch_block_transaction_hashes(index, handle_tx_hashes);
    bool success = promise.get_future().get();
    BITCOIN_ASSERT(success);
    return hashes;
}

inventory_list sync_blockchain::block_transaction_hashes(
    size_t depth) const
{
    std::error_code discard_ec;
    return block_transaction_hashes(depth, discard_ec);
}
inventory_list sync_blockchain::block_transaction_hashes(
    size_t depth, std::error_code& ec) const
{
    return block_tx_hashes_impl(chain_, depth, ec);
}

inventory_list sync_blockchain::block_transaction_hashes(
    const hash_digest& block_hash) const
{
    std::error_code discard_ec;
    return block_transaction_hashes(block_hash, discard_ec);
}
inventory_list sync_blockchain::block_transaction_hashes(
    const hash_digest& block_hash, std::error_code& ec) const
{
    return block_tx_hashes_impl(chain_, block_hash, ec);
}

size_t sync_blockchain::block_depth(const hash_digest& block_hash) const
{
    std::error_code discard_ec;
    return block_depth(block_hash, discard_ec);
}
size_t sync_blockchain::block_depth(const hash_digest& block_hash,
    std::error_code& ec) const
{
    return sync_get_impl<size_t>(
        std::bind(&blockchain::fetch_block_depth, &chain_, _1, _2),
        block_hash, ec);
}

size_t sync_blockchain::last_depth() const
{
    std::error_code discard_ec;
    return last_depth(discard_ec);
}
size_t sync_blockchain::last_depth(std::error_code& ec) const
{
    // We discard the index since it isn't used for fetching the last depth.
    // sync_get_impl expects an index value so we give it a value to discard.
    return sync_get_impl<size_t>(
        std::bind(&blockchain::fetch_last_depth, &chain_, _2),
        0, ec);
}

transaction_type sync_blockchain::transaction(
    const hash_digest& transaction_hash) const
{
    std::error_code discard_ec;
    return transaction(transaction_hash, discard_ec);
}
transaction_type sync_blockchain::transaction(
    const hash_digest& transaction_hash, std::error_code& ec) const
{
    return sync_get_impl<transaction_type>(
        std::bind(&blockchain::fetch_transaction, &chain_, _1, _2),
        transaction_hash, ec);
}

transaction_index_t sync_blockchain::transaction_index(
    const hash_digest& transaction_hash) const
{
    std::error_code discard_ec;
    return transaction_index(transaction_hash, discard_ec);
}
transaction_index_t sync_blockchain::transaction_index(
    const hash_digest& transaction_hash, std::error_code& ec) const
{
    transaction_index_t tx_index;
    std::promise<bool> promise;
    auto handle_tx_index =
        [&ec, &tx_index, &promise]
            (const std::error_code& cec, size_t depth, size_t offset)
        {
            ec = cec;
            tx_index = {depth, offset};
            promise.set_value(true);
        };
    chain_.fetch_transaction_index(transaction_hash, handle_tx_index);
    bool success = promise.get_future().get();
    BITCOIN_ASSERT(success);
    return tx_index;
}

input_point sync_blockchain::spend(
    const output_point& outpoint) const
{
    std::error_code discard_ec;
    return spend(outpoint, discard_ec);
}
input_point sync_blockchain::spend(
    const output_point& outpoint, std::error_code& ec) const
{
    return sync_get_impl<input_point>(
        std::bind(&blockchain::fetch_spend, &chain_, _1, _2),
        outpoint, ec);
}

output_point_list sync_blockchain::outputs(
    const payment_address& address) const
{
    std::error_code discard_ec;
    return outputs(address, discard_ec);
}
output_point_list sync_blockchain::outputs(
    const payment_address& address, std::error_code& ec) const
{
    return sync_get_impl<output_point_list>(
        std::bind(&blockchain::fetch_outputs, &chain_, _1, _2),
        address, ec);
}

history_t sync_blockchain::history(
    const bc::payment_address& address) const
{
    std::error_code discard_ec;
    return history(address, discard_ec);
}
history_t sync_blockchain::history(
    const bc::payment_address& address, std::error_code& ec) const
{
    history_t history;
    std::promise<bool> promise;
    auto handle_history =
        [&ec, &history, &promise]
            (const std::error_code& cec,
             const output_point_list& outpoints,
             const input_point_list& inpoints)
        {
            ec = cec;
            history.outpoints = outpoints;
            history.inpoints = inpoints;
            promise.set_value(true);
        };
    fetch_history(chain_, address, handle_history);
    bool success = promise.get_future().get();
    BITCOIN_ASSERT(success);
    return history;
}

output_value_list sync_blockchain::output_values(
    const output_point_list& outpoints) const
{
    std::error_code discard_ec;
    return output_values(outpoints, discard_ec);
}
output_value_list sync_blockchain::output_values(
    const output_point_list& outpoints, std::error_code& ec) const
{
    return sync_get_impl<output_value_list>(
        std::bind(&fetch_output_values, std::ref(chain_), _1, _2),
        outpoints, ec);
}


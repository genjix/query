#ifndef QUERY_SYNC_BLOCKCHAIN_H
#define QUERY_SYNC_BLOCKCHAIN_H

#include <bitcoin/bitcoin.hpp>

struct transaction_index_t
{
    size_t depth, offset;
};

class sync_blockchain
{
public:
    sync_blockchain(bc::blockchain& chain);

    bc::block_type block_header(size_t depth) const;
    bc::block_type block_header(size_t depth, std::error_code& ec) const;

    bc::block_type block_header(const bc::hash_digest& block_hash) const;
    bc::block_type block_header(const bc::hash_digest& block_hash,
        std::error_code& ec) const;

    bc::inventory_list block_transaction_hashes(
        size_t depth) const;
    bc::inventory_list block_transaction_hashes(
        size_t depth, std::error_code& ec) const;

    bc::inventory_list block_transaction_hashes(
        const bc::hash_digest& block_hash) const;
    bc::inventory_list block_transaction_hashes(
        const bc::hash_digest& block_hash, std::error_code& ec) const;

    size_t block_depth(const bc::hash_digest& block_hash) const;
    size_t block_depth(const bc::hash_digest& block_hash,
        std::error_code& ec) const;

    size_t last_depth() const;
    size_t last_depth(std::error_code& ec) const;

    bc::transaction_type transaction(
        const bc::hash_digest& transaction_hash) const;
    bc::transaction_type transaction(
        const bc::hash_digest& transaction_hash, std::error_code& ec) const;

    transaction_index_t transaction_index(
        const bc::hash_digest& transaction_hash) const;
    transaction_index_t transaction_index(
        const bc::hash_digest& transaction_hash, std::error_code& ec) const;

    bc::input_point spend(
        const bc::output_point& outpoint) const;
    bc::input_point spend(
        const bc::output_point& outpoint, std::error_code& ec) const;

    bc::output_point_list outputs(
        const bc::payment_address& address) const;
    bc::output_point_list outputs(
        const bc::payment_address& address, std::error_code& ec) const;
private:
    bc::blockchain& chain_;
};

#endif


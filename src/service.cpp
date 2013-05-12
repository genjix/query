#include "service.hpp"

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "echo.hpp"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace bc;

template <typename T>
std::string to_binary(const T& bytes)
{
    std::string binary;
    extend_data(binary, bytes);
    return binary;
}

query_service_handler::query_service_handler(node_impl& node)
  : chain_(node.blockchain())
{
}

void query_service_handler::initialize(stop_function_type stop_function)
{
    stop_function_ = stop_function;
}

void query_service_handler::stop()
{
    echo() << "Stopping server...";
    stop_function_();
}

void check_errc(const std::error_code& ec)
{
    if (ec)
    {
        ErrorCode except;
        except.what = 0;
        except.why = ec.message();
        throw except;
    }
}

template<typename IndexType>
void block_header_impl(sync_blockchain& chain,
    BlockHeader& blk, IndexType index)
{
    std::error_code ec;
    auto b = chain.block_header(index, ec);
    check_errc(ec);
    blk.version = b.version;
    blk.timestamp = b.timestamp;
    blk.previous_block_hash = to_binary(b.previous_block_hash);
    blk.merkle = to_binary(b.merkle);
    blk.bits = b.bits;
    blk.nonce = b.nonce;
}

void query_service_handler::block_header_by_depth(
    BlockHeader& blk, const int32_t depth)
{
    block_header_impl(chain_, blk, depth);
}

hash_digest proper_hash(const std::string& hash_str)
{
    hash_digest hash;
    if (hash_str.size() != hash.size())
    {
        ErrorCode except;
        except.what = 0;
        except.why = "Invalid hash";
        throw except;
    }
    std::copy(hash_str.begin(), hash_str.end(), hash.begin());
    return hash;
}

void query_service_handler::block_header_by_hash(
    BlockHeader& blk, const std::string& hash)
{
    block_header_impl(chain_, blk, proper_hash(hash));
}

template <typename IndexType>
void block_tx_hashes_impl(
    sync_blockchain& chain, HashList& tx_hashes, IndexType index)
{
    std::error_code ec;
    auto txs = chain.block_transaction_hashes(index, ec);
    check_errc(ec);
    for (const auto& inv: txs)
    {
        BITCOIN_ASSERT(inv.type == inventory_type_id::transaction);
        tx_hashes.push_back(to_binary(inv.hash));
    }
}

void query_service_handler::block_transaction_hashes_by_depth(
    HashList& tx_hashes, const int32_t depth)
{
    block_tx_hashes_impl(chain_, tx_hashes, depth);
}

void query_service_handler::block_transaction_hashes_by_hash(
    HashList& tx_hashes, const std::string& hash)
{
    block_tx_hashes_impl(chain_, tx_hashes, proper_hash(hash));
}

int32_t query_service_handler::block_depth(const std::string& hash)
{
    std::error_code ec;
    auto depth = chain_.block_depth(proper_hash(hash), ec);
    check_errc(ec);
    return depth;
}

int32_t query_service_handler::last_depth()
{
    std::error_code ec;
    auto depth = chain_.last_depth(ec);
    check_errc(ec);
    return depth;
}

void query_service_handler::transaction(
    Transaction& tx, const std::string& hash)
{
    std::error_code ec;
    auto tmp_tx = chain_.transaction(proper_hash(hash), ec);
    check_errc(ec);
    tx.version = tmp_tx.version;
    tx.locktime = tmp_tx.locktime;
    for (const auto& tx_input: tmp_tx.inputs)
    {
        TransactionInput in;
        in.previous_output.hash = to_binary(tx_input.previous_output.hash);
        in.previous_output.index = tx_input.previous_output.index;
        in.input_script = to_binary(save_script(tx_input.input_script));
        in.sequence = tx_input.sequence;
        tx.inputs.push_back(in);
    }
    for (const auto& tx_output: tmp_tx.outputs)
    {
        TransactionOutput out;
        out.value = tx_output.value;
        out.output_script = to_binary(save_script(tx_output.output_script));
        tx.outputs.push_back(out);
    }
}

void query_service_handler::transaction_index(
    TransactionIndex& tx_index, const std::string& hash)
{
    std::error_code ec;
    auto tidx = chain_.transaction_index(proper_hash(hash), ec);
    check_errc(ec);
    tx_index.depth = tidx.depth;
    tx_index.offset = tidx.offset;
}

void query_service_handler::spend(
    InputPoint& inpoint, const OutputPoint& outpoint)
{
    std::error_code ec;
    output_point out{
        proper_hash(outpoint.hash), (uint32_t)outpoint.index};
    auto ipt = chain_.spend(out, ec);
    check_errc(ec);
    inpoint.hash = to_binary(ipt.hash);
    inpoint.index = ipt.index;
}

void query_service_handler::outputs(
    OutputPointList& outpoints, const std::string& address)
{
    std::error_code ec;
    auto outs = chain_.outputs(address, ec);
    check_errc(ec);
    for (const auto& out: outs)
    {
        OutputPoint outpoint;
        outpoint.hash = to_binary(out.hash);
        outpoint.index = out.index;
        outpoints.push_back(outpoint);
    }
}

void start_thrift_server(node_impl& node)
{
    boost::shared_ptr<TProtocolFactory> protocol_factory(
        new TBinaryProtocolFactory());
    boost::shared_ptr<query_service_handler> handler(
        new query_service_handler(node));
    boost::shared_ptr<TProcessor> processor(
        new QueryServiceProcessor(handler));
    boost::shared_ptr<TServerTransport> server_transport(
        new TServerSocket(9090));
    boost::shared_ptr<TTransportFactory> transport_factory(
        new TBufferedTransportFactory());

    boost::shared_ptr<ThreadManager> thread_manager =
        ThreadManager::newSimpleThreadManager(10);
    boost::shared_ptr<PosixThreadFactory> thread_factory =
        boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    thread_manager->threadFactory(thread_factory);
    thread_manager->start();
    TThreadPoolServer server(processor, server_transport,
        transport_factory, protocol_factory, thread_manager);
    handler->initialize(
        std::bind(&TThreadPoolServer::stop, &server));

    echo() << "Starting server...";
    server.serve();
}


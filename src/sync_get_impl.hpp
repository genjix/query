#ifndef QUERY_SYNC_GET_IMPL_HPP
#define QUERY_SYNC_GET_IMPL_HPP

#include <future>
#include <system_error>

template<typename ReturnType, typename FetchFunc, typename IndexType>
ReturnType sync_get_impl(FetchFunc fetch,
    IndexType index, std::error_code& ec)
{
    ReturnType obj;
    std::promise<bool> promise;
    auto handle =
        [&ec, &obj, &promise]
            (const std::error_code& cec, const ReturnType& cobj)
        {
            ec = cec;
            obj = cobj;
            promise.set_value(true);
        };
    fetch(index, handle);
    bool success = promise.get_future().get();
    BITCOIN_ASSERT(success);
    return obj;
}

#endif


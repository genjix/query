#ifndef QUERY_SYNC_GET_IMPL_HPP
#define QUERY_SYNC_GET_IMPL_HPP

#include <future>
#include <system_error>

template<typename ReturnType, typename FetchFunc, typename IndexType>
ReturnType sync_get_impl(FetchFunc fetch,
    IndexType index, std::error_code& ec)
{
    std::promise<std::error_code> ec_promise;
    std::promise<ReturnType> obj_promise;
    auto handle =
        [&ec_promise, &obj_promise]
            (const std::error_code& ec, const ReturnType& obj)
        {
            ec_promise.set_value(ec);
            obj_promise.set_value(obj);
        };
    fetch(index, handle);
    ec = ec_promise.get_future().get();
    return obj_promise.get_future().get();
}

#endif


#pragma once

#include "Base/Utils/StaticFor.h"
#include "Base/Async/Async.h"

namespace punk
{
    template <size_t I, size_t N, typename F>
    auto async_static_for(F&& f) -> async_simple::coro::Lazy<>
    {
        static_assert(I <= N);
        if constexpr (I < N)
        {
            co_await std::forward<F>(f).template operator()<I>();
            co_await async_static_for<I + 1, N>(std::forward<F>(f));
        }
        co_return;
    }
}
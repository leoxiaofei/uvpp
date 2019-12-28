#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp {
class Async : public handle<uv_async_t>
{
public:
    Async(Loop &l, Callback callback): handle<uv_async_t>(), loop_(l.get())
    {
        init(callback);
    }

    Async(Callback callback): handle<uv_async_t>(), loop_(uv_default_loop())
    {
        init(callback);
    }

    Error send()
    {
        return Error(uv_async_send(get()));
    }

private:

    Error init(Callback callback)
    {
        callbacks::store(get()->data, internal::uv_cid_async, callback);

        return Error(uv_async_init(loop_, get(), [](uv_async_t* handle)
        {
            callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_async);
        }));
    }

    uv_loop_t *loop_;

};
}
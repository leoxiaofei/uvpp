#pragma once

#include "handle.hpp"
#include "error.hpp"

namespace uvpp {

class Poll : public handle<uv_poll_t>
{
public:
    Poll(int fd):
        handle<uv_poll_t>()
    {
        uv_poll_init(uv_default_loop(), get(),fd);
    }

    Poll(Loop& l, int fd):
        handle<uv_poll_t>()
    {
        uv_poll_init(l.get(), get(), fd);
    }

    Error start( int events, std::function<void(int status,int events)> callback)
    {
        callbacks::store(get()->data, internal::uv_cid_poll, callback);
        return Error(uv_poll_start(get(), events,
                                   [](uv_poll_t* handle, int status, int events)
        {
            callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_poll, status, events);
        }
                                  ));
    }

    Error stop()
    {
        return Error(uv_poll_stop(get()));
    }
};
}
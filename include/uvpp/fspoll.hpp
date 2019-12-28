#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "file.hpp"

namespace uvpp {

class FsPoll : public handle<uv_fs_poll_t>
{
public:
    FsPoll():
        handle<uv_fs_poll_t>()
    {
        uv_fs_poll_init(uv_default_loop(), get());
    }

    FsPoll(Loop& l):
        handle<uv_fs_poll_t>()
    {
        uv_fs_poll_init(l.get(), get());
    }

    Error start(const std::string &path, unsigned int interval, std::function<void(Error err,int status,Stats prev,Stats current)> callback)
    {

        callbacks::store(get()->data, internal::uv_cid_fs_poll, callback);

        return Error(uv_fs_poll_start(get(),
                                      [](uv_fs_poll_t* handle,  int status, const uv_stat_t* prev, const uv_stat_t* curr)
        {
            Stats back,current;
            if (status<0)
                callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_fs_poll, Error(status), status, back, current);
            else
            {
                back = statsFromUV(prev);
                current = statsFromUV(curr);
                callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_fs_poll, Error(0), status, back, current);
            }
        }, path.c_str(), interval
                                     ));
    }

    Error stop()
    {
        return Error(uv_fs_poll_stop(get()));
    }
};
}
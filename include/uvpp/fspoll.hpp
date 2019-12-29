#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "file.hpp"

namespace uvpp {

class FsPoll : public Handle<uv_fs_poll_t>
{
public:
    FsPoll():
        Handle<uv_fs_poll_t>()
    {
        uv_fs_poll_init(uv_default_loop(), get());
    }

    FsPoll(Loop& l):
        Handle<uv_fs_poll_t>()
    {
        uv_fs_poll_init(l.get(), get());
    }

    Result start(const std::string &path, unsigned int interval, std::function<void(Result err,int status,Stats prev,Stats current)> callback)
    {

        callbacks::store(get()->data, internal::uv_cid_fs_poll, callback);

        return Result(uv_fs_poll_start(get(),
                                      [](uv_fs_poll_t* handle,  int status, const uv_stat_t* prev, const uv_stat_t* curr)
        {
            Stats back,current;
            if (status<0)
                callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_fs_poll, Result(status), status, back, current);
            else
            {
                back = statsFromUV(prev);
                current = statsFromUV(curr);
                callbacks::invoke<decltype(callback)>(handle->data, internal::uv_cid_fs_poll, Result(0), status, back, current);
            }
        }, path.c_str(), interval
                                     ));
    }

    Result stop()
    {
        return Result(uv_fs_poll_stop(get()));
    }
};
}
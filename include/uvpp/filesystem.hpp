#pragma once

#include "loop.hpp"
#include "request.hpp"

namespace uvpp
{

class FileSystem
{
    uv_loop_t* loop;
public:
    FileSystem()
        : loop(uv_default_loop())
    {
    }

    FileSystem(Loop& l)
        : loop(l.get())
    {
    }

    Result stat(const char* path, const CallbackWithFs& cb_fs)
    {
        return Result(uv_fs_stat(loop, NewReq<Fs>(cb_fs), path, Fs::fs_cb);
    }
};
 
} // namespace uvpp

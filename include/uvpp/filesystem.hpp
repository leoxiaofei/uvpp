#pragma once

#include "loop.hpp"

namespace uvpp
{

class FileSystem
{
    uv_loop_t* loop;
public:
    FileSystem(Loop& l)
        : loop(l.get())
    {
    }

    Result stat(const char* path, const CallbackWithFs& cb_fs)
    {
        return Result(uv_fs_stat(loop, NewReq<Fs>(cb_fs), path, Fs::fs_cb));
    }

	Result mkdir(const char* path, int mode, const CallbackWithFs& cb_fs)
	{
        return Result(uv_fs_mkdir(loop, NewReq<Fs>(cb_fs), path, mode, Fs::fs_cb));
	}
};
 
} // namespace uvpp

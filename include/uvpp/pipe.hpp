#pragma once

#include "stream.hpp"
#include "loop.hpp"

namespace uvpp {
class Pipe : public Stream<Pipe, uv_pipe_t>
{
public:
    Pipe(const bool fd_pass = false)
		: Stream()
    {
        uv_pipe_init(uv_default_loop(), get(), fd_pass ? 1 : 0);
    }

    Pipe(Loop& l, const bool fd_pass = false)
		: Stream()
    {
        uv_pipe_init(l.get(), get(), fd_pass ? 1 : 0);
    }

    Result bind(const std::string& name)
    {
		return Result(uv_pipe_bind(get(), name.c_str()));
    }

    void connect(const std::string& name, const CallbackWithResult& cb_connect)
    {
		uv_pipe_connect(NewReq<Connect>(cb_connect), get(),
			name.c_str(), &Connect::connect_cb);
    }

    bool getsockname(std::string& name)
    {
        std::vector<char> buf(100);
        size_t buf_size = buf.size();
        int r = uv_pipe_getsockname(get(), buf.data(), &buf_size);
        if (r == UV_ENOBUFS) {
            buf.resize(buf_size);
            r = uv_pipe_getsockname(get(), buf.data(), &buf_size);
        }

        if (r == 0)
        {
            name = std::string(buf.data(), buf_size);
            return true;
        }
        return false;
    }

    bool getpeername(std::string& name)
    {
        std::vector<char> buf(100);
        size_t buf_size = buf.size();
        int r = uv_pipe_getpeername(get(), buf.data(), &buf_size);
        if (r == UV_ENOBUFS) {
            buf.resize(buf_size);
            r = uv_pipe_getpeername(get(), buf.data(), &buf_size);
        }

        if (r == 0)
        {
            name = std::string(buf.data(), buf_size);
            return true;
        }
        return false;
    }

    int pending_count()
    {
        return uv_pipe_pending_count(get());
    }

	void pending_instances(int count)
	{
		uv_pipe_pending_instances(get(), count);
	}

    uv_handle_type pending_type()
    {
        return uv_pipe_pending_type(get());
    }
};
}

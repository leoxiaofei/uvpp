#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp {
class Async : public Handle<Async, uv_async_t>
{
	Callback m_cb_async;
public:
    Async(Loop &l, const Callback& cb_async)
    {
		m_cb_async = cb_async;
		uv_async_init(l.get(), get(), INVOKE_HD_CB(m_cb_async));
    }

    Async(const Callback& cb_async)
    {
		m_cb_async = cb_async;
		uv_async_init(uv_default_loop(), get(), INVOKE_HD_CB(m_cb_async));
    }

    Result send()
    {
        return Result(uv_async_send(get()));
    }

private:

};

}
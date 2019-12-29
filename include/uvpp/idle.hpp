#pragma once

#include "loop.hpp"
#include "handle.hpp"

namespace uvpp {
class Idle : public Handle<Idle, uv_idle_t>
{
public:
	Callback m_cb_idle;
public:
    Idle()
		: Handle<Idle, uv_idle_t>()
    {
		uv_idle_init(uv_default_loop(), get());
    }

    Idle(Loop& l, Callback callback)
		: Handle<Idle, uv_idle_t>()
    {
		uv_idle_init(l.get(), get());
	}

    Result start(const Callback& cb_idle)
    {
		m_cb_idle = cb_idle;
        return Result(uv_idle_start(get(), INVOKE_HD_CB(m_cb_idle)));
    }

    Result stop()
    {
        return Result(uv_idle_stop(get()));
    }

private:

};
}




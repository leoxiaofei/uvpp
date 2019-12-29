#pragma once

#include "handle.hpp"
#include "error.hpp"
#include <chrono>

namespace uvpp {
	class Timer : public Handle<Timer, uv_timer_t>
	{
		Callback m_cb_timer;
	public:
		Timer()
		{
			uv_timer_init(uv_default_loop(), get());
		}

		Timer(Loop& l)
		{
			uv_timer_init(l.get(), get());
		}

		Result start(const Callback& cb_timer,
			const uint64_t &timeout, const uint64_t &repeat = 0)
		{
			m_cb_timer = cb_timer;
			return Result(uv_timer_start(get(), INVOKE_HD_CB(m_cb_timer), timeout, repeat));
		}

		Result start(const uint64_t &timeout, const uint64_t &repeat = 0)
		{
			return Result(uv_timer_start(get(), INVOKE_HD_CB(m_cb_timer), timeout, repeat));
		}

		Result stop()
		{
			return Result(uv_timer_stop(get()));
		}

		Result again()
		{
			return Result(uv_timer_again(get()));
		}
	};
}
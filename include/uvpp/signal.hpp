#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp {

	class Signal : public Handle<Signal, uv_signal_t>
	{
	public:
		typedef std::function<void(int sugnum)> CallbackWithSignal;

		CallbackWithSignal m_cb_signal;

	public:
		Signal()
		{
			uv_signal_init(uv_default_loop(), get());
		}

		Signal(Loop& l)
		{
			uv_signal_init(l.get(), get());
		}

		Result start(int signum, const CallbackWithSignal& cb_signal)
		{
			m_cb_signal = cb_signal;

			return Result(uv_signal_start(get(), [](uv_signal_t* handle, int signum)
			{
				if (Signal::self(handle)->m_cb_signal)
				{
					Signal::self(handle)->m_cb_signal(signum);
				}
			}, signum));
		}

		Result stop()
		{
			return Result(uv_signal_stop(get()));
		}
	};
}

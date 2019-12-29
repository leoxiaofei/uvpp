#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"

namespace uvpp {

	typedef std::function<void(int status, int events)> CallbackWithEvents;

	class Poll : public Handle<Poll, uv_poll_t>
	{
	public:
		CallbackWithEvents m_cb_poll;
	public:
		Poll(int fd)
			: Handle<Poll, uv_poll_t>()
		{
			uv_poll_init(uv_default_loop(), get(), fd);
		}

		Poll(Loop& l, int fd)
			: Handle<Poll, uv_poll_t>()
		{
			uv_poll_init(l.get(), get(), fd);
		}

		Result start(int events, const CallbackWithEvents& cb_poll)
		{
			m_cb_poll = cb_poll;
			return Result(uv_poll_start(get(), events,
				[](uv_poll_t* handle, int status, int events)
			{
				if (Poll::self(handle)->m_cb_poll)
				{
					Poll::self(handle)->m_cb_poll(status, events);
				}
			} ));
		}

		Result stop()
		{
			return Result(uv_poll_stop(get()));
		}
	};
}
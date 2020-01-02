#pragma once

#include <uv.h>
#include <functional>

namespace uvpp
{
	typedef std::function<void()> Callback;

	class Thread
	{
		uv_thread_t m_thd_id;
		Callback m_cb_run;
	public:
		Thread(const Callback& cb_run)
		{
			m_cb_run = cb_run;
			uv_thread_create(&m_thd_id, Thread::run, this);
		}

		~Thread()
		{

		}

		int join()
		{
			return uv_thread_join(&m_thd_id);
		}

		uv_thread_t id() const
		{
			return m_thd_id;
		}

		static uv_thread_t currentThreadId()
		{
			return uv_thread_self();
		}

	protected:
		static void run(void* data)
		{
			static_cast<Thread*>(data)->m_cb_run();
		}
	};
}

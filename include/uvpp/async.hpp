#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "loop.hpp"
#include <queue>
#include <mutex>

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

	Async(Loop &l)
	{
		uv_async_init(l.get(), get(), INVOKE_HD_CB(m_cb_async));
	}

    Async(const Callback& cb_async)
    {
		m_cb_async = cb_async;
		uv_async_init(uv_default_loop(), get(), INVOKE_HD_CB(m_cb_async));
    }

	void async_cb(const Callback& cb_async)
	{
		m_cb_async = cb_async;
	}

    Result send()
    {
        return Result(uv_async_send(get()));
    }

private:

};


class AsyncQueue : public Handle<AsyncQueue, uv_async_t>
{
	std::queue<Callback> m_cb_queue;
	std::mutex m_mx_queue;
public:
	AsyncQueue(Loop &l)
	{
		uv_async_init(l.get(), get(), AsyncQueue::cb_async);
	}

	void queue(Callback&& cb)
	{
		std::lock_guard<std::mutex> locker(m_mx_queue);
		m_cb_queue.push(std::move(cb));
		send();
	}

	void queue(const Callback& cb)
	{
		std::lock_guard<std::mutex> locker(m_mx_queue);
		m_cb_queue.push(cb);
		send();
	}

	static void cb_async(uv_async_t* handle)
	{
		SELF::self(handle)->run();
	}


protected:
	void run()
	{
		m_cb_queue.front()();
			
		std::lock_guard<std::mutex> locker(m_mx_queue);
		m_cb_queue.pop();
		if (!m_cb_queue.empty())
		{
			send();
		}
	}

	Result send()
	{
		return Result(uv_async_send(get()));
	}

};
}
#pragma once

#include "error.hpp"
#include "request.hpp"

#include <functional>
#include <memory>

namespace uvpp {
/**
 *  Class that represents the uv_loop instance.
 */
class Loop
{
	using Deleter = void(*)(uv_loop_t *);
    std::unique_ptr<uv_loop_t, Deleter> m_uv_loop;
public:
    /**
     *  Default constructor
     */
    Loop()
		: m_uv_loop(new uv_loop_t, 
			[](uv_loop_t* p) { delete p; })
    {
        if (uv_loop_init(get()))
        {
            throw std::runtime_error("uv_loop_init error");
        }

		m_uv_loop->data = this;
    }

	Loop(std::unique_ptr<uv_loop_t, Deleter>&& uv_loop)
		: m_uv_loop(std::move(uv_loop))
	{
		m_uv_loop->data = this;
	}

    /**
     *  Destructor
     */
    ~Loop()
    {
        if (m_uv_loop)
        {
            uv_loop_close(get());
        }
    }

    Loop(const Loop&) = delete;
    Loop& operator=(const Loop&) = delete;

    /**
     *  Returns internal handle for libuv functions.
     */
    inline uv_loop_t* get()
    {
        return m_uv_loop.get();
    }

    /**
     *  Starts the loop.
     */
    bool run()
    {
        return uv_run(get(), UV_RUN_DEFAULT) == 0;
    }

    /**
     *  Polls for new events once. Blocks if there are no pending events.
     */
    bool run_once()
    {
        return uv_run(get(), UV_RUN_ONCE) == 0;
    }

    /**
     *  Polls for new events once without blocking.
     */
    bool run_nowait()
    {
        return uv_run(get(), UV_RUN_NOWAIT) == 0;
    }

    /**
     *  ...
     *  Internally, this function just calls uv_update_time() function.
     */
    void update_time()
    {
        uv_update_time(get());
    }

    /**
     *  ...
     *  Internally, this function just calls uv_now() function.
     */
    int64_t now()
    {
        return uv_now(get());
    }

    /**
     * Stops the loop
     */
    void stop()
    {
        uv_stop(get());
    }

	void addRef()
	{
		++m_uv_loop->active_handles;
	}

	Result queue_work(const Callback& cb_work)
	{
		return Result(uv_queue_work(get(), NewReq<Work>(cb_work),
			Work::work_cb, Work::after_work_cb));
	}

	Result queue_work(const Callback& cb_work,
		const CallbackWithResult& cb_after_work)
	{
		return Result(uv_queue_work(get(), NewReq<Work>(cb_work, cb_after_work),
			Work::work_cb, Work::after_work_cb));
	}

	static std::shared_ptr<Loop> getDefault()
	{
		static std::weak_ptr<Loop> ref;

		std::shared_ptr<Loop> loop;

		if (ref.expired()) 
		{
			if (uv_loop_t* def = uv_default_loop())
			{
				auto ptr = std::unique_ptr<uv_loop_t, Deleter>(def, [](uv_loop_t *) {});
				loop = std::make_shared<Loop>(std::move(ptr));
				ref = loop;
			}
		}
		else
		{
			loop = ref.lock();
		}

		return loop;
	}
};

}

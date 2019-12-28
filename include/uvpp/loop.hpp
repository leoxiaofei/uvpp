#pragma once

#include "error.hpp"

#include <functional>
#include <memory>

namespace uvpp {
/**
 *  Class that represents the uv_loop instance.
 */
class Loop
{
public:
    /**
     *  Default constructor
     *  @param use_default indicates whether to use default loop or create a new loop.
     */
    Loop(bool use_default=false)
        : m_uv_loop(use_default ? uv_default_loop() : new uv_loop_t,
			[use_default](uv_loop_t *loop) { if (!use_default) delete loop; })
    {
        if (!use_default && uv_loop_init(m_uv_loop.get()))
        {
            throw std::runtime_error("uv_loop_init error");
        }
    }

    /**
     *  Destructor
     */
    ~Loop()
    {
        if (m_uv_loop.get())
        {
            // no matter default loop or not: http://nikhilm.github.io/uvbook/basics.html#event-loops
            uv_loop_close(m_uv_loop.get());
        }
    }

    Loop(const Loop&) = delete;
    Loop& operator=(const Loop&) = delete;
    Loop(Loop&& other)
        : m_uv_loop(std::forward<decltype(other.m_uv_loop)>(other.m_uv_loop))
    {

    }

    Loop& operator=(Loop&& other)
    {
        if (this != &other)
        {
            m_uv_loop = std::forward<decltype(other.m_uv_loop)>(other.m_uv_loop);
        }
        return *this;
    }

    /**
     *  Returns internal handle for libuv functions.
     */
    uv_loop_t* get()
    {
        return m_uv_loop.get();
    }

    /**
     *  Starts the loop.
     */
    bool run()
    {
        return uv_run(m_uv_loop.get(), UV_RUN_DEFAULT) == 0;
    }

    /**
     *  Polls for new events once. Blocks if there are no pending events.
     */
    bool run_once()
    {
        return uv_run(m_uv_loop.get(), UV_RUN_ONCE) == 0;
    }

    /**
     *  Polls for new events once without blocking.
     */
    bool run_nowait()
    {
        return uv_run(m_uv_loop.get(), UV_RUN_NOWAIT) == 0;
    }

    /**
     *  ...
     *  Internally, this function just calls uv_update_time() function.
     */
    void update_time()
    {
        uv_update_time(m_uv_loop.get());
    }

    /**
     *  ...
     *  Internally, this function just calls uv_now() function.
     */
    int64_t now()
    {
        return uv_now(m_uv_loop.get());
    }

    /**
     * Stops the loop
     */
    void stop()
    {
        uv_stop(m_uv_loop.get());
    }

private:

    // Custom deleter
    typedef std::function<void(uv_loop_t*)> Deleter;

    std::unique_ptr<uv_loop_t, Deleter> m_uv_loop;
};

/**
 *  Starts the default loop.
 */
inline int run()
{
    return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

/**
 *  Polls for new events for the default loop once.
 *  Blocks only if there are no pending events.
 */
inline int run_once()
{
    return uv_run(uv_default_loop(), UV_RUN_ONCE);
}

/**
 *  Polls for new events for the default loop once.
 *  Non-blocking.
 */
inline int run_nowait()
{
    return uv_run(uv_default_loop(), UV_RUN_NOWAIT);
}
}

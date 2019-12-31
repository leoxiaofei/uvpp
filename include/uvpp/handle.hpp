#pragma once

#include "error.hpp"
#include "callback.hpp"


namespace uvpp {
	/**
	 * Wraps a libuv's uv_handle_t, or derived such as uv_stream_t, uv_tcp_t etc.
	 *
	 * Resources are released on the close call as mandated by libuv and NOT on the dtor
	 */
	template<typename HANDLE_O, typename HANDLE_T>
	class Handle
	{
		typedef Handle<HANDLE_O, HANDLE_T> SELF;
		HANDLE_T m_uv_handle;

	public:
		Callback m_cb_close;

	protected:
		Handle()
		{
			m_uv_handle.data = this;
		}

		virtual ~Handle()
		{
			close();
		}

		Handle(const Handle&) = delete;
		Handle& operator=(const Handle&) = delete;

	public:
		template<typename T = HANDLE_T>
		T* get()
		{
			return reinterpret_cast<T*>(&m_uv_handle);
		}

		template<typename T = HANDLE_T>
		const T* get() const
		{
			return reinterpret_cast<const T*>(&m_uv_handle);
		}

		template<typename SUB_REQUEST_T>
		static HANDLE_O* self(SUB_REQUEST_T* handle)
		{
			return reinterpret_cast<HANDLE_O*>(handle->data);
		}

		bool is_active() const
		{
			return uv_is_active(get<uv_handle_t>()) != 0;
		}

		void close()
		{
			if (!uv_is_closing(get<uv_handle_t>()))
			{
				uv_close(this->get<uv_handle_t>(), INVOKE_HD_CB(m_cb_close));
			}
		}

	protected:
	};

}


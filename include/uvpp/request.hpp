#pragma once

#include "error.hpp"
#include "callback.hpp"
#include <uv.h>

namespace uvpp
{
	/**
	 * Wraps a libuv's uv_handle_t, or derived such as uv_stream_t, uv_tcp_t etc.
	 *
	 * Resources are released on the close call as mandated by libuv and NOT on the dtor
	 */
	template<typename REQUEST_O, typename REQUEST_T>
	class Request
	{
		REQUEST_T m_uv_request;
	protected:
		Request()
		{
			m_uv_request.data = this;
		}

		~Request()
		{
		}

		Request(const Request&) = delete;
		Request& operator=(const Request&) = delete;

	public:
		typedef REQUEST_T UV_REQ;
		template<typename T = REQUEST_T>
		T* get()
		{
			return reinterpret_cast<T*>(&m_uv_request);
		}

		template<typename T = REQUEST_T>
		const T* get() const
		{
			return reinterpret_cast<const T*>(&m_uv_request);
		}

		template<typename SUB_REQUEST_T>
		static REQUEST_O* self(SUB_REQUEST_T* handle)
		{
			return reinterpret_cast<REQUEST_O*>(handle->data);
		}

		int cancel()
		{
			return uv_cancel(get<uv_req_t>());
		}

	protected:
	};

	class Write : public Request<Write, uv_write_t>
	{
		CallbackWithResult m_cb_write;
	public:
		Write(const CallbackWithResult& cb_write)
			: m_cb_write(cb_write)
		{}

		static void write_cb(uv_write_t* req, int status)
		{
			std::unique_ptr<Write> sp(self(req));
			if (sp->m_cb_write)
			{
				sp->m_cb_write(Result(status));
			}
		}

	};

	class Shutdown : public Request<Shutdown, uv_shutdown_t>
	{
		CallbackWithResult m_cb_shutdown;
	public:
		Shutdown(const CallbackWithResult& cb_shutdown)
			: m_cb_shutdown(cb_shutdown)
		{}

		static void shutdown_cb(uv_shutdown_t* req, int status)
		{
			std::unique_ptr<Shutdown> sp(self(req));
			if (sp->m_cb_shutdown)
			{
				sp->m_cb_shutdown(Result(status));
			}
		}

	};

	typedef std::function<void(class Fs*)> CallbackWithFs;

	class Fs : public Request<Fs, uv_fs_t>
	{ 
		CallbackWithFs m_cb_fs;
	public:
		Fs(const CallbackWithFs& cb_fs)
			: m_cb_fs(cb_fs)
		{

		}

		~Fs()
		{
			uv_fs_req_cleanup(get());
		}

		static void fs_cb(uv_fs_t* req)
		{
			std::unique_ptr<Fs> sp(self(req));
			if(sp->m_cb_fs)
			{
				sp->m_cb_fs(sp.get());
			}

			/*
			  uv_fs_type fs_type;
				uv_loop_t* loop;
				uv_fs_cb cb;
				ssize_t result;
				void* ptr;
				const char* path;
				uv_stat_t statbuf;  //Stores the result of uv_fs_stat() and uv_fs_fstat(). 
				UV_FS_PRIVATE_FIELDS
			*/
		}
	};

	class Work : public Request<Work, uv_work_t>
	{
		Callback m_cb_work;
		CallbackWithResult m_cb_after_work;
	public:
		Work(const Callback& cb_work)
			: m_cb_work(cb_work)
		{}

		Work(const Callback& cb_work,
			const CallbackWithResult& cb_after_work)
			: m_cb_work(cb_work)
			, m_cb_after_work(cb_after_work)
		{}

		static void work_cb(uv_work_t* req)
		{
			Work* sp = self(req);
			if (sp->m_cb_work)
			{
				sp->m_cb_work();
			}
		}

		static void after_work_cb(uv_work_t* req, int status)
		{
			std::unique_ptr<Work> sp(self(req));
			if (sp->m_cb_after_work)
			{
				sp->m_cb_after_work(Result(status));
			}
		}
	};

	class Connect : public Request<Connect, uv_connect_t>
	{
		CallbackWithResult m_cb_connect;
	public:
		Connect(const CallbackWithResult& cb_connect)
			: m_cb_connect(cb_connect)
		{}

		static void connect_cb(uv_connect_t* req, int status)
		{
			std::unique_ptr<Connect> sp(self(req));
			if (sp->m_cb_connect)
			{
				sp->m_cb_connect(Result(status));
			}

		}
	};

	typedef std::function<void(Result,
		const std::shared_ptr<addrinfo>&)> CallbackWithAddrInfo;

	class GetAddrInfo : public Request<GetAddrInfo, uv_getaddrinfo_t>
	{
		CallbackWithAddrInfo m_cb_getaddrinfo;

	public:
		GetAddrInfo(const CallbackWithAddrInfo& cb_getaddrinfo) 
			: m_cb_getaddrinfo(cb_getaddrinfo)
		{

		}

		static void getaddrinfo_cb(uv_getaddrinfo_t *resolver, 
			int status, struct addrinfo *res)
		{
			std::unique_ptr<GetAddrInfo> sp(self(resolver));
			std::shared_ptr<addrinfo> sp_res(res, uv_freeaddrinfo);

			if(sp->m_cb_getaddrinfo)
			{
				sp->m_cb_getaddrinfo(Result(status), sp_res);
			}
		}
	};

	template<class T, typename... Args>
	typename T::UV_REQ* NewReq(const Args&... args)
	{
		T* req = new T(args...);
		return req->get();
	}
}

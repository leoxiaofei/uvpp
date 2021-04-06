#pragma once

#include "handle.hpp"
#include "error.hpp"
#include "request.hpp"
#include "pool.hpp"
#include <algorithm>
#include <memory>

namespace uvpp {

	typedef std::function<void(const char* buf, size_t len)> CallbackWithBuffer;

	template<typename HANDLE_O, typename HANDLE_T>
	class Stream : public Handle<HANDLE_O, HANDLE_T>
	{
	public:
		typedef Stream<HANDLE_O, HANDLE_T> Self;
		CallbackWithResult m_cb_listen;
		CallbackWithBuffer m_cb_read;
		CallbackWithResult m_cb_read_error;

	protected:
		Stream()
			: Handle<HANDLE_O, HANDLE_T>()
		{}

		~Stream()
		{
			read_stop();
		}

	public:
		Result listen(const CallbackWithResult& cb_listen, int backlog = 128)
		{
			m_cb_listen = cb_listen;
			return Result(uv_listen(this->template get<uv_stream_t>(), backlog,
				INVOKE_HD_CBRST(m_cb_listen)));
		}

		Result accept(Stream& client)
		{
			return Result(uv_accept(this->template get<uv_stream_t>(), 
				client.template get<uv_stream_t>()));
		}

		Result read_start(const CallbackWithBuffer& cb_read,
			const CallbackWithResult& cb_read_error)
		{
			m_cb_read = cb_read;
			m_cb_read_error = cb_read_error;

			return Result(uv_read_start(this->template get<uv_stream_t>(),
				[](uv_handle_t*, size_t suggested_size, uv_buf_t* buf)
				{
					assert(buf);
					suggested_size -= 8;
					buf->base = DPool::malloc(suggested_size, &suggested_size);
					buf->len = (int)suggested_size;
				},
				[](uv_stream_t* s, ssize_t nread, const uv_buf_t* buf)
				{
					typedef void (*FREEFUNC)(char* p);
					std::unique_ptr<char, FREEFUNC> baseHolder(
						buf->base, &DPool::free);

					if (nread > 0)
					{
						if (Self::self(s)->m_cb_read)
						{
							Self::self(s)->m_cb_read(buf->base, nread);
						}
					}
					else
					{
						if (Self::self(s)->m_cb_read_error)
						{
							Self::self(s)->m_cb_read_error(Result((int)nread));
						}
					}
				}));
		}

		Result read_stop()
		{
			return Result(uv_read_stop(this->template get<uv_stream_t>()));
		}

		//Result write(const char* buf, unsigned int len, const CallbackWithResult& callback)
		//{
		//	uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf), len) };
		//	return Result(uv_write(NewReq<Write>(callback), this->template get<uv_stream_t>(),
		//		bufs, 1, &Write::write_cb));
		//}

		//Result write(const std::string& buf, const CallbackWithResult& callback)
		//{
		//	uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf.c_str()),
		//		static_cast<uint32_t>(buf.size())) };
		//	return Result(uv_write(NewReq<Write>(callback), this->template get<uv_stream_t>(),
		//		bufs, 1, &Write::write_cb));
		//}

		//Result write(const std::vector<char>& buf, const CallbackWithResult& callback)
		//{
		//	uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(&buf[0]),
		//		static_cast<uint32_t>(buf.size())) };
		//	return Result(uv_write(NewReq<Write>(callback), 
		//		this->template get<uv_stream_t>(), bufs, 1, &Write::write_cb));
		//}

		template <typename...T>
		Result write(const CallbackWithResult& callback, const T&... buf)
		{
			uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf.data()), static_cast<uint32_t>(buf.size()))... };
			return Result(uv_write(NewReq<Write>(callback),
				this->template get<uv_stream_t>(), bufs, sizeof...(T), &Write::write_cb));
		}

		Result shutdown(const CallbackWithResult& callback)
		{
			return Result(uv_shutdown(NewReq<Shutdown>(callback),
				this->template get<uv_stream_t>(), Shutdown::shutdown_cb));
		}

		Result is_readable() const
		{
			return Result(uv_is_readable(this->template get<uv_stream_t>()));
		}

		Result is_writable() const
		{
			return Result(uv_is_writable(this->template get<uv_stream_t>()));
		}

		Result stream_set_blocking(int blocking)
		{
			return Result(uv_stream_set_blocking(this->template get<uv_stream_t>(), blocking));
		}
	};
}

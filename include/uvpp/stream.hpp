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
		typedef Stream<HANDLE_O, HANDLE_T> SELF;
	public:
		CallbackWithResult m_cb_listen;
		CallbackWithBuffer m_cb_read;

	protected:
		Stream()
			: Handle<HANDLE_O, HANDLE_T>()
		{}

		~Stream()
		{
			read_stop();
		}

	public:
		Result listen(CallbackWithResult cb_listen, int backlog = 128)
		{
			m_cb_listen = cb_listen;
			return Result(uv_listen(this->get<uv_stream_t>(), backlog,
				INVOKE_HD_CBRST(m_cb_listen)));
		}

		Result accept(Stream& client)
		{
			return Result(uv_accept(this->get<uv_stream_t>(), 
				client.get<uv_stream_t>()));
		}

		Result read_start(CallbackWithBuffer cb_read)
		{
			m_cb_read = cb_read;
			return Result(uv_read_start(this->get<uv_stream_t>(),
				[](uv_handle_t*, size_t suggested_size, uv_buf_t* buf)
				{
					assert(buf);
					buf->base = DPool::malloc(suggested_size, &suggested_size);
					buf->len = suggested_size;
				},
				[](uv_stream_t* s, ssize_t nread, const uv_buf_t* buf)
				{
					auto del = &DPool::free;
					// handle callback throwing exception: hold data in unique_ptr
					std::unique_ptr<char, decltype(del) > baseHolder(buf->base, del);

					if (SELF::self(s)->m_cb_read)
					{
						SELF::self(s)->m_cb_read(nread < 0 ? 0 : buf->base, nread);
					}
				}));
		}

		Result read_stop()
		{
			return Result(uv_read_stop(this->get<uv_stream_t>()));
		}

		Result write(const char* buf, int len, CallbackWithResult callback)
		{
			uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf), len) };
			return Result(uv_write(NewReq<Write>(callback), this->get<uv_stream_t>(),
				bufs, 1, &Write::write_cb));
		}

		bool write(const std::string& buf, CallbackWithResult callback)
		{
			uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(buf.c_str()), buf.length()) };
			return Result(uv_write(NewReq<Write>(callback), this->get<uv_stream_t>(),
				bufs, 1, &Write::write_cb));
		}

		bool write(const std::vector<char>& buf, CallbackWithResult callback)
		{
			uv_buf_t bufs[] = { uv_buf_init(const_cast<char*>(&buf[0]), buf.size()) };
			return Result(uv_write(NewReq<Write>(callback), 
				this->get<uv_stream_t>(), bufs, 1, &Write::write_cb));
		}

		Result shutdown(CallbackWithResult callback)
		{
			return Result(uv_shutdown(new Shutdown(callback),
				this->get<uv_stream_t>(), Shutdown::shutdown_cb));
		}

		Result is_readable() const
		{
			return Result(uv_is_readable(this->get<uv_stream_t>()));
		}

		Result is_writable() const
		{
			return Result(uv_is_writable(this->get<uv_stream_t>()));
		}

		Result stream_set_blocking(int blocking)
		{
			return Result(uv_stream_set_blocking(this->get<uv_stream_t>(), blocking));
		}

	};
}

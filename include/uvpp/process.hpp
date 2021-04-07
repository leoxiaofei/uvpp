#pragma once

#include "loop.hpp"
#include "stream.hpp"
#include "uv.h"
#include <vector>

namespace uvpp {

	typedef std::function<void(int64_t exit_status, int term_signal)> cb_exit;

	class Process : public Handle<Process, uv_process_t>
	{
		cb_exit on_cb_exit;
		uv_process_options_t options;
		std::vector<uv_stdio_container_t> child_stdio;
		
	public:
		Process()
			: options{0}
		{
			init();
		}

		Result kill(int signum)
		{
			return Result(uv_process_kill(get(), signum));
		}

		uv_pid_t get_pid()
		{
			return uv_process_get_pid(get());
		}

		Result spawn(Loop& l, char** args)
		{
			options.args = args;
			options.file = args[0];

			options.stdio_count = child_stdio.size();
			options.stdio = child_stdio.data();
			return Result(uv_spawn(l.get(), get(), &options));
		}

		template<class CB>
		void set_exit_cb(CB && cb)
		{
			on_cb_exit = std::forward<CB>(cb);
		}

		void set_flags(unsigned int flags)
		{
			options.flags = flags;
		}

		enum stdio_type {ST_IN, ST_OUT, ST_ERR};

		template<typename HANDLE_O, typename HANDLE_T>
		void set_stdio(stdio_type type, uvpp::Stream<HANDLE_O, HANDLE_T>& pipe)
		{
			if (type >= child_stdio.size())
			{
				child_stdio.resize(type + 1);
			}

			uv_stdio_container_t& st_co = child_stdio[type];
			st_co.flags = (uv_stdio_flags)(UV_CREATE_PIPE 
				| (type > ST_IN ? UV_WRITABLE_PIPE : UV_READABLE_PIPE));
			st_co.data.stream = pipe.template get<uv_stream_t>();
		}

	protected:
		void init()
		{
			options.exit_cb = [](uv_process_t* handle, int64_t exit_status, int term_signal)
			{
				if (auto& on_cb_exit = Process::self(handle)->on_cb_exit)
				{
					on_cb_exit(exit_status, term_signal);
				}
			};
		}

	};
}

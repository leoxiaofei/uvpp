#pragma once

#include "stream.hpp"
#include "net.hpp"
#include "loop.hpp"
#include "request.hpp"

namespace uvpp {
	class Tcp : public Stream<Tcp, uv_tcp_t>
	{
	public:
		Tcp()
		{
			uv_tcp_init(uv_default_loop(), get());
		}

		Tcp(Loop& l)
		{
			uv_tcp_init(l.get(), get());
		}

		Result nodelay(bool enable)
		{
			return Result(uv_tcp_nodelay(get(), enable ? 1 : 0));
		}

		Result keepalive(bool enable, unsigned int delay)
		{
			return Result(uv_tcp_keepalive(get(), enable ? 1 : 0, delay));
		}

		Result simultanious_accepts(bool enable)
		{
			return Result(uv_tcp_simultaneous_accepts(get(), enable ? 1 : 0));
		}

		// FIXME: refactor with getaddrinfo
		Result bind(const std::string& ip, int port, unsigned int flags = 0)
		{
			ip4_addr addr = to_ip4_addr(ip, port);
			return Result(uv_tcp_bind(get(), reinterpret_cast<sockaddr*>(&addr), flags));
		}

		Result bind6(const std::string& ip, int port, unsigned int flags = 0)
		{
			ip6_addr addr = to_ip6_addr(ip, port);
			return Result(uv_tcp_bind(get(), reinterpret_cast<sockaddr*>(&addr), flags));
		}

		Result connect(const std::string& ip, int port, const CallbackWithResult& cb_connect)
		{
			ip4_addr addr = to_ip4_addr(ip, port);

			return Result(uv_tcp_connect(NewReq<Connect>(cb_connect), get(),
				reinterpret_cast<const sockaddr*>(&addr), &Connect::connect_cb));
		}

		Result connect6(const std::string& ip, int port, CallbackWithResult cb_connect)
		{
			ip6_addr addr = to_ip6_addr(ip, port);
			return Result(uv_tcp_connect(NewReq<Connect>(cb_connect), get(),
				reinterpret_cast<const sockaddr*>(&addr), &Connect::connect_cb));
		}

		bool getsockname(bool& ip4, std::string& ip, int& port)
		{
			bool ret(false);
			struct sockaddr_storage addr;
			int len = sizeof(addr);
			if (uv_tcp_getsockname(get(), reinterpret_cast<struct sockaddr*>(&addr), &len) == 0)
			{
				ip4 = (addr.ss_family == AF_INET);
				ret = ip4 ? from_ip4_addr(reinterpret_cast<ip4_addr*>(&addr), ip, port)
					: from_ip6_addr(reinterpret_cast<ip6_addr*>(&addr), ip, port);
			}
			return ret;
		}

		bool getpeername(bool& ip4, std::string& ip, int& port)
		{
			bool ret(false);
			struct sockaddr_storage addr;
			int len = sizeof(addr);
			if (uv_tcp_getpeername(get(), reinterpret_cast<struct sockaddr*>(&addr), &len) == 0)
			{
				ip4 = (addr.ss_family == AF_INET);
				ret = ip4 ? from_ip4_addr(reinterpret_cast<ip4_addr*>(&addr), ip, port)
					: from_ip6_addr(reinterpret_cast<ip6_addr*>(&addr), ip, port);
			}
			return ret;
		}
	};
}

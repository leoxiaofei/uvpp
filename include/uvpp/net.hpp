#pragma once

#include <memory>
#include <sstream>
#include "error.hpp"
#include "loop.hpp"
#include "request.hpp"

#ifdef _MSC_BUILD
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"userenv.lib")
#endif

/// Formatted string, allows to use stream operators and returns a std::string with the resulting format
#define fs(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << x)).str ())

namespace uvpp {
typedef sockaddr_in ip4_addr;
typedef sockaddr_in6 ip6_addr;

inline ip4_addr to_ip4_addr(const std::string& ip, int port)
{
    ip4_addr result;
    int res = 0;
    if ((res = uv_ip4_addr(ip.c_str(), port, &result)) != 0)
        throw Exception(fs("uv_ip4_addr error: " << Result(res).str()));
    return result;
}

inline ip6_addr to_ip6_addr(const std::string& ip, int port)
{
    ip6_addr result;
    int res = 0;
    if ((res = uv_ip6_addr(ip.c_str(), port, &result)) != 0)
        throw Exception(fs("uv_ip6_addr error: " << Result(res).str()));
    return result;
}

inline bool from_ip4_addr(ip4_addr* src, std::string& ip, int& port)
{
    char dest[16];
    if (uv_ip4_name(src, dest, 16) == 0)
    {
        ip = dest;
        port = static_cast<int>(ntohs(src->sin_port));
        return true;
    }
    return false;
}

inline bool from_ip6_addr(ip6_addr* src, std::string& ip, int& port)
{
    char dest[46];
    if (uv_ip6_name(src, dest, 46) == 0)
    {
        ip = dest;
        port = static_cast<int>(ntohs(src->sin6_port));
        return true;
    }
    return false;
}

inline Result resolve(Loop& loop, const std::string& addr, 
	const CallbackWithAddrInfo& cb_getaddrinfo)
{
	return Result(uv_getaddrinfo(loop.get(),
		NewReq<GetAddrInfo>(cb_getaddrinfo),
		&GetAddrInfo::getaddrinfo_cb, addr.c_str(), 0, 0));
}

}
#undef fs

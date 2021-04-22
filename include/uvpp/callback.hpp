#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "error.hpp"

namespace uvpp 
{
	typedef std::function<void()> Callback;
	typedef std::function<void(const Result&)> CallbackWithResult;

/// Callback
#define INVOKE_HD_CB(cb) [](typename Self::Hwnd* handle) \
	{if(Self::self(handle)->cb) Self::self(handle)->cb();}

/// CallbackWithResult
#define INVOKE_HD_CBRST(cb) [](typename Self::Hwnd* handle, int status) \
	{if(Self::self(handle)->cb) Self::self(handle)->cb(Result(status));}

#define INVOKE_CLOSE_CB(cb) [](uv_handle_t* handle) \
	{if(Self::self(handle)->cb) Self::self(handle)->cb();}

#define INVOKE_CONNECT_CB(cb) [](uv_stream_t* handle, int status) \
	{if(Self::self(handle)->cb) Self::self(handle)->cb(Result(status));}
}


#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "error.hpp"

namespace uvpp 
{
	typedef std::function<void()> Callback;
	typedef std::function<void(Result)> CallbackWithResult;

/// Callback
#define INVOKE_HD_CB(cb) [](auto* handle) \
	{if(SELF::self(handle)->cb) SELF::self(handle)->cb();}

/// CallbackWithResult
#define INVOKE_HD_CBRST(cb) [](auto* handle, int status) \
	{if(SELF::self(handle)->cb) SELF::self(handle)->cb(Result(status));}

}


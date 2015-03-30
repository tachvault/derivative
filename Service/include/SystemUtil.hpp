/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SYSTEMUTIL_H_
#define _DERIVATIVE_SYSTEMUTIL_H_
#pragma once

#include "Global.hpp"
#include "boost/filesystem/path.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef SERVICEUTIL_EXPORTS
#ifdef __GNUC__
#define SERVICE_UTIL_DLL_API __attribute__ ((dllexport))
#else
#define SERVICE_UTIL_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define SERVICE_UTIL_DLL_API __attribute__ ((dllimport))
#else
#define SERVICE_UTIL_DLL_API __declspec(dllimport)
#endif
#endif
#define ESERVICE_UTIL_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define SERVICE_UTIL_DLL_API __attribute__ ((visibility ("default")))
#define SERVICE_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define SERVICE_UTIL_DLL_API
#define SERVICE_UTIL_DLL_LOCAL
#endif
#endif

typedef int (* DLL_INIT_FUNCTION) (int, char**);

namespace derivative
{

	namespace SystemUtil
	{
		/// This operation loads the specified library and calls the
		/// named entry point if one is specified. It returns 0
		/// if the operation is successful, otherwise returns error code
		SERVICE_UTIL_DLL_API bool LoadSharedLibrary (const char* lib, const char* func = 0);
	};
}

#endif

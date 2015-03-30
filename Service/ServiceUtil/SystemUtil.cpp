/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/


#include <io.h>

#if defined _WIN32
#include <windows.h>
#include <winsvc.h>
#include <DbgHelp.h>
#else
#include <dlfcn.h>
#endif
#include <map>
#include <string>
#include <iostream>
#include <limits>

#include "SystemUtil.hpp"

#undef max

namespace derivative
{
	namespace SystemUtil
	{
		bool LoadSharedLibrary (const char* libInput, const char* func)
		{
			std::string lib(libInput);

#if defined _WIN32
			lib.append(".dll");
			HINSTANCE sdlLib = ::LoadLibraryA(lib.c_str());
#else
			lib.append(".so");
			void* sdlLib = dlopen(lib.c_str(), RTLD_LAZY);
#endif
			if (!sdlLib)
			{
				auto error = GetLastError();
				return false;
			}
			else if (func)
			{
#if defined _WIN32
				FARPROC fp = GetProcAddress(sdlLib, func);
#else
				void* fp = dlsym(sdlLib,func);
#endif
				if (fp == NULL)
				{
					return false;
				}
				(*fp)();
			}
			return true;
		}
	}
}

#define max(a,b)  ((a > b) ? a : b)

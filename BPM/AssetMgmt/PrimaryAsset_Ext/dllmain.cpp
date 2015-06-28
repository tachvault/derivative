// dllmain.cpp : Defines the entry point for the DLL application.

#include <Windows.h>
#include "IDataSource.hpp"
#include "IRCurve.hpp"

#ifdef __GNUC__
#define ATTACH_PREFIX static void __attribute__ ((constructor))
#define DETACH_PREFIX  static void __attribute__ ((destructor))
#else
#define ATTACH_PREFIX void
#define DETACH_PREFIX void
#endif

using namespace derivative;

ATTACH_PREFIX attach()
{}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//attach();
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


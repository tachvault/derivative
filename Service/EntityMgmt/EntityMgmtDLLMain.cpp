// dllmain.cpp : Defines the entry point for the DLL application.
#include <iostream>
#if defined _WIN32
#include <Windows.h>
#endif
#include "EntityManager.hpp"

using namespace derivative;

#ifdef __GNUC__
#define ATTACH_PREFIX static void __attribute__ ((constructor))
#define DETACH_PREFIX  static void __attribute__ ((destructor))
#else
#define ATTACH_PREFIX void
#define DETACH_PREFIX void
#endif

ATTACH_PREFIX attach()
{
	EntityManager& entMgr = EntityManager::getInstance();
	std::cout << "EntityManager is initialized " << std::endl;
}
DETACH_PREFIX detach()
{}

#if defined _WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			attach();			
		}
		break;
	case DLL_THREAD_ATTACH:			
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif


// dllmain.cpp : Defines the entry point for the DLL application.

#if defined _WIN32
#include <Windows.h>
#endif
#include "IDataSource.hpp"
#include "CurrencyHolder.hpp"
#include "CountryHolder.hpp"
#include "ExchangeHolder.hpp"
#include "ExchangeExt.hpp"

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
	// start logging
	if (const char* env_p = std::getenv("LOG_DIR"))
	{
		std::string log_dir = std::string(env_p);
		FLAGS_log_dir = log_dir.c_str();
		google::InitGoogleLogging("Derivative");
	}
	else
	{
		throw std::runtime_error("Log directory is not defined in environment variables");
	}
	
	/// load all the currency data from database
	CurrencyHolder& currHolder = CurrencyHolder::getInstance();
	currHolder.Init(MYSQL);

	/// load all the country data from database
	CountryHolder& cntryHolder = CountryHolder::getInstance();
	cntryHolder.Init(MYSQL);

	/// load all the exchange data from database
	ExchangeHolder& exHolder = ExchangeHolder::getInstance();
	exHolder.Init(MYSQL);

	/// load alias symbol (ticker name) used by data sources 
	/// such as Yahoo for each ticker name we use
	ExchangeExt& extMap = ExchangeExt::getInstance();
	extMap.Init(MYSQL);
}
DETACH_PREFIX detach()
{}

#if defined _WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
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

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <future>
#include "MessageDispatcher.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "SystemUtil.hpp"
#include "SystemManager.hpp"
#include "IRESTJSONRequestInterceptor.hpp"
#include "BPMLoader.hpp"

namespace derivative
{
	/// load the libraries first
	void LoadLibraries(LibMapType& derivativeLibs, runModeEnum mode)
	{
		std::pair <LibMapType::iterator, LibMapType::iterator> ret;
		ret = derivativeLibs.equal_range(mode);
		for (LibMapType::iterator it = ret.first; it != ret.second; ++it)
		{
			cout << "Loading " << it->second << " in " << it->first << " mode " << endl;
			bool retValue = SystemUtil::LoadSharedLibrary(it->second.c_str());
			assert(retValue == true);
		}
	}
}

using namespace derivative;

void StartDispatcher()
{
	/// get the message dispatcher
	Name nm(MessageDispatcher::TYPEID, 1);
	std::shared_ptr<MessageDispatcher> disp = BuildMessageDispatcher(nm);

	// Spin forever each message dispatcher. Ideally there should 
	for (;;)
	{
		disp->Spin(-1);
	}

	// if we get here, we have a problem
	// write an error log and exit.
	std::cout << "No Message Dispatcher! exiting" << endl;
	exit(1);
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: loader <run mode> address" << std::endl;
		return -1;
	}

	// start logging
	FLAGS_log_dir = "C://Temp/glog";
	google::InitGoogleLogging("Derivative");

	/// get the mode (STANDALONE, APP_SERVER, LOAD_BALANCER)
	/// and load the required libraries for each mode
	runModeEnum mode = static_cast<runModeEnum>(atoi(argv[1]));
	std::string addr = argv[2];

	/// define a map to hold all the library info per run mode.
	LibMapType derivativeLibs;

	/// Initialize the map. Now it is hard coded
	/// later need be loaded from a config file
	InitLibConfig(derivativeLibs);

	/// load required libraries by run mode.
	LoadLibraries(derivativeLibs, mode);

	/// start the web interceptors
	WebAddress equiOptJson = getEquityOptionJSONAddr(addr);
	Name nm = IRESTJSONRequestInterceptor::ConstructName(equiOptJson.address, equiOptJson.port, equiOptJson.path);
	std::shared_ptr<IRESTJSONRequestInterceptor> interceptorJSON = EntityMgrUtil::ConstructEntity<IRESTJSONRequestInterceptor>(nm);

	/// Start processing the message asynchronously in a new thread
	try
	{
		auto future = std::async(std::launch::async, &IRESTJSONRequestInterceptor::StartInterceptor, interceptorJSON);
	}
	catch (std::exception & e)
	{
		LOG(ERROR) << " Error starting Casablanca web services " << e.what() << endl;
		throw e;
	}

	/// Now set the run mode so that rest of the modules can be loaded
	/// and executed conditionally based on the runmode
	SystemManager& sysMgr = SystemManager::getInstance();
	sysMgr.SetRunMode(mode);

	if (mode == runModeEnum::STANDALONE || mode == runModeEnum::APP_SERVER)
	{
		/// Start BPMLoader
		BPMLoader& bpm = BPMLoader::getInstance();
		bpm.LoadLIBORRates();
		bpm.LoadRates();

		/// start the dispatcher
		StartDispatcher();
	}

	// the function should never return
	return 0;
}
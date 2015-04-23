/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_CONFIG_H_
#define _DERIVATIVE_CONFIG_H_

#include <string>
#include <map>

namespace derivative
{
	enum runModeEnum
	{
		STANDALONE = 0, /// run one instance of application server and load balancer together 
		APP_SERVER = 1,  /// run application server alone
		LOAD_BALANCER = 2 /// run load balancer alone
	};

	struct WebAddress
	{
		string address;

		unsigned int port;

		string path;

		WebAddress(string addr, unsigned int pt, string p)
			:address(addr), port(pt), path(p)
		{}
	};

	/// define map that keeps track of run modes and required libraries for
	/// each run mode
	typedef std::multimap<runModeEnum, std::string> LibMapType;
	
	inline void InitLibConfig(LibMapType& derivativeLibs)
	{
		/// standalone mode required all the libraries loaded
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "MessageDispatcher"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "ESB"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "Webservice"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "DataSource_MySQL"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "DataSource_REST"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "PrimaryAsset"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "MySQLDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "YahooDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "XigniteDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "Facades"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "BPMLoader"));

		/// application server mode required the following libraries loaded
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "MessageDispatcher"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "DataSource_MySQL"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "DataSource_REST"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "PrimaryAsset"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "MySQLDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "YahooDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "XigniteDataAccess"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(APP_SERVER, "Facades"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(STANDALONE, "BPMLoader"));

		/// load balancer mode required the following libraries loaded
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(LOAD_BALANCER, "ESB"));
		derivativeLibs.insert(std::pair<runModeEnum, std::string>(LOAD_BALANCER, "WebService"));
	}

	inline WebAddress getEquityOptionJSONAddr()
	{
		WebAddress waddr(string("http://localhost:"), 34568, string("JSON"));

		return waddr;
	}

}

/* namespace derivative */

#endif /* _DERIVATIVE_CONFIG_H_ */


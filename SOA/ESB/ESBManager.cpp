/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <memory>
#include <exception>
#include <functional> 
#include <algorithm>
#include "ESBManager.hpp"
#include "DException.hpp"
#include "ServiceRouter.hpp"
#include "ServiceLogger.hpp"
#include "SecurityManager.hpp"


namespace derivative
{
	bool ESBManager::m_initialized = false;

	/// Supports singleton. But NOT lazy initialization.
	/// When this DLL is getting loaded, the getInstance should
	/// be called as part of attach method.
	ESBManager& ESBManager::getInstance()
	{
		static std::unique_ptr<ESBManager> _instance;
		if (!m_initialized)
		{
            _instance.reset(new ESBManager);
			m_initialized = true;
			LOG(INFO) << "ESBManager is initialized" << endl;
		}
        return *_instance;      
	}

	void ESBManager::HandleRequest(std::shared_ptr<IMessage> msg)
	{
		/// execute the request if authorization succeeds. 
		/// If failed then security manager would raise exception.
		m_router->RouteMessage(msg);
	}

	inline bool ESBManager::Authorize(const std::string& token)
	{
		return m_security->AuthorizeRequest(token);
	}

	void ESBManager::LogRequest(const std::string& token, const std::string& datetime, const std::string& url)
	{
		m_logger->LogMessage(token, datetime, url);
	}

	ESBManager::ESBManager()
	{
		/// construct router
		m_router = std::unique_ptr<ServiceRouter>(new ServiceRouter);

		/// construct security manager
		m_security = std::unique_ptr<SecurityManager>(new SecurityManager);

		/// construct logger
		m_logger = std::unique_ptr<ServiceLogger>(new ServiceLogger);
		
		LOG(INFO) << "ESBManager constructor is called " << endl;
	}

	ESBManager::~ESBManager()
	{
		LOG(INFO) << "ESBManager destructor is called " << endl;
	}

} /* namespace derivative */
/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <exception>
#include <functional> 
#include <algorithm>
#include "SystemManager.hpp"
#include "DException.hpp"

namespace derivative
{
	bool SystemManager::m_initialized = false;

	/// Supports singleton. But NOT lazy initialization.
	/// When this DLL is getting loaded, the getInstance should
	/// be called as part of attach method.
	SystemManager& SystemManager::getInstance()
	{
		static std::unique_ptr<SystemManager> _instance;
		if (!m_initialized)
		{
            _instance.reset(new SystemManager);
			m_initialized = true;
			LOG(INFO) << "SystemManager is initialized" << endl;
		}
        return *_instance;      
	}

	SystemManager::SystemManager()
	{
		LOG(INFO) << "SystemManager constructor is called " << endl;
	}

	SystemManager::~SystemManager()
	{
		LOG(INFO) << "SystemManager destructor is called " << endl;
	}

} /* namespace derivative */
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <exception>
#include <functional> 
#include <algorithm>
#include "MsgProcessorManager.hpp"
#include "DException.hpp"

namespace derivative
{
	bool MsgProcessorManager::m_initialized = false;

	/// Supports singleton. But NOT lazy initialization.
	/// When this DLL is getting loaded, the getInstance should
	/// be called as part of attach method.
	MsgProcessorManager& MsgProcessorManager::getInstance()
	{
		static std::unique_ptr<MsgProcessorManager> _instance;
		if (!m_initialized)
		{
            _instance.reset(new MsgProcessorManager);
			m_initialized = true;
			LOG(INFO) << "MsgProcessorManager is initialized" << endl;
		}
        return *_instance;      
	}

	void MsgProcessorManager::registerProcessor(msgType msgId, grpType processorId)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		m_msgToProcessor.insert(std::make_pair(msgId, processorId));

		LOG(INFO) << "Msg, Processor pair " <<  msgId << ", " << processorId << " registered " << endl;
	}

	unsigned int MsgProcessorManager::findProcessor(msgType msgId)
	{
		/// lock guard is sufficient
		std::lock_guard<std::mutex> guard(m_mutex);

		/// find all the concrete types for the given alias name.
		auto i = m_msgToProcessor.find(msgId);
		if (i != m_msgToProcessor.end())
		{
			return i->second;
		}
		else
		{
			return 0;
		}
	}

	MsgProcessorManager::MsgProcessorManager()
	{
		LOG(INFO) << "MsgProcessorManager constructor is called " << endl;
	}

	MsgProcessorManager::~MsgProcessorManager()
	{
		LOG(INFO) << "MsgProcessorManager destructor is called " << endl;
	}

} /* namespace derivative */
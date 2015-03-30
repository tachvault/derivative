/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include "ServiceRouter.hpp"
#include "DException.hpp"
#include "MessageDispatcher.hpp"
#include "SystemManager.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	ServiceRouter::ServiceRouter()
	{
		LOG(INFO) << "ServiceRouter constructor is called " << endl;
	}

	ServiceRouter::~ServiceRouter()
	{
		LOG(INFO) << "ServiceRouter destructor is called " << endl;
	}

	void ServiceRouter::RouteMessage(std::shared_ptr<IMessage>& msg)
	{
		/// Now set the run mode so that rest of the modules can be loaded
		/// and executed conditionally based on the runmode
		SystemManager& sysMgr = SystemManager::getInstance();
		if (sysMgr.GetRunMode() == runModeEnum::STANDALONE)
		{
			/// then get the MessageDispatcher (should be only one)
			/// and push the message
			Name nm(MessageDispatcher::TYPEID, 1);
			std::shared_ptr<MessageDispatcher> disp = BuildMessageDispatcher(nm);
			disp->HandleMessage(msg);
		}
		else
		{
			throw std::logic_error("Not implememented yet");
		}
	}
} /* namespace derivative */
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "GroupRegister.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "MsgProcessorManager.hpp"
#include "MsgRegister.hpp"

namespace derivative
{
	MsgRegister::MsgRegister(grpType msgType, grpType processorType)
	{
		// Make sure that both groups exist by binding 0
		MsgProcessorManager& entMgr = MsgProcessorManager::getInstance();
		entMgr.registerProcessor(msgType, processorType);
	}
}
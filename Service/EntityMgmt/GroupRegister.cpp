/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "GroupRegister.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "EntityManager.hpp"

namespace derivative
{
	// constructor for GroupRegister. 
	// should be called during the initialization of the DLL.
	GroupRegister::GroupRegister (unsigned short grpId, shared_ptr<IObject> const& exemplar)
	{
		if (exemplar != 0)
		{
			EntityManager& entMgr = EntityManager::getInstance();

			entMgr.registerObject(Name(grpId), exemplar);
		}
	}
	

	AliasRegister::AliasRegister(unsigned short concreteType, unsigned short iID)
	{
		// Make sure that both groups exist by binding 0
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerAlias(concreteType, iID);
	}

	DAORegister::DAORegister(unsigned short entityId, unsigned short source, unsigned short DAOId)
	{
		// Make sure that both groups exist by binding 0
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerDAO(entityId, source, DAOId);
	}
}
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <exception>
#include <functional> 
#include <algorithm>
#include "EntityManager.hpp"
#include "DException.hpp"

namespace derivative
{
	bool EntityManager::m_initialized = false;

	/// Supports singleton. But NOT lazy initialization.
	/// When this DLL is getting loaded, the getInstance should
	/// be called as part of attach method.
	EntityManager& EntityManager::getInstance()
	{
		static std::unique_ptr<EntityManager> _instance;
		if (!m_initialized)
		{
            _instance.reset(new EntityManager);
			m_initialized = true;
			LOG(INFO) << "EntityManager is initialized" << endl;
		}
        return *_instance;      
	}

	void EntityManager::registerObject(const Name& nm, const std::shared_ptr<IObject> &obj)
	{
		/// lock the mutex and keep it locked until
		/// call is made to EntityGroup
		std::unique_lock<std::mutex> unique_lock(m_mutex);

		/// if the EntityGroup not in the registry then insert as new entity group
		/// delegate the addion job of the passed obj to the created entityGroup
		if (m_registry.find(nm.GetGrpId()) == m_registry.end())
		{
			auto entityGroup = std::make_shared<EntityGroup>();	
			m_registry.insert(std::make_pair(nm.GetGrpId(),entityGroup));
			LOG(INFO) << "Created new EntityGroup for " << nm << endl;

			/// unlock so that it other entity groups
			/// can access the registry
			unique_lock.unlock();
			entityGroup->registerObject(nm, obj);			
		}
		/// the entity group already there in registry
		else 
		{
			auto entityGroup = m_registry.at(nm.GetGrpId());
			LOG(INFO) << "EntityGroup already exists for " << nm << endl;
			/// delegate processing the object to the entity group having same type
			unique_lock.unlock();
			entityGroup->registerObject(nm, obj);
		}
	}

	void EntityManager::registerObjects(const std::deque<const std::shared_ptr<IObject> > &objs)
	{
		///return if objs empty
		if (objs.empty())
		{
			return;
		}

		/// Get the first name on the group.
		/// so that we can get the GroupObject
		auto nm = (*objs.begin())->GetName();

		/// lock the mutex and keep it locked until
		/// call is made to EntityGroup
		std::unique_lock<std::mutex> unique_lock(m_mutex);
		
		/// if the EntityGroup not in the registry then insert as new entity group
		/// delegate the addion job of the passed obj to the created entityGroup
		if (m_registry.find(nm.GetGrpId()) == m_registry.end())
		{
			auto entityGroup = std::make_shared<EntityGroup>();	
			m_registry.insert(std::make_pair(nm.GetGrpId(),entityGroup));
			LOG(INFO) << "Created new EntityGroup for " << nm << endl;

			/// unlock so that it other entity groups
			/// can access the registry
			unique_lock.unlock();
			entityGroup->registerObjects(objs);			
		}
		/// the entity group already there in registry
		else 
		{
			auto entityGroup = m_registry.at(nm.GetGrpId());
			LOG(INFO) << "EntityGroup already exists for " << nm << endl;
			/// delegate processing the object to the entity group having same type
			unique_lock.unlock();
			entityGroup->registerObjects(objs);
		}
	}

	void EntityManager::registerAlias(grpType concreteId, grpType aliasId)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		/// insert conceret Id, alias Id pair into m_aliasToConcrete
		/// Since the registration usually happen through the
		/// AliasRegister, no attempt is made to check if
		/// the std::make_pair(concreteId, aliasId) is
		/// already registered
		m_aliasToConcrete.insert(std::make_pair(aliasId, concreteId));

		/// also keep track of this information with concreteType as the key
		m_concreteToAlias.insert(std::make_pair(concreteId, aliasId));
		LOG(INFO) << "Concrete, Alias pair " <<  aliasId << ", " << concreteId << " registered " << endl;
	}

	void EntityManager::registerDAO(grpType entityId, unsigned short source,  grpType DAOId)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		/// insert <source,conceret Id>, alias Id into m_DAO
		std::pair<ushort, grpType> entityPair;
	    entityPair = make_pair ( source, entityId);
		auto i = m_DAO.find(entityPair);
		if (i == m_DAO.end())
		{
			m_DAO.insert(std::make_pair(entityPair, DAOId));
		}
		else
		{
			/// entityId and DAOId pair already registered
			/// throw RegistryException
			LOG(WARNING) << "DAO alias " <<  DAOId << " for the entity " << entityId << " already registered " << endl;
			throw RegistryException("DAO alias for the entity already registered");
		}	
	}

	bool EntityManager::contains(shared_ptr<IObject>& obj)
	{
		if (obj == nullptr) 
		{
			LOG(ERROR) << " Null pointer object passed" << endl;
			return false; /// don't want to throw exception
		}

		/// See if the entry already exists
		Name nm = obj->GetName();

		/// Lock for access to m_registry
		std::unique_lock<std::mutex> unique_lock(m_mutex);
		/// Get the shared_ptr<EntityGroup> for the given group ID
		if (m_registry.find(nm.GetGrpId()) != m_registry.end())
		{
			/// delegate the contains request to entityGroup
			auto entityGroup = m_registry.at(nm.GetGrpId());
			LOG(INFO) << "EntityGroup already exists for " << nm << endl;
			unique_lock.unlock();
			return entityGroup->contains(obj);
		}
		LOG(WARNING) << "Given object not found in registry " << obj->GetName() << endl;
		return false;
	}
	shared_ptr<IObject> EntityManager::findObject(const Name& nm)
	{
		/// Lock for access to m_registry
		std::unique_lock<std::mutex> unique_lock(m_mutex);
		/// if the entity group in the registry then proceed
		/// to find the object
		if (m_registry.find(nm.GetGrpId()) != m_registry.end())
		{
			/// get the EntityGroup and deletegate finding the EntityGroup			
			LOG(INFO) << "EntityGroup exists for " << nm << endl;
			auto entityGroup = m_registry.at(nm.GetGrpId());
			unique_lock.unlock();
			return entityGroup->findObject(nm);
		}

		/// if no entity group created then throw RegistryException.
		/// (i.e Exemplar constructor never ran or this named object not registered!!!)

		/// We don't need to explicitly unlock the unique lock
		/// the std::unique_lock mechanism should guarentee the
		/// release of the m_mutex when exception is thrown
		LOG(WARNING) << "Exemplar object never registered and given Named object not found in registry for " << nm << endl;
		throw RegistryException("Exemplar object never registered and given Named object not found in registry");		
	}

	std::vector<std::shared_ptr<IObject> > EntityManager::findObjects(grpType id)
	{
		/// Lock for access to m_registry
		std::unique_lock<std::mutex> unique_lock(m_mutex);
		/// if the entity group in the registry then proceed
		/// to find the object
		if (m_registry.find(id) != m_registry.end())
		{
			/// get the EntityGroup and deletegate finding the EntityGroup
			auto entityGroup = m_registry.at(id);
			unique_lock.unlock();
			return entityGroup->findObjects();
		}
	}

	unsigned int EntityManager::findAlias(const Name& nm)
	{
		/// lock guard is sufficient
		std::lock_guard<std::mutex> guard(m_mutex);
		bool found = false;

		/// find all the concrete types for the given alias name.
		auto i = m_aliasToConcrete.find(nm.GetGrpId());
		if (i != m_aliasToConcrete.end())
		{
			return i->second;
		}
		else
		{
			return 0;
		}
	}

	bool EntityManager::findAlias(const Name& nm, std::vector<Name> &names)
	{
		/// lock guard is sufficient
		std::lock_guard<std::mutex> guard(m_mutex);
		bool found = false;

		/// find all the concrete types for the given alias name.
		auto i = m_aliasToConcrete.equal_range(nm.GetGrpId());

		/// for each pair in the range
		for (; i.first != i.second; ++i.first) 
		{
			/// construct the concrete Name with given obj ID
			/// and concrete type
			Name entityName(i.first->second, nm.GetObjId());
			auto k = std::find_if (names.begin(), names.end(), [entityName] (const Name& name) { return name == entityName; });
			if (k == names.end())
			{				
				/// find the object crosponding to to each concerete type and given object Id
				LOG(INFO) << " Objects exists in Alias registry for " << nm << endl;
				names.push_back(entityName);
				found = true;
			}
		}
		return found;
	}

	bool EntityManager::findAlias(const Name& nm, std::vector<std::shared_ptr<IObject> >& objs)
	{
		/// find all the concrete types for the given alias name.
		/// We need conceretNames in order to properly unlock
		/// and lock the m_mutex. Get the concrete names.
		std::vector<Name> concreteNames;	
		findAlias(nm, concreteNames);

		/// Now try to find the concerete objects from the registry
		for(auto nm: concreteNames)
		{
			try
			{
				auto obj = findObject(nm);
				/// if concrete type is not null
				if (obj)
				{
					LOG(INFO) << "Concrete object  found with name = " << obj->GetName() << endl;
					objs.push_back(obj);
				}
			}
			catch(RegistryException& e)
			{
				LOG(ERROR) << " Exception thrown " << e.what() << endl;
				LOG(ERROR) << " Concrete object type not regsitered in registry " << nm << endl;
				throw e;
			}
		}

		return (!objs.empty());
	}

	grpType EntityManager::findDAO(ushort source, grpType entityId)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		/// find DAO types for the given entity interface ID and source.
		std::pair<ushort, grpType> entityPair;
	    entityPair = make_pair ( source, entityId);
		auto i = m_DAO.find(entityPair);
        if (i != m_DAO.end())
		{
			return i->second;
		}
		LOG(ERROR) << "DAO not found for  entity type ID  = " << entityId << endl;
		throw RegistryException("DAO not found");		
	}

	grpType EntityManager::findDAO(ushort source, const std::shared_ptr<IObject>& obj)
	{
		std::lock_guard<std::mutex> guard(m_mutex);

		/// we need to find the alias types for the given object name
		/// then from alias types we need to find the DAO

		/// find all the alias types for the given object name.
		auto i = m_concreteToAlias.equal_range(obj->GetName().GetGrpId());

		/// for each pair in the range
		for (; i.first != i.second; ++i.first) 
		{
			/// see if the alias type found has a DAO
			std::pair<ushort, grpType> entityPair;
	        entityPair = make_pair ( source, i.first->second);
		    auto j = m_DAO.find(entityPair);
            if (j != m_DAO.end())
		    {
			   return j->second;
		    }
		}	
		LOG(ERROR) << "DAO not found for  object  = " << obj->GetName() << endl;
		throw RegistryException("DAO not found");	
	}

	void EntityManager::unbind(std::shared_ptr<IObject> &obj)
	{
		if (obj == nullptr) 
		{
			LOG(ERROR) << " Null pointer object passed" << endl;
			return; /// nothing to unbind;
		}

		/// find the entity group crossponding to the
		/// given Name nm
		Name nm = obj->GetName();

		std::unique_lock<std::mutex> unique_lock(m_mutex);

		/// if the object group exists then erase the object 
		if (m_registry.find(nm.GetGrpId()) != m_registry.end())
		{
			auto entityGroup = m_registry.at(nm.GetGrpId());
			LOG(INFO) << "EntityGroup found for  = " << obj->GetName() << endl;

			/// unlock it so that other groups can use the registry
			unique_lock.unlock();

			/// EntityGroup Mutex provide mutual exclusion for its member attributes
			entityGroup->unbind(obj);
		}
		else
		{
			LOG(ERROR) << " object not found in the registry" << endl;
		}
	}

	EntityManager::EntityManager()
	{
		LOG(INFO) << "EntityManager constructor is called " << endl;
	}

	EntityManager::~EntityManager()
	{
		LOG(INFO) << "EntityManager destructor is called " << endl;
	}

} /* namespace derivative */
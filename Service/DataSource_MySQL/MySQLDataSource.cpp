/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "MySQLDataSource.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "IDAO.hpp"
#include "IMake.hpp"
#include "GroupRegister.hpp"
#include "Global.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(MySQLDataSource);
	ALIAS_REGISTER(MySQLDataSource, IDataSource);


	MySQLDataSource::MySQLDataSource(const Exemplar &ex)
		:m_name(TYPEID), m_source(MYSQL)
	{
		LOG(INFO) << " Exemplar id constructed " << endl;
	}

	MySQLDataSource::MySQLDataSource (const Name& nm)
		:m_name(nm), m_source(MYSQL) 
	{
		LOG(INFO) << " MySQLDataSource constructed with name " << nm << endl;		
	}

	MySQLDataSource::~MySQLDataSource ()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	std::shared_ptr<IMake> MySQLDataSource::Make(const Name &nm)
	{
		/// Construct data source from given name and register with EntityManager
		std::shared_ptr<MySQLDataSource> dataSource = make_shared<MySQLDataSource>(nm);
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerObject(nm,dataSource);

		/// return constructed object if no exception is thrown
		return dataSource;
	}

	std::shared_ptr<IMake> MySQLDataSource::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("function not implemented");
	}

	std::shared_ptr<IObject> MySQLDataSource::GetEntity(const Name &nm, unsigned int source)     
	{
		/// if we are here then the Entity is not registered in the in-memory
		/// registry. That means DAO of the entity is also not in not 
		/// in the memory. Try to get the entity from the database.
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from database
		LOG(INFO) << " Got MySQL DAO " << entityDAO << " for " << nm << endl;
		std::shared_ptr<IObject>  entity = entityDAO->find(nm);
		return entity;
	}

	void MySQLDataSource::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities, unsigned int source)
	{
		/// if we are here then the Entity is not registered in the in-memory
		/// registry. That means DAO of the entity is also not in not 
		/// in the memory. Try to get the entity from the database.
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from database
		LOG(INFO) << " Got MySQL DAO " << entityDAO << " for " << nm << endl;
		entityDAO->find(nm, entities);
	}

	void MySQLDataSource::flush(const std::shared_ptr<IObject>& obj)
	{
		if (obj == nullptr) 
		{
			LOG(ERROR) << " Null pointer object passed" << endl;
			return;
		}

		std::shared_ptr<IDAO>& entityDAO = getDAO(obj->GetName());

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from database
		entityDAO->update();
	}

	void MySQLDataSource::insert(const std::shared_ptr<IObject>& obj)
	{
		if (obj == nullptr) 
		{
			LOG(ERROR) << " Null pointer object passed" << endl;
			return;
		}

		std::shared_ptr<IDAO>& entityDAO = getDAO(obj->GetName());

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from database
		entityDAO->insert();
	}

	std::shared_ptr<IDAO> MySQLDataSource::getDAO(const Name& nm)
	{
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();

		/// Get the DAO group ID for the given Entity type
		/// in this case the source is MYSQL		
		/// if DAO type is not registered for the entity interface ID
		/// it is an unrecoverable error. The caller need to handle
		grpType daoGrpId = entMgr.findDAO(MYSQL, nm.GetGrpId());

		/// try to find the DAO in.
		Name daoName(daoGrpId, nm.GetObjId());
		try
		{	
			/// first try to find from EntityManager
			/// if the DAO already created and registered then
			/// entity manager would return the DAO
			std::shared_ptr<IObject> obj = entMgr.findObject(daoName);

			/// if found  then return the entityDAO			
			if (obj)
			{		
				std::shared_ptr<IDAO> entityDAO = dynamic_pointer_cast<IDAO>(obj);	
				return entityDAO;
			}
			else
			{
				/// We need to construct DAO from DAO exemplar
				/// get the examplar object for the DAO
				/// Exemplar objects should be initialized
				/// during global initialization time.			
				/// Let the caller catch and process
				/// RegistryException if thrown
				std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(daoGrpId));

				/// Now we have the exampler object. 
				/// Make the exemplar DAO to construct DAO for the given entity
				/// Remember each Entity object associated with a DAO
				std::shared_ptr<IMake> obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(daoName);

				std::shared_ptr<IDAO> entityDAO = dynamic_pointer_cast<IDAO>(obj);

				return entityDAO;
			}
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " EntityGroup for the given DAO type not found for " << nm << endl;
			LOG(WARNING)  << e.what() << endl;
			throw e;
		}
	}
} /* namespace derivative */
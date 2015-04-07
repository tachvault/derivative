/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/
#include "RESTDataSource.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "IDAO.hpp"
#include "IMake.hpp"
#include "GroupRegister.hpp"
#include "Global.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(RESTDataSource);
	ALIAS_REGISTER(RESTDataSource, IDataSource);

	RESTDataSource::RESTDataSource(const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << " Exemplar id constructed " << endl;
	}

	RESTDataSource::RESTDataSource(const Name& nm)
		: m_name(nm)
	{
		m_source = { YAHOO, XIGNITE };
		LOG(INFO) << " MySQLDataSource constructed with name " << nm << endl;
	}

	RESTDataSource::~RESTDataSource()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	std::shared_ptr<IMake> RESTDataSource::Make(const Name &nm)
	{
		/// Construct data source from given name and register with EntityManager
		std::shared_ptr<RESTDataSource> dataSource = make_shared<RESTDataSource>(nm);
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerObject(nm, dataSource);

		/// return constructed object if no exception is thrown
		return dataSource;
	}

	std::shared_ptr<IMake> RESTDataSource::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This method is not supported");
	}

	std::shared_ptr<IObject> RESTDataSource::GetEntity(const Name &nm, unsigned int source)
	{
		/// Try to get the entity from yahoo through RESTDAO for the
		/// given Entity. Ex: StockValue will have a specific RESTDAO
		/// and HistoricStockValue will have different REST DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm, source);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from REST
		LOG(INFO) << " Got REST DAO " << entityDAO << " for " << nm << endl;
		try
		{
			std::shared_ptr<IObject>  entity = entityDAO->find(nm);
			entityDAO->Passivate();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
			return entity;
		}
		catch (exception& e)
		{
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			entityDAO->Passivate();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
			throw e;
		}
	}

	void RESTDataSource::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities, unsigned int source)
	{
		/// Try to get the entity from yahoo through RESTDAO for the
		/// given Entity. Ex: StockValue will have a specific RESTDAO
		/// and HistoricStockValue will have different REST DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm, source);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from REST
		LOG(INFO) << " Got REST DAO " << entityDAO << " for " << nm << endl;
		try
		{
			entityDAO->find(nm, entities);
			entityDAO->Passivate();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
		}
		catch (exception& e)
		{
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			entityDAO->Passivate();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
			throw e;
		}
	}

	bool RESTDataSource::refreshObject(std::shared_ptr<IObject>& obj, unsigned int source)
	{
		/// Try to get the entity from yahoo through RESTDAO for the
		/// given Entity. Ex: StockValue will have a specific RESTDAO
		/// and HistoricStockValue will have different REST DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(obj, source);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from REST
		LOG(INFO) << " Got REST DAO " << entityDAO << " for " << obj->GetName() << endl;
		try
		{
			bool retStatus = entityDAO->refresh(obj);
			entityDAO->Passivate();
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
			return retStatus;
		}
		catch (exception& e)
		{
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			entityDAO->Passivate();
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pool.at(dynamic_pointer_cast<IObject>(entityDAO)->GetName().GetGrpId())->push(entityDAO);
			throw e;
		}
	}

	void RESTDataSource::flush(const std::shared_ptr<IObject>& obj)
	{
		LOG(ERROR) << " Invalid method for REST" << endl;
		throw std::logic_error("Invalid method flush for RESTDataSource");
	}

	void RESTDataSource::insert(const std::shared_ptr<IObject>& obj)
	{
		LOG(ERROR) << " Invalid method for REST" << endl;
		throw std::logic_error("Invalid method flush for RESTDataSource");
	}

	std::shared_ptr<IDAO> RESTDataSource::getDAO(const Name& nm, unsigned int source)
	{
		/// Get the DAO group ID for the given Entity type
		/// in this case the source is YAHOO		
		/// if DAO type is not registered for the entity interface ID
		/// it is an unrecoverable error. The caller need to handle
		/// thrown exception
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();
		grpType daoTypeId = entMgr.findDAO(source, nm.GetGrpId());
		return getDAO(daoTypeId);
	}

	std::shared_ptr<IDAO> RESTDataSource::getDAO(const std::shared_ptr<IObject>& obj, unsigned int source)
	{
		/// Get the DAO group ID for the given Entity type
		/// in this case the source is YAHOO		
		/// if DAO type is not registered for the entity interface ID
		/// it is an unrecoverable error. The caller need to handle
		/// thrown exception

		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();
		grpType daoTypeId = entMgr.findDAO(source, obj);
		return getDAO(daoTypeId);
	}

	std::shared_ptr<IDAO> RESTDataSource::getDAO(grpType daoGrpId)
	{
		/// if no Object pool created for the DAO then insert an ObjectPool
		/// instance that is associated with the given group ID
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_pool.find(daoGrpId) == m_pool.end())
		{
			lock.unlock();
			std::shared_ptr<ObjectPool<IDAO> > pool = std::make_shared<ObjectPool<IDAO> >(daoGrpId);
			lock.lock();
			m_pool.insert(std::pair<grpType, std::shared_ptr<ObjectPool<IDAO> > >(daoGrpId, pool));
		}
		lock.unlock();

		/// now check if there is a passive processor in the resource pool
		std::shared_ptr<IDAO> dao;
		lock.lock();
		if (m_pool.at(daoGrpId)->try_pop(dao))
		{
			return dao;
		}
		lock.unlock();

		/// Otherwise We need to construct DAO from DAO exemplar; get the examplar object for the DAO
		/// Exemplar objects should be initialized during global initialization time.
		try
		{
			/// Get the EntityManager instance
			EntityManager& entMgr = EntityManager::getInstance();
			/// we have daoGrpId, we can retrive exemplar for DAO
			/// if exemplar not registered with EntityManager, EntityManager
			/// would throw an exception and caller needs to handle the exception
			std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(daoGrpId));

			/// Now we have the exampler object. 
			/// Make the exemplar DAO to construct DAO for the given entity
			/// Remember each Entity object associated with a DAO
			lock.lock();
			auto objId = m_pool.at(daoGrpId)->increment();
			lock.unlock();
			assert(objId < dynamic_pointer_cast<IDAO>(exemplar)->GetMaxDAOCount());
			std::shared_ptr<IMake> obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(Name(daoGrpId, objId));
			std::shared_ptr<IDAO>  entityDAO = dynamic_pointer_cast<IDAO>(obj);
			return entityDAO;
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " EntityGroup for the given DAO type not found for " << daoGrpId << endl;
			LOG(WARNING) << e.what() << endl;
			throw e;
		}
	}
} /* namespace derivative */
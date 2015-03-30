/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/
#include "YahooDataSource.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "IDAO.hpp"
#include "IMake.hpp"
#include "GroupRegister.hpp"
#include "Global.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	namespace
	{
		GroupRegister YahooDataSourceGrp(YahooDataSource::TYPEID,  std::make_shared<YahooDataSource>(Exemplar())); 
		AliasRegister ar_IDataSource_Yahoo(YahooDataSource::TYPEID, IDataSource::TYPEID);
	}

	unsigned short YahooDataSource::maxPooledDAO = 10;
	YahooDataSource::YahooDataSource (const Exemplar &ex)
		:m_name(TYPEID), m_source(YAHOO),m_DAOCount(0)
	{
		LOG(INFO) << " Exemplar id constructed " << endl;
	}

	YahooDataSource::YahooDataSource (const Name& nm)
		:m_name(nm), m_source(YAHOO),m_DAOCount(0)
	{
		LOG(INFO) << " MySQLDataSource constructed with name " << nm << endl;
	}

	YahooDataSource::~YahooDataSource ()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	std::shared_ptr<IMake> YahooDataSource::Make(const Name &nm)
	{
		/// Construct data source from given name and register with EntityManager
		std::shared_ptr<YahooDataSource> dataSource = make_shared<YahooDataSource>(nm);
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerObject(nm,dataSource);

		/// return constructed object if no exception is thrown
		return dataSource;
	}

	std::shared_ptr<IMake> YahooDataSource::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This method is not supported");
	}

	std::shared_ptr<IObject> YahooDataSource::GetEntity(const Name &nm)     
	{
		/// Try to get the entity from yahoo through YahooDAO for the
		/// given Entity. Ex: StockValue will have a specific YahooDAO
		/// and HistoricStockValue will have different Yahoo DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from Yahoo
		LOG(INFO) << " Got Yahoo DAO " << entityDAO << " for " << nm << endl;
		try
		{
			std::shared_ptr<IObject>  entity = entityDAO->find(nm);
			/// before return, push the DAO into the queue
			m_DAOQueue.push(entityDAO);
			return entity;
		}
		catch(exception& e)
		{
			/// We need to catch all exceptions so that
			/// the DAO object can be returned to the pool
			m_DAOQueue.push(entityDAO);
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			throw e;
		}		
	}

	void YahooDataSource::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		/// Try to get the entity from yahoo through YahooDAO for the
		/// given Entity. Ex: StockValue will have a specific YahooDAO
		/// and HistoricStockValue will have different Yahoo DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(nm);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from Yahoo
		LOG(INFO) << " Got Yahoo DAO " << entityDAO << " for " << nm << endl;
		try
		{
			entityDAO->find(nm, entities);
			/// before return, push the DAO into the queue
			m_DAOQueue.push(entityDAO);
		}
		catch(exception& e)
		{
			/// We need to catch all exceptions so that
			/// the DAO object can be returned to the pool
			m_DAOQueue.push(entityDAO);
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			throw e;
		}		
	}

	bool YahooDataSource::refreshObject(std::shared_ptr<IObject>& obj)
	{
		/// Try to get the entity from yahoo through YahooDAO for the
		/// given Entity. Ex: StockValue will have a specific YahooDAO
		/// and HistoricStockValue will have different Yahoo DAO
		std::shared_ptr<IDAO>& entityDAO = getDAO(obj);

		/// Now we have the DAO for the entity.
		/// Call on the DAO to to retrieve entity object from Yahoo
		LOG(INFO) << " Got Yahoo DAO " << entityDAO << " for " << obj->GetName() << endl;
		try
		{
			bool retStatus = entityDAO->refresh(obj);
			/// before return, push the DAO into the queue
			m_DAOQueue.push(entityDAO);
			return retStatus;
		}
		catch(exception& e)
		{
			/// We need to catch all exceptions so that
			/// the DAO object can be returned to the pool
			m_DAOQueue.push(entityDAO);
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			throw e;
		}		
	}

	void YahooDataSource::flush(const std::shared_ptr<IObject>& obj)
	{
		LOG(ERROR) << " Invalid method for Yahoo" << endl;
		throw std::logic_error("Invalid method flush for YahooDataSource");
	}

	void YahooDataSource::insert(const std::shared_ptr<IObject>& obj)
	{
		LOG(ERROR) << " Invalid method for Yahoo" << endl;
		throw std::logic_error("Invalid method flush for YahooDataSource");
	}

	std::shared_ptr<IDAO> YahooDataSource::getDAO(const Name& nm)
	{
		/// Get the DAO group ID for the given Entity type
		/// in this case the source is YAHOO		
		/// if DAO type is not registered for the entity interface ID
		/// it is an unrecoverable error. The caller need to handle
		/// thrown exception
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();
		grpType daoTypeId =  entMgr.findDAO(YAHOO, nm.GetGrpId());
		return getDAO(daoTypeId);
	}

	std::shared_ptr<IDAO> YahooDataSource::getDAO(const std::shared_ptr<IObject>& obj)
	{
		/// Get the DAO group ID for the given Entity type
		/// in this case the source is YAHOO		
		/// if DAO type is not registered for the entity interface ID
		/// it is an unrecoverable error. The caller need to handle
		/// thrown exception

		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();
		grpType daoTypeId =  entMgr.findDAO(YAHOO, obj);
		return getDAO(daoTypeId);
	}

	std::shared_ptr<IDAO> YahooDataSource::getDAO(grpType daoGrpId)
	{
		/// check if the queue is empty. If not empty then
		/// simply pop a DAO from the queue and return.

		/// If queue is empty and the count of DAO is less then
		/// max count then create a new DAO and return

		/// Otherwise let the caller wait for free up of a DAO

		/// Allow only one thread enter this member function
		/// at a time.
		std::lock_guard<std::mutex> guard(m_mutex);

		if (!m_DAOQueue.empty())
		{
			return m_DAOQueue.wait_and_pop();
		}

		/// if empty then either we have to construct a new DAO
		/// object if max count not reached. otherwise go in blocked wait		
		if (m_DAOCount < maxPooledDAO)
		{
			/// We need to construct DAO from DAO exemplar
			/// get the examplar object for the DAO
			/// Exemplar objects should be initialized
			/// during global initialization time.
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
				std::shared_ptr<IMake> obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(Name(daoGrpId, ++m_DAOCount));

				std::shared_ptr<IDAO> entityDAO = dynamic_pointer_cast<IDAO>(obj);
				return entityDAO;
			}
			catch(RegistryException& e)
			{
				LOG(WARNING) << " EntityGroup for the given DAO type not found for " << daoGrpId << endl;
				LOG(WARNING)  << e.what() << endl;
				throw e;
			}
		}
		else
		{
			/// go in waiting state with the ThreadSafeQueue
			return m_DAOQueue.wait_and_pop();
		}
	}
} /* namespace derivative */
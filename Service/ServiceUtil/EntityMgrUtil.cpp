/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <deque>

#include "Global.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "EntityMgrUtil.hpp"
#include "IObject.hpp"
#include "Name.hpp"

namespace derivative
{
	std::shared_ptr<IObject> EntityMgrUtil::registerObject(const Name& nm, const std::shared_ptr<IObject> &obj)
	{
		/// get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();
		try
		{
			return entMgr.registerObject(nm, obj);
		}
		catch (RegistryException e)
		{
			LOG(WARNING) << nm << " already registered with EntityManager " << endl;
			LOG(WARNING) << " Registration exception is thrown with message " << e.what() << endl;
			throw e;
		}
	}

	void EntityMgrUtil::registerObjects(const std::vector<std::shared_ptr<IObject> > &objs)
	{
		/// get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();

		entMgr.registerObjects(objs);
	}

	/// Find by primary key
	std::shared_ptr<IObject> EntityMgrUtil::findObject(const Name& nm, unsigned short source)
	{
		/// get the EntityManager instance
		/// if exception thrown then let the client handle the 
		/// exception. We can't proceed from exception
		EntityManager& entMgr = EntityManager::getInstance();

		/// get the concsrete Name for the given alias Name
		unsigned int concreteType = entMgr.findAlias(nm);
		Name concreteName = nm;
		if (concreteType > 0)
		{
			concreteName = Name(concreteType, nm.GetObjId());
		}

		/// check if the object is already registered with EntityManager
		try
		{
			std::shared_ptr<IObject> obj = entMgr.findObject(concreteName);
			if (obj)
			{
				return obj;
			}

			/// no object with same type or child type found
			/// in registry. We have to fetch from database			
			LOG(INFO) << " Attempt to fetch from data store " << endl;
			obj = fetch(nm, source);
			return obj;
		}
		catch (RegistryException& e)
		{
			/// the object is not in registry
			/// need to pick up from the external source
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			throw e;
		}
	}

	/// Find by primary key
	void EntityMgrUtil::findObjects(const Name& nm, std::vector<std::shared_ptr<IObject> >& entities, unsigned short source)
	{
		/// get the EntityManager instance
		/// if exception thrown then let the client handle the 
		/// exception. We can't proceed from exception
		EntityManager& entMgr = EntityManager::getInstance();

		/// get the concrete Name for the given alias Name
		unsigned int concreteType = entMgr.findAlias(nm);
		Name concreteName = nm;
		if (concreteType > 0)
		{
			concreteName = Name(concreteType, nm.GetObjId());
		}
		try
		{
			LOG(INFO) << " Attempt to fetch from data store " << endl;
			fetch(nm, entities, source);
		}
		catch (RegistryException& e)
		{
			/// the object is not in registry
			/// need to pick up from the external source
			LOG(ERROR) << " Exception thrown " << e.what() << endl;
			throw e;
		}
	}

	std::vector<std::shared_ptr<IObject> > EntityMgrUtil::findObjects(grpType id)
	{
		/// get the EntityManager instance
		/// if exception thrown then let the client handle the 
		/// exception. We can't proceed from exception
		EntityManager& entMgr = EntityManager::getInstance();

		/// get the concsrete Name for the given alias Name
		Name nm(id, 0);

		/// check if the object is already registered with EntityManager
		return entMgr.findObjects(entMgr.findAlias(nm));
	}

	bool EntityMgrUtil::refreshObject(std::shared_ptr<IObject>& obj, unsigned short source)
	{
		/// check if the object is already registered with EntityManager

		/// get the data source corresponding to the source paramter
		std::shared_ptr<IDataSource> dataSrc = getDataSourceHandler(obj->GetName(), source);

		/// get the data src to perform refresh
		return dataSrc->refreshObject(obj, source);
	}

	std::shared_ptr<IObject> EntityMgrUtil::fetch(const Name& nm, unsigned short source)
	{
		std::shared_ptr<IDataSource> dataSrc = getDataSourceHandler(nm, source);

		/// Using MySQLDataSource, fetch the data from MySQL Database
		/// MySQLDataSource would use the DAO to atcually do the fetch

		LOG(INFO) << "Using dataSrc " << dataSrc->GetName() << " fetch the given object " << nm << endl;

		/// Any exception thrown will be propogated to caller
		/// The exceptions could be from Derivative exceptions to
		/// MySQL C++ connector exceptions.
		std::shared_ptr<IObject>  entity = dataSrc->GetEntity(nm, source);
		return entity;
	}

	void EntityMgrUtil::fetch(const Name& nm, std::vector<std::shared_ptr<IObject> >& entities, unsigned short source)
	{
		std::shared_ptr<IDataSource> dataSrc = getDataSourceHandler(nm, source);

		/// Using MySQLDataSource, fetch the data from MySQL Database
		/// MySQLDataSource would use the DAO to atcually do the fetch

		LOG(INFO) << "Using dataSrc " << dataSrc->GetName() << " fetch the given object " << nm << endl;

		/// Any exception thrown will be propogated to caller
		/// The exceptions could be from Derivative exceptions to
		/// MySQL C++ connector exceptions.
		dataSrc->find(nm, entities, source);
	}

	std::shared_ptr<IDataSource> EntityMgrUtil::getDataSourceHandler(const Name& nm, unsigned short source)
	{
		/// get the EntityManager instance
		/// if exception thrown then let the client handle the 
		/// exception. We can't proceed from exception
		EntityManager& entMgr = EntityManager::getInstance();

		/// Get the DataSource using IDataSource interface TYPEID
		/// if source is MYSQL then use 'source' as the object ID
		/// if the source is RESTful server) then the type ID
		/// of the entity should be used as the Object ID
		Name dataSrcInterfaceName = Name(IDataSource::TYPEID, source);
		std::vector<std::shared_ptr<IObject> > objs;
		try
		{
			entMgr.findAlias(dataSrcInterfaceName, objs);
			std::shared_ptr<IDataSource> dataSrc;
			for (std::shared_ptr<IObject> obj : objs)
			{
				if (dynamic_pointer_cast<IDataSource>(obj)->InSource(source))
				{
					LOG(INFO) << " Found data source for " << dataSrcInterfaceName << endl;
					dataSrc = dynamic_pointer_cast<IDataSource>(obj);
					return dataSrc;
				}
			}
		}
		catch (RegistryException e)
		{
			/// No concrete objects found with the given "ObjectID" and alias of IDataSource::TYPEID
			LOG(ERROR) << " No concerete type for DataSource found for interface type IDataSource" << endl;
		}

		/// if we are here then it means no DataSource object is not found in registry
		/// with given source and Object ID.
		/// (i.e, this is the first time the fetch using given DataSource
		/// is attempted. We need to find the Exemplar for DataSource
		/// Get Exemplar:
		///    find the concrete object for the given alias
		///    find the objects bound with DataSource concrete type
		std::vector<Name> names;
		entMgr.findAlias(dataSrcInterfaceName, names);

		if (names.empty())
		{
			LOG(ERROR) << " No concerete type for MySQL DataSource found for interface type IDataSource" << endl;
			throw RegistryException("No concerete type for MySQL DataSource found");
		}

		for (auto &dataSourceName : names)
		{
			try
			{
				/// get the examplar object for the DataSource from EntityManager
				std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(dataSourceName.GetGrpId()));

				/// Now we have the exampler object. 
				/// Make the exemplar to construct DataSource for the given type
				std::shared_ptr<IMake>& obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(dataSourceName);

				/// register with EntityManager now
				std::shared_ptr<IObject>& regObj = (dynamic_pointer_cast<IObject>(obj));
				regObj = EntityMgrUtil::registerObject(regObj->GetName(), regObj);

				/// cast IDataSource and return if the data source of the object is the same as
				/// requested parameter
				std::shared_ptr<IDataSource> dataSrc = dynamic_pointer_cast<IDataSource>(regObj);
				if (dataSrc->InSource(source))
				{
					return dataSrc;
				}
			}
			catch (RegistryException& e)
			{
				LOG(ERROR) << " No exemplar bound for MySQL data source" << endl;
				LOG(ERROR) << e.what() << endl;
				throw e;
			}
		}
		throw DataSourceException("Unable to find/create data source for the given source");
	}

	void EntityMgrUtil::unbind(std::shared_ptr<IObject> &obj)
	{
	}

	void EntityMgrUtil::flush(std::shared_ptr<IObject> &obj)
	{
		/// construct MySQL handler since it is the only data source
		/// used as the persistent data source
		std::shared_ptr<IDataSource> dataSrc = getDataSourceHandler(obj->GetName(), MYSQL);

		/// Using MySQLDataSource, fetch the data from MySQL Database
		/// MySQLDataSource would use the DAO to atcually do the fetch

		LOG(INFO) << "Using dataSrc " << dataSrc->GetName() << endl;

		/// Any exception thrown will be propogated to caller
		/// The exceptions could be from Derivative exceptions to
		/// MySQL C++ connector exceptions.
		dataSrc->flush(obj);
	}


} /* namespace derivative */
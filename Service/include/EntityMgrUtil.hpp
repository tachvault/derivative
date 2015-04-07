/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ENTITYMGRUTIL_H_
#define _DERIVATIVE_ENTITYMGRUTIL_H_
#pragma once

#include "Global.hpp"
#include "IDataSource.hpp"
#include "DException.hpp"

#if defined SERVICEUTIL_EXPORTS
#define SERVICE_UTIL_DLL_API __declspec(dllexport)
#else
#define SERVICE_UTIL_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	class Name;
	class IObject;
	class EntityManager;

	/// EntityMgrUtil is a utility class with static functions 
	/// designed to reduce client reduce coupling between Entity Manager 
	/// and other service later components. (Act as Mediator class)
	/// Ex: With Data source shared library. 
	class  SERVICE_UTIL_DLL_API EntityMgrUtil
	{

	public:	

		template <typename T>
		static std::shared_ptr<T> ConstructEntity(const Name& nm)
		{
			/// First get the T exemplar object from the registry
			/// Get the EntityManager instance
			EntityManager& entMgr = EntityManager::getInstance();

			/// get the concrete types for the given alias name
			std::vector<Name> names;
			entMgr.findAlias(nm, names);
			if (names.empty())
			{
				LOG(ERROR) << " No concrete name found for " << nm << endl;
				throw DataSourceException("type T not bound with correct alias");
			}

			Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

			/// copy the keys from Alias Name to concrete name
			entityName.SetKeys(nm.GetKeyMap());

			/// get the examplar object for the IRValue
			/// Exemplar objects should be initialized
			/// during global initialization time.
			std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

			/// Now we have the exampler object. 
			/// Make the exemplar T to construct T for the given name		
			std::shared_ptr<T> obj = dynamic_pointer_cast<T>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));

			return obj;
		};

		/// register an object that implements IObject pure virtual class.
		/// if the given object is already registered with the name
		/// then it will propogate RegistryException
		static std::shared_ptr<IObject> registerObject(const Name& nm, const std::shared_ptr<IObject> &obj);

		/// register a set of objects together.
		/// Useful routine when registering streaming data
		static void registerObjects(const std::deque<const std::shared_ptr<IObject> > &objs);

		/// get the MySQL DataSource object
		static std::shared_ptr<IDataSource> getDataSourceHandler(const Name& nm,unsigned short source);

		/// Find an object using its name. The object ID should
		/// indicate primary key in the data store if
		/// if the object is not already bound in registry
		static std::shared_ptr<IObject> findObject(const Name& nm, unsigned short source = MYSQL);

		/// Find an objects from the registry given the object type
		static std::vector<std::shared_ptr<IObject> > findObjects(grpType id);

		/// Find an objects using its name. This call will directly fetch data from
		/// the external data store. There will not be attempt made to check the internal
		/// registry. Appropriate for loading bulk data during system startup.
        static void findObjects(const Name& nm, std::vector<std::shared_ptr<IObject> >& entities, unsigned short source = MYSQL);

		/// refresh and object that is in the memory.
		/// if the object is not in the memory in return false.
		static bool refreshObject(shared_ptr<IObject>& obj, unsigned short source);

		/// Unbind an Entity in memory (with he EntityMgrUtil).
		/// If the unound object is referenced by other objects then
		/// the unbound object would still be accessible through other objects
		static void unbind(std::shared_ptr<IObject> &obj);

		/// flush the data to the pesistence store (database)
		static void flush(std::shared_ptr<IObject> &obj);

		/// this static member function is resonsible
		/// for fetching named object from external source
		static std::shared_ptr<IObject> fetch(const Name& nm, unsigned short source);

		/// this static member function is resonsible
		/// for fetching named objects from external source
		static void fetch(const Name& nm, std::vector<std::shared_ptr<IObject> >& entities, unsigned short source);

	private:
		
		/// construct DataSourceHandler object ID from source and Name.GetGrpId()
		size_t constructObjectID(const Name& nm, unsigned short source);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_ENTITYMGRUTIL_H_ */

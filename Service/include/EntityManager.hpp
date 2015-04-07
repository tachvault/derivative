/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#pragma once
#ifndef _DERIVATIVE_ENTITYMANAGER_H_
#define _DERIVATIVE_ENTITYMANAGER_H_
#pragma warning (push)
#pragma warning (disable : 4251)

#include <memory>
#include <mutex>
#include <limits>   
#include <unordered_map>
#include <deque>

#include "Global.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "EntityGroup.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef ENTITYMGMT_EXPORTS
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllexport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllimport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ENTITY_MGMT_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define ENTITY_MGMT_DLL_API __attribute__ ((visibility ("default")))
    #define ENTITY_MGMT_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define ENTITY_MGMT_DLL_API
    #define ENTITY_MGMT_DLL_LOCAL
  #endif
#endif

namespace derivative
{
	/// EntityManager is a singleton object responsible for
	/// maintaining named object identity and 
	/// entity object persistence.
	class  ENTITY_MGMT_DLL_API EntityManager
	{

	public:	

		/// Hash map type for <Alias object ID (IStock), Concrete Group ID (Ex: Stock)>
		/// Also used in <Concrete Group ID (Ex: Stock), Alias object ID (IStock)>
		typedef std::unordered_multimap<grpType, grpType> HashAliasType;

		/// Hash map type for <Source, Alias Group ID (Ex: MYSQL, IStock), Data Access Object(StockMySQLDAO)>
		typedef std::map<std::pair<ushort, grpType>, grpType> HashDAOType;

		/// Hash map type for <grpType, std::shared_ptr<EntityGroup> >
		/// Note: the grpType is concrete type (not interface type). Ex: Stock (not IStock)
		typedef std::unordered_map<grpType, std::shared_ptr<EntityGroup> > HashEntityGroupType;

		/// destructor
		~EntityManager();

		/// Get the EntityManager singleton instance
		static EntityManager& getInstance();

		/// Register an object with the entity manager
		std::shared_ptr<IObject> registerObject(const Name& nm, const std::shared_ptr<IObject> &obj);

		/// Register multiple objects together.
		/// This more efficient when streaming data is
		/// registered together from external data sources.
		void registerObjects(const std::deque<const std::shared_ptr<IObject> > &objs);

		/// Register alias with the concerete Id.
		void registerAlias(grpType concreteId, grpType aliasId);

		/// Register DAO.
		void registerDAO(grpType entityId, ushort source, grpType DAOId);

		/// Find by primary key
		/// If the object in the memory then EntityManager will return the object
		/// Otherwise the EntityManager will get the DAO to retrieve the object and
		/// related objects from database into memory.
		/// The DAO also would register the retirved objects with the EntityManager
		/// Only the objects that are not already registered will be registered.
		std::shared_ptr<IObject> findObject(const Name& nm);

		/// return objects for the given group type
		std::vector<std::shared_ptr<IObject> > findObjects(grpType id);

		/// return the first conceret name for the alias
		/// the return value would be random if the given
		/// alias is in higher level (Ex: IAsset)
	    unsigned int findAlias(const Name& nm);

		/// given my interface name
		/// use alias to resolve the concrete types and
		/// return the set of objects as vector
		/// It is upto the caller to figure out the actual concrete type
		// with dynamic_pointer_cast ?? 
		/// (Since IAsset could have Stock and Bond as concrete types)
		bool findAlias(const Name& nm, std::vector<std::shared_ptr<IObject> >& objs);

		/// given my interface name
		/// use alias to resolve the concrete types
		bool findAlias(const Name& nm, std::vector<Name> &names);

		/// find DAO grpId for a given entity grpId
		/// the entity grpId could be that concrete or interface
		grpType findDAO(ushort source, grpType aliasId);

		/// This is necessary when we wany to DAO for a given object
		grpType findDAO(ushort source, const std::shared_ptr<IObject>& obj);

		/// Unbind an Entity in memory (with he EntityManager).
		/// If the unound object is referenced by other objects then
		/// the unbound object would still be accessible through other objects
		void unbind(std::shared_ptr<IObject> &obj);

		/// Check if the instance is a managed entity instance belonging to the current persistence context.
		bool contains(std::shared_ptr<IObject> &obj);

	private:

		/// constructor
		EntityManager();

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(EntityManager);

		/// EntityManager singleton instance
		static unique_ptr<EntityManager> m_instance;

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		/// Provides mutually exclusive access to 
		/// EntityManager class members.
		std::mutex m_mutex;

		HashEntityGroupType  m_registry;   

		/// Alias registry stores alias type id as key and concrete type id as value
		/// Hash map type for <Alias object ID (IStock), Concrete Group ID (Ex: Stock)>
		/// example: <IStock, Stock>
		/// example: <IAsset, Stock>
		/// example: <IAsset, Bond>
		HashAliasType m_aliasToConcrete;

		/// Alias registry stores concrete type id as value and alias type id as key
		/// Hash map type for <Concrete Group ID (Ex: Stock), Alias object ID (IStock)>
		/// example: <Stock, IStock>
		/// example: <Stock, IAsset>
		HashAliasType m_concreteToAlias;

		/// DAO registry stores business object type id as key and DAO type id as value		
		/// example: <std::pair<YAHOO, IStock>, StockDAO>
		HashDAOType m_DAO;
	};

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_IENTITYMANAGER_H_ */

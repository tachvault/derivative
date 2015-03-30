/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ENTITYGROUP_H_
#define _DERIVATIVE_ENTITYGROUP_H_
#pragma once

#include <memory>
#include <unordered_map>
#include <deque>
#include <mutex>

#include "IObject.hpp"
#include "Global.hpp"

namespace derivative
{
	/// EntityManager delegates managing group of objects with same type (having same type identifier)
	class EntityGroup
	{

	public:	

		/// Hash map type for <Object ID, Named object pointer>
		typedef std::unordered_map<std::size_t, std::shared_ptr<IObject> > HashObjectType;

		/// Register an object with the entity manager
		void registerObject(const Name& nm, const std::shared_ptr<IObject> &obj);

		/// Register multiple objects together.
		/// This more efficient when streaming data is
		/// registered together from external data sources.
		void EntityGroup::registerObjects(const std::deque<const std::shared_ptr<IObject> > &objs);

		/// Find by primary key
		std::shared_ptr<IObject> findObject(const Name& nm);

		/// return all the objects
		/// return by value and let the compiler do the optmization.
		std::vector<std::shared_ptr<IObject> > findObjects();

		/// Unbind an Entity in memory (with he EntityManager).
		/// If the unound object is referenced by other objects then
		/// the unbound object would still be accessible through other objects
		void unbind(std::shared_ptr<IObject> &obj);

		/// Check if the instance is a managed entity instance belonging to the current persistence context.
		bool contains(std::shared_ptr<IObject> &obj);

		/// define default constructor
		EntityGroup();

		/// define destructor
		~EntityGroup();
		
	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(EntityGroup);

		/// Keeps exemplar object created during global
		/// initialization
		shared_ptr<IObject> m_exemplar;

		/// Provides mutually exclusive access to 
		/// EntityGroup class members.
		std::mutex m_mutex;

		/// unordered map of 
		/// std::unordered_map<Grp ID, std::shared_ptr<IObject> >
		HashObjectType m_objRegistry;		
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IENTITYMANAGER_H_ */

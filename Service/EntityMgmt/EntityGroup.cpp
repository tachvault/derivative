/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include <functional> 
#include <algorithm>
#include "Name.hpp"
#include "EntityGroup.hpp"
#include "DException.hpp"

namespace derivative
{
	std::shared_ptr<IObject> EntityGroup::registerObject(const Name& nm, const std::shared_ptr<IObject> &obj)
	{
		/// Lock for access to private members 
		std::lock_guard<std::mutex> guard(m_mutex);

		/// check if the name is for exemplar object.
		/// if object is an exemplar then we don't need
		/// to add the object into the object registry
		/// just set the exemplar member variable pointing
		/// to the given object
		if (nm.isExemplar())
		{
			m_exemplar = obj;
			LOG(INFO) << "Set the Exemplar object " << endl;
			return obj;
		}

		/// Get the iterator for the key nm.ObjId
		auto i = m_objRegistry.find(nm.GetObjId());

		/// If the given object not in the map
		if (i == m_objRegistry.end()) 
		{
			/// insert the given Name and IObject
			m_objRegistry.insert(std::make_pair(nm.GetObjId(), obj));
			LOG(INFO) << "Added object with Created new EntityGroup for" << nm << endl;
			return obj;
		}
		else
		{
			/// an object with the given key (object ID) already bound)
			LOG(INFO) << "Name already registered for " << nm << endl;
			return i->second;
		}
	}

	void EntityGroup::registerObjects(const std::deque<const std::shared_ptr<IObject> > &objs)
	{
		/// Lock for access to private members 
		std::lock_guard<std::mutex> guard(m_mutex);

		/// merge the objs with m_objRegistry
		for (auto obj: objs)
		{
			/// Get the iterator for the key nm.ObjId
			auto i = m_objRegistry.find(obj->GetName().GetObjId());

			/// If the given object not in the map
			if (i == m_objRegistry.end()) 
			{
				/// insert the given Name and IObject
				m_objRegistry.insert(std::make_pair(obj->GetName().GetObjId(), obj));
				LOG(INFO) << "Added object with Created new EntityGroup for" << obj->GetName().GetObjId() << endl;
			}
		}
	}

	bool EntityGroup::contains(shared_ptr<IObject>& obj)
	{
		/// Lock for access to private members
		std::lock_guard<std::mutex> guard(m_mutex);

		/// if the object is the exemplar then return true
		if (m_exemplar == obj)
		{
			LOG(INFO) << "Given object is the exemplar of group " << obj->GetName() << endl;
			return true;
		}

		/// if we are here then it means the given object is
		/// not exemplar.
		/// See if the entry already exists
		Name nm = obj->GetName();
		auto j = m_objRegistry.find(nm.GetObjId());

		/// if the Object in the registry then return true
		if (m_objRegistry.find(nm.GetObjId()) != m_objRegistry.end())
		{		
			LOG(INFO) << "Given object found in registry " << obj->GetName() << endl;
			return true;
		}
		LOG(WARNING) << "Given object not found in registry " << obj->GetName() << endl;
		return false;
	}

	shared_ptr<IObject> EntityGroup::findObject(const Name& nm)
	{
		/// Lock for access to private members
		std::lock_guard<std::mutex> guard(m_mutex);

		/// check if the name is for exemplar object.
		/// if object is an exemplar then return the
		/// exemplar object from attribute m_exemplar
		if (nm.isExemplar())
		{
			LOG(INFO) << "Name is for Exemplar. Return Exemplar for " << m_exemplar->GetName() << endl;
			return m_exemplar;
		}

		/// the name is not exemplar's name
		/// See if the Object group for given Name nm exists
		auto j = m_objRegistry.find(nm.GetObjId());

		/// if the Object group in the registry then proceed
		/// to find the object
		if (j != m_objRegistry.end())
		{
			/// return the object
			LOG(INFO) << "Object found in registry. Return object for " << j->second->GetName() << endl;
			return j->second;
		}

		LOG(INFO) << "Object not found in registry with name " << nm << endl;

		/// if the object for the given name is not in the memory
		/// then client look for the object in the external source
		/// let the client choose the data source and find the object

		return nullptr;
	}

	/// return all the objects
	std::vector<std::shared_ptr<IObject> > EntityGroup::findObjects()
	{
		std::vector<std::shared_ptr<IObject> > objs;
		for (auto &p : m_objRegistry)
		{
			objs.push_back(p.second);
		}
		return objs;
	}

	void EntityGroup::unbind(std::shared_ptr<IObject> &obj)
	{
		/// Lock for access to private members of
		/// this entityGroup's member attributes
		std::lock_guard<std::mutex> guard(m_mutex);

		/// See if the entry already exists
		Name nm = obj->GetName();
		auto j = m_objRegistry.find(nm.GetObjId());

		/// if the object group exists then  erase the object 
		if (j != m_objRegistry.end())
		{
			LOG(INFO) << "Object " << obj->GetName()  << " found in registry and is unbound  = " << endl;
			m_objRegistry.erase(nm.GetObjId());
		}
		else
		{
			LOG(ERROR) << "Object " << obj->GetName()  << " not found in registry and is unbound  = " << endl;
		}
	}

	EntityGroup::EntityGroup()
	{
		LOG(INFO) << "EntityGroup constructor is called " << endl;
	}

	EntityGroup::~EntityGroup()
	{
		LOG(INFO) << "EntityGroup destructor is called " << endl;
	}

} /* namespace derivative */
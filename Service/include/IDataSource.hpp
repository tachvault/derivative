/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDATASOURCE_H_
#define _DERIVATIVE_IDATASOURCE_H_
#pragma once

#include <memory>
#include <deque>
#include "Name.hpp"
#include "IMake.hpp"
#include "ClassType.hpp"

namespace derivative
{
	/// define the types of data sources
	enum { MYSQL = 1, YAHOO = 2, XIGNITE = 3 };

	// IDataSource interface for external data source
	// for entity objects.
	class IDataSource : virtual public IMake
	{
	public:

		enum {TYPEID = INTERFACE_DATASOURCE_TYPE};

		/// return the source
		virtual bool InSource(unsigned int source)  const = 0;

		/// given an interface name (Ex: Name(IStock::TYPEID, key)) the 
		/// data source is required to return the concrete object
		virtual std::shared_ptr<IObject> GetEntity(const Name &nm, unsigned int source) = 0;

		/// given an interface name (Ex: Name(IStock::TYPEID, key)) the 
		/// data source is required to return the concrete objects
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities, unsigned int source) = 0;

		/// flush the entity to the external source
		/// this function is applicable only for persistence sources
		/// such as MySQL database. 
		virtual void flush(const std::shared_ptr<IObject>& obj) = 0;

		/// insert the object into database (MySQL) 
		virtual void insert(const std::shared_ptr<IObject>& obj) = 0;

		/// perform refresh
		virtual bool refreshObject(std::shared_ptr<IObject>& obj, unsigned int source) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDataSource() 
		{
		}  
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IDATASOURCE_H_ */

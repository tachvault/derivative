/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAO_H_
#define _DERIVATIVE_IDAO_H_

#include <memory>

namespace derivative
{
	class IObject;

	/// IStockDAO interface for persistency 
	/// of Stock entity object.
	class IDAO
	{

		enum {TYPEID = INTERFACE_DAO_TYPE};

	public:
		
		/// max DAO objects allowed per a DAO type (concrete type)
		virtual int GetMaxDAOCount() const = 0;

		/// once serviced, DAO need to be cleared of any member variable
		/// references.
		virtual void Passivate() = 0;

		/// insert associated entity into the data source
		virtual void insert() = 0;

		/// remove the associted entity from the data source
		/// and unbind the entity from the EntityManager
		/// Also unbind itself from the EntityManager
        virtual bool del() = 0;

		/// Given a name find the entity from the data source 
		/// If the name is passed then it is essential that
		/// name is set with Keys as well. Otherwise the
		/// DAOs cannot find the entity by keys
        virtual const std::shared_ptr<IObject> find(const Name& nm) = 0;

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
        virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> >& entities) = 0;

		/// update the entity attributes into data source
		virtual bool update() = 0;

		/// refresh the given object from yahoo
		virtual bool refresh(std::shared_ptr<IObject>& obj) = 0;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_I`STOCKDAO_H_ */

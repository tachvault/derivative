/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_LIBORVALUEXIGNITEDAO_H_
#define _DERIVATIVE_LIBORVALUEXIGNITEDAO_H_

#include <memory>
#include <ppltasks.h>

#include "IDAO.hpp"
#include "IMake.hpp"
#include "IIBORValue.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for LIBORValue entity.
	class LIBORValueXigniteDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_LIBORRATEVALUEXIGNITEDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator LIBORValueXigniteDAO object
		LIBORValueXigniteDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		LIBORValueXigniteDAO(const Name& daoName)
			:m_name(daoName)
		{
		}

		/// IObject::GetName()
		virtual const Name& GetName()
		{
			return m_name;
		}	

		/// IMake::Make (const Name& nm)
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		/// IMake::Make (const Name& nm, const std::deque<boost::any>& agrs) with additional
		/// arguments
		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs)
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for LIBORValue entity");
		}

		/// IDAO method for inserting LIBORValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for LIBORValue entity");
		}

		/// /// IDAO method for removing LIBORValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for LIBORValue entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::find(..) not applicable for LIBORValue entity");
		}

		/// IDAO method for updating data source with LIBORValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for LIBORValue entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for LIBORValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for LIBORValue entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_value = nullptr;
		}

	private:

		/// Populate the interestRate specific attributes
		Concurrency::task_status  findLIBORValue(const dd::date& issueDate);

		Name m_name;

		// Associated interestRate entity
		std::shared_ptr<IIBORValue> m_value;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_LIBORVALUEXIGNITEDAO_H_ */

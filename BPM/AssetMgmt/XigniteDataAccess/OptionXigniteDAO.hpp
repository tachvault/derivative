/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/
	
#ifndef _DERIVATIVE_IRVALUEXIGNITEDAO_H_
#define _DERIVATIVE_IRVALUEXIGNITEDAO_H_

#include <ppltasks.h>
#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for Option entity.
	class OptionXigniteDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_OPTIONXIGNITEDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator OptionXigniteDAO object
		OptionXigniteDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		OptionXigniteDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for Option entity");
		}

		/// IDAO method for inserting Option object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for Option entity");
		}

		/// /// IDAO method for removing Option object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for Option entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::find(..) not applicable for Option entity");
		}

		/// IDAO method for updating data source with Option values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Option entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for Option entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Option entity");
		}

	private:

		/// Populate the interestRate specific attributes
		Concurrency::task_status  findOption(const Name& nm);

		Name m_name;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IRVALUEXIGNITEDAO_H_ */

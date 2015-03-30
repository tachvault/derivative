/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEYAHOODAO_H_
#define _DERIVATIVE_EXCHANGERATEYAHOODAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "IExchangeRateValue.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for ExchangeRateValue entity.
	class ExchangeRateValueYahooDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_EXCHANGERATEVALUEYAHOODAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator ExchangeRateValueYahooDAO object
		ExchangeRateValueYahooDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		ExchangeRateValueYahooDAO(const Name& daoName)
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

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// IDAO method for inserting ExchangeRateValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for ExchangeRateValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for ExchangeRateValue entity");
		}

		/// /// IDAO method for removing ExchangeRateValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for ExchangeRateValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for ExchangeRateValue entity");
		}

		/// IDAO method for fetching exchangeRateVal from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj);

		/// IDAO method for updating data source with ExchangeRateValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for ExchangeRateValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ExchangeRateValue entity");
		}

	private:

		/// Populate the exchangeRateVal specific attributes
		void findExchangeRateValue();

		Name m_name;

		/// Associated exchangeRateVal entity
		std::shared_ptr<IExchangeRateValue> m_exchangeRateVal;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEYAHOODAO_H_ */

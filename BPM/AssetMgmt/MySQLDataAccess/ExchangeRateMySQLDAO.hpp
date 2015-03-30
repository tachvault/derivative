/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEMYSQLDAO_H_
#define _DERIVATIVE_EXCHANGERATEMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IExchangeRate.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for ExchangeRate entity.
	class ExchangeRateMySQLDAO : virtual public IDAO,
		virtual public IObject,
		virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_EXCHANGERATEMYSQLDAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator ExchangeRateMySQLDAO object
		ExchangeRateMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}

		/// Constructor for IMake::Make(..)
		ExchangeRateMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for ExchangeRate entity");
		}

		/// IDAO method for inserting ExchangeRate object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for ExchangeRate entity");
		}

		/// /// IDAO method for removing ExchangeRate object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for ExchangeRate entity");
		}

		/// IDAO method for fetching exchangeRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with ExchangeRate values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ExchangeRate entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ExchangeRate entity");
		}

	private:

		/// Populate the exchangeRate specific attributes
		void findExchangeRate();

		/// Populate the primitive asset specific attributes
		/// shared by all primitive assets
		void findExchange();

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated exchangeRate entity
		std::shared_ptr<IExchangeRate> m_exchangeRate;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEMYSQLDAO_H_ */

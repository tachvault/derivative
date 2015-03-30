/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DAILYEQUITYOPTIONVALUEMYSQLDAO_H_
#define _DERIVATIVE_DAILYEQUITYOPTIONVALUEMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for DailyEquityOptionValue entity.
	class DailyEquityOptionValueMySQLDAO : virtual public IDAO,
		virtual public IObject,
		virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_DAILYEQUITYOPTIONVALUEMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator DailyEquityOptionValueMySQLDAO object
		DailyEquityOptionValueMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}

		/// Constructor for IMake::Make(..)
		DailyEquityOptionValueMySQLDAO(const Name& daoName)
			:m_name(daoName)
		{
		}

		/// IObject::GetName()
		virtual const Name& GetName()
		{
			return m_name;
		}	

		/// Standard functionality common in all DAOs
		/// for constructing Entity until Name() attribute
		/// filled. 
		std::shared_ptr<IDailyEquityOptionValue> constructEntity(const Name& nm);

		/// IMake::Make (const Name& nm)
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		/// IMake::Make (const Name& nm, const std::deque<boost::any>& agrs) with additional
		/// arguments
		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs)
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for DailyEquityOptionValue entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for DailyEquityOptionValue entity");
		}

		/// IDAO method for inserting DailyEquityOptionValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for DailyEquityOptionValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for DailyEquityOptionValue entity");
		}

		/// /// IDAO method for removing DailyEquityOptionValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for DailyEquityOptionValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for DailyEquityOptionValue entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source. Given a trade trade this call will pick up
		/// all the data closest in trade date for the given date.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// IDAO method for updating data source with DailyEquityOptionValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for DailyEquityOptionValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for DailyEquityOptionValue entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for DailyEquityOptionValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for DailyEquityOptionValue entity");
		}

	private:

		/// Populate the options from the database
		void findDailyEquityOptionValue(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// Populate the option attributes from the database for the given value
		std::shared_ptr<IObject> findOption(const Name& nm);

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_DAILYEQUITYOPTIONVALUEMYSQLDAO_H_ */

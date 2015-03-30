/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FIXEDRATEBONDVALUEMYSQLDAO_H_
#define _DERIVATIVE_FIXEDRATEBONDVALUEMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IFixedRateBondValue.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for FixedRateBondValue entity.
	class FixedRateBondValueMySQLDAO : virtual public IDAO,
		virtual public IObject,
		virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_FIXEDRATEBONDVALUEMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator FixedRateBondValueMySQLDAO object
		FixedRateBondValueMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}

		/// Constructor for IMake::Make(..)
		FixedRateBondValueMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for FixedRateBondValue entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for FixedRateBondValue entity");
		}

		/// IDAO method for inserting FixedRateBondValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for FixedRateBondValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for FixedRateBondValue entity");
		}

		/// /// IDAO method for removing FixedRateBondValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for FixedRateBondValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for FixedRateBondValue entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source. Given a trade trade this call will pick up
		/// all the data closest in trade date for the given date.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// IDAO method for updating data source with FixedRateBondValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for FixedRateBondValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for FixedRateBondValue entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for FixedRateBondValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for FixedRateBondValue entity");
		}

	private:

		std::shared_ptr<IFixedRateBondValue> constructEntity(const Name& nm);

		/// Populate the Zero coupon bonds from the database
		void findFixedRateBondValue(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// Populate the Zero coupon bond attributes from the database for the given value
		std::shared_ptr<IObject> findBond(std::shared_ptr<IFixedRateBondValue>& value);

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FIXEDRATEBONDVALUEMYSQLDAO_H_ */

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ZEROCOUPONBONDVALUEMYSQLDAO_H_
#define _DERIVATIVE_ZEROCOUPONBONDVALUEMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IZeroCouponBondValue.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for ZeroCouponBondValue entity.
	class ZeroCouponBondValueMySQLDAO : virtual public IDAO,
		virtual public IObject,
		virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_ZEROCOUPONBONDVALUEMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator ZeroCouponBondValueMySQLDAO object
		ZeroCouponBondValueMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}

		/// Constructor for IMake::Make(..)
		ZeroCouponBondValueMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for ZeroCouponBondValue entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for ZeroCouponBondValue entity");
		}

		/// IDAO method for inserting ZeroCouponBondValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for ZeroCouponBondValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for ZeroCouponBondValue entity");
		}

		/// /// IDAO method for removing ZeroCouponBondValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for ZeroCouponBondValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for ZeroCouponBondValue entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source. Given a trade trade this call will pick up
		/// all the data closest in trade date for the given date.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// IDAO method for updating data source with ZeroCouponBondValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for ZeroCouponBondValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ZeroCouponBondValue entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for ZeroCouponBondValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ZeroCouponBondValue entity");
		}

	private:

		std::shared_ptr<IZeroCouponBondValue> constructEntity(const Name& nm);

		/// Populate the Zero coupon bonds from the database
		void findZeroCouponBondValue(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// Populate the Zero coupon bond attributes from the database for the given value
		std::shared_ptr<IObject> findBond(std::shared_ptr<IZeroCouponBondValue>& value);

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_ZEROCOUPONBONDVALUEMYSQLDAO_H_ */

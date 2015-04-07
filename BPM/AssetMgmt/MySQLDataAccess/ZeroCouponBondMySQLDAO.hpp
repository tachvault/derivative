/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ZEROCOUPONBONDMYSQLDAO_H_
#define _DERIVATIVE_ZEROCOUPONBONDMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IZeroCouponBond.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for ZeroCouponBond entity.
	class ZeroCouponBondMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_ZEROCOUPONBONDMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator ZeroCouponBondMySQLDAO object
		ZeroCouponBondMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		ZeroCouponBondMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for ZeroCouponBond entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for ZeroCouponBond entity");
		}

		/// IDAO method for inserting ZeroCouponBond object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for ZeroCouponBond entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for ZeroCouponBond entity");
		}

		/// /// IDAO method for removing ZeroCouponBond object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for ZeroCouponBond entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for ZeroCouponBond entity");
		}

		/// IDAO method for fetching interestRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with ZeroCouponBond values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for ZeroCouponBond entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ZeroCouponBond entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for ZeroCouponBond entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for ZeroCouponBond entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_rate = nullptr;
		}

	private:

		/// Populate the interestRate specific attributes
		void findZeroCouponBond(const Name& name);

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated interestRate entity
		std::shared_ptr<IZeroCouponBond> m_rate;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_ZEROCOUPONBONDMYSQLDAO_H_ */

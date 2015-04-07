/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_STOCKMYSQLDAO_H_
#define _DERIVATIVE_STOCKMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IStock.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for Stock entity.
	class StockMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_STOCKMYSQLDAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator StockMySQLDAO object
		StockMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		StockMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for Stock entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for Stock entity");
		}

		/// IDAO method for inserting Stock object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for Stock entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for Stock entity");
		}

		/// /// IDAO method for removing Stock object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for Stock entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for Stock entity");
		}

		/// IDAO method for fetching stock from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with Stock values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for Stock entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Stock entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for Stock entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Stock entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_stock = nullptr;
		}

	private:

		/// Populate the stock specific attributes
		void findStock(const Name& nm);

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated stock entity
		std::shared_ptr<IStock> m_stock;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_STOCKMYSQLDAO_H_ */

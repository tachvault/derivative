/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESVALUEMYSQLDAO_H_
#define _DERIVATIVE_FUTURESVALUEMYSQLDAO_H_

#include <memory>
#include "MySqlConnection.hpp"
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "IFuturesValue.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for FuturesValue entity.
	class FuturesValueMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum { TYPEID = CLASS_FUTURESVALUEMYSQLDAO_TYPE };

		/// constructors.

		/// Constructor with Exemplar for the Creator FuturesValueMySQLDAO object
		FuturesValueMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		FuturesValueMySQLDAO(const Name& daoName)
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

		/// IDAO method for inserting FuturesValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for FuturesValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for FuturesValue entity");
		}

		/// /// IDAO method for removing FuturesValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for FuturesValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for FuturesValue entity");
		}

		/// IDAO method for fetching futuresVal from data source
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for FuturesValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for FuturesValue entity");
		}

		/// IDAO method for updating data source with FuturesValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for FuturesValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for FuturesValue entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_futures = nullptr;
			m_futuresVal = nullptr;
		}

	private:

		/// Populate the futuresVal specific attributes
		void findFuturesValue();

		Name m_name;

		/// Associated futuresVal entity
		std::shared_ptr<IFuturesValue> m_futuresVal;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated futures entity
		std::shared_ptr<IFutures> m_futures;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESVALUEMYSQLDAO_H_ */

/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESMYSQLDAO_H_
#define _DERIVATIVE_FUTURESMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IMake.hpp"
#include "IFutures.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for Futures entity.
	class FuturesMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum { TYPEID = CLASS_FUTURESMYSQLDAO_TYPE };

		/// constructors.

		/// Constructor with Exemplar for the Creator FuturesMySQLDAO object
		FuturesMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		FuturesMySQLDAO(const Name& daoName)
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
			LOG(WARNING) << " IDAO::insert(..) not applicable for Futures entity" << endl;
			throw DataSourceException("IDAO::Make (const Name &nm, const std::deque<boost::any>& agrs) not applicable for Futures entity");
		}

		/// IDAO method for inserting Futures object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for Futures entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for Futures entity");
		}

		/// /// IDAO method for removing Futures object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for Futures entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for Futures entity");
		}

		/// IDAO method for fetching futures from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with Futures values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for Futures entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Futures entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for Futures entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for Futures entity");
		}

	private:

		/// Populate the futures specific attributes
		void findFutures();

		/// Populate the primitive asset specific attributes
		/// shared by all primitive assets
		void findExchange();

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated futures entity
		std::shared_ptr<IFutures> m_futures;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESMYSQLDAO_H_ */

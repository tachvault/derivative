/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_STOCKYAHOODAO_H_
#define _DERIVATIVE_STOCKYAHOODAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "IStockValue.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for StockValue entity.
	class StockValueYahooDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_STOCKVALUEYAHOODAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator StockValueYahooDAO object
		StockValueYahooDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		StockValueYahooDAO(const Name& daoName)
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

		/// IDAO method for inserting StockValue object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for StockValue entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for StockValue entity");
		}

		/// /// IDAO method for removing StockValue object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for StockValue entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for StockValue entity");
		}

		/// IDAO method for fetching stockVal from data source
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

		/// IDAO method for updating data source with StockValue values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for StockValue entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for StockValue entity");
		}

	private:

		/// Populate the stockVal specific attributes
		void findStockValue();

		Name m_name;

		/// Associated stockVal entity
		std::shared_ptr<IStockValue> m_stockVal;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_STOCKYAHOODAO_H_ */

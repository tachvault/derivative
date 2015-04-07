/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICSTOCKINFOMYSQLDAO_H_
#define _DERIVATIVE_HISTORICSTOCKINFOMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "HistoricalStockInfo.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for HistoricalStockInfo entity.
	class HistoricalStockInfoMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_HISTORICSTOCKINFOMYSQLDAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator HistoricalStockInfoMySQLDAO object
		HistoricalStockInfoMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID), m_stockInfo(nullptr)
		{
		}
		
		/// Constructor for IMake::Make(..)
		HistoricalStockInfoMySQLDAO(const Name& daoName)
			:m_name(daoName), m_stockInfo(nullptr)
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
		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// IDAO method for inserting HistoricalStockInfo object into data source
		virtual void insert();

		/// /// IDAO method for removing HistoricalStockInfo object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for HistoricalStockInfo entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for HistoricalStockInfo entity");
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

		/// IDAO method for updating data source with HistoricalStockInfo values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for HistoricalStockInfo entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricalStockInfo entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for HistoricStockIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricStockIOfoDAP entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_stockInfo = nullptr;
		}
				
	private:

		/// Populate the stock specific attributes
		void findHistoricalStockInfo();

		/// construct DailyStockValue object
		std::shared_ptr<IDailyStockValue> ConstructDailyStockValue(const shared_ptr<IStock>& stock, const std::shared_ptr<IMake> exemplar, std::string& symbol, std::string& tradeDate, \
		                              double open, double high, double low, double close, double AdjClose);

		/// construct exemplar for DailyStockInfo
		std::shared_ptr<IMake> ConstructExemplar();

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated HistoricStockInfo entity
		std::shared_ptr<HistoricalStockInfo> m_stockInfo;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_HISTORICSTOCKINFOMYSQLDAO_H_ */

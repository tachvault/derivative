/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICALSTOCKINFOYAHOODAO_H_
#define _DERIVATIVE_HISTORICALSTOCKINFOYAHOODAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	class IStock;
	class IDailyStockValue;
	class HistoricalStockInfo;

	// Data Access Object for StockValue entity.
	class HistoricalStockInfoYahooDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_DAILYSTOCKVALUEYAHOODAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator HistoricalStockInfoYahooDAO object
		HistoricalStockInfoYahooDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		HistoricalStockInfoYahooDAO(const Name& daoName)
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

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs)
		{
			LOG(WARNING) << " IMake::Make(..) not applicable for HistoricStockIOfoDAP entity" << endl;
			throw DataSourceException("IMake::Make(..) not applicable for HistoricStockIOfoDAP entity");
		}

		/// IDAO method for inserting HistoricStockIOfoDAP object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for HistoricStockIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for HistoricStockIOfoDAP entity");
		}

		/// /// IDAO method for removing HistoricStockIOfoDAP object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for HistoricStockIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for HistoricStockIOfoDAP entity");
		}

		/// IDAO method for fetching stockVal from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with HistoricStockIOfoDAP values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for HistoricStockIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricStockIOfoDAP entity");
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

		/// Populate the stockVal specific attributes
		void findDailyStockValues();

		/// construct DailyStockValue object
		std::shared_ptr<IDailyStockValue> ConstructDailyStockValue(const shared_ptr<IStock>& stock, const std::shared_ptr<IMake> exemplar, const std::string& symbol, const std::string& tradeDate, \
		                              double open, double high, double low, double close, double AdjClose);

		/// construct exemplar for DailyStockInfo
		std::shared_ptr<IMake> ConstructExemplar();

		Name m_name;

		/// Associated stockInfo entity
		std::shared_ptr<HistoricalStockInfo> m_stockInfo;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_STOCKYAHOODAO_H_ */

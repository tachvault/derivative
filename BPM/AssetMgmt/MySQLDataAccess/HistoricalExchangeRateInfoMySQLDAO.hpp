/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICEXCHANGERATEINFOMYSQLDAO_H_
#define _DERIVATIVE_HISTORICEXCHANGERATEINFOMYSQLDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "HistoricalExchangeRateInfo.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	class IDailyExchangeRateValue;
	class HistoricalExchangeRateInfo;

	// Data Access Object for HistoricalExchangeRateInfo entity.
	class HistoricalExchangeRateInfoMySQLDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_HISTORICEXCHANGERATEINFOMYSQLDAO_TYPE};

		/// constructors.

		/// Constructor with Exemplar for the Creator HistoricalExchangeRateInfoMySQLDAO object
		HistoricalExchangeRateInfoMySQLDAO (const Exemplar &ex)
			:m_name(TYPEID), m_exchangeRateInfo(nullptr)
		{
		}
		
		/// Constructor for IMake::Make(..)
		HistoricalExchangeRateInfoMySQLDAO(const Name& daoName)
			:m_name(daoName), m_exchangeRateInfo(nullptr)
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

		/// IDAO method for inserting HistoricalExchangeRateInfo object into data source
		virtual void insert();

		/// /// IDAO method for removing HistoricalExchangeRateInfo object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for HistoricalExchangeRateInfo entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for HistoricalExchangeRateInfo entity");
		}

		/// IDAO method for fetching exchangeRate from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with HistoricalExchangeRateInfo values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for HistoricalExchangeRateInfo entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricalExchangeRateInfo entity");
		}

		/// refresh the given object from yahoo
		virtual bool refresh(shared_ptr<IObject>& obj)
		{
			LOG(WARNING) << " IDAO::refresh(..) not applicable for HistoricExchangeRateIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricExchangeRateIOfoDAP entity");
		}

		virtual int GetMaxDAOCount() const
		{
			return MaxCount;
		}

		virtual void Passivate()
		{
			m_exchangeRateInfo = nullptr;
		}
				
	private:

		/// Populate the exchangeRate specific attributes


		void findHistoricalExchangeRateInfo();

		/// construct DailyExchangeRateValue object
		std::shared_ptr<IDailyExchangeRateValue> ConstructDailyExchangeRateValue(const shared_ptr<IExchangeRate>& exchangeRate, \
		const std::shared_ptr<IMake> exemplar, size_t forex_id, std::string& tradeDate, \
		double open, double high, double low, double close);

		/// construct exemplar for DailyExchangeRateInfo
		std::shared_ptr<IMake> ConstructExemplar();

		Name m_name;

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		// Associated HistoricExchangeRateInfo entity
		std::shared_ptr<HistoricalExchangeRateInfo> m_exchangeRateInfo;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_HISTORICEXCHANGERATEINFOMYSQLDAO_H_ */

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICALEXCHANGERATEINFOXIGNITEDAO_H_
#define _DERIVATIVE_HISTORICALEXCHANGERATEINFOXIGNITEDAO_H_

#include <memory>
#include "IDAO.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	class IExchangeRate;
	class IDailyExchangeRateValue;
	class HistoricalExchangeRateInfo;

	// Data Access Object for ExchangeRateValue entity.
	class HistoricalExchangeRateInfoXigniteDAO : virtual public IDAO,
		             virtual public IObject,
					 virtual public IMake					 
	{
	public:

		enum {TYPEID = CLASS_DAILYEXCHANGERATEVALUEXIGNITEDAO_TYPE};

		/// Constructor with Exemplar for the Creator HistoricalExchangeRateInfoXigniteDAO object
		HistoricalExchangeRateInfoXigniteDAO (const Exemplar &ex)
			:m_name(TYPEID)
		{
		}
		
		/// Constructor for IMake::Make(..)
		HistoricalExchangeRateInfoXigniteDAO(const Name& daoName)
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
			LOG(WARNING) << " IMake::Make(..) not applicable for HistoricExchangeRateIOfoDAP entity" << endl;
			throw DataSourceException("IMake::Make(..) not applicable for HistoricExchangeRateIOfoDAP entity");
		}

		/// IDAO method for inserting HistoricExchangeRateIOfoDAP object into data source
		virtual void insert()
		{
			LOG(WARNING) << " IDAO::insert(..) not applicable for HistoricExchangeRateIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::insert(..) not applicable for HistoricExchangeRateIOfoDAP entity");
		}

		/// /// IDAO method for removing HistoricExchangeRateIOfoDAP object from data source
		virtual bool del()
		{
			LOG(WARNING) << " IDAO::del(..) not applicable for HistoricExchangeRateIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::del(..) not applicable for HistoricExchangeRateIOfoDAP entity");
		}

		/// IDAO method for fetching exchangeRateVal from data ource
		virtual const std::shared_ptr<IObject> find(const Name& nm);

		/// Given a name that imply multiple objects, find the entities
		/// from the data source.
		virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
		{
			LOG(WARNING) << " IDAO::find(nm, entities) not applicable for ExchangeRate entity" << endl;
			throw DataSourceException("IDAO::find(nm, entities) not applicable for ExchangeRate entity");
		}

		/// IDAO method for updating data source with HistoricExchangeRateIOfoDAP values
		virtual bool update()
		{
			LOG(WARNING) << " IDAO::update(..) not applicable for HistoricExchangeRateIOfoDAP entity" << endl;
			throw DataSourceException("IDAO::update(..) not applicable for HistoricExchangeRateIOfoDAP entity");
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
			m_rateInfo = nullptr;
		}

	private:

		/// Populate the exchangeRateVal specific attributes
		void findDailyExchangeRateValues();

		/// construct DailyExchangeRateValue object
		std::shared_ptr<IDailyExchangeRateValue> ConstructDailyExchangeRateValue(const shared_ptr<IExchangeRate>& rate, const std::shared_ptr<IMake> exemplar, \
			const std::string& domestic, const std::string& foreign, const std::string& tradeDate, \
			double open, double high, double low, double close, double adjClose);

		/// construct exemplar for DailyExchangeRateInfo
		std::shared_ptr<IMake> ConstructExemplar();

		Name m_name;

		/// Associated exchangeRateInfo entity
		std::shared_ptr<HistoricalExchangeRateInfo> m_rateInfo;

		static const int MaxCount;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEXIGNITEDAO_H_ */

/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/



#include <string>
#include <deque>

#include "HistoricalExchangeRateInfo.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "EntityManager.hpp"

#include "CurrencyHolder.hpp"
#include "Currency.hpp"

namespace derivative
{
	namespace
	{
		/// Due to Visual C++ compiler where mutex lock  hangs if called from .dll static variable inicialization, while the dll is loaded 
		/// during loading of dependency libraries on startup of an .exe file.
		/// GroupRegister ExchangeRateValGrp(HistoricalExchangeRateInfo::TYPEID,  std::make_shared<HistoricalExchangeRateInfo>(Exemplar())); 
	}

	HistoricalExchangeRateInfo::HistoricalExchangeRateInfo (const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	HistoricalExchangeRateInfo::HistoricalExchangeRateInfo (const Name& nm)
		:m_name(nm)
	{
	}	

	/// construct HistoricalExchangeRateInfo from 
	/// ExchangeRate symbol, Exchange, start date and end date
	HistoricalExchangeRateInfo::HistoricalExchangeRateInfo(const std::string& domesticCurrCode, const std::string& foreignCurrCode,	\
			const dd::date& startDate, const dd::date& endDate)
			:m_startDate(startDate), m_endDate(endDate), \
			m_name(ConstructName(domesticCurrCode, foreignCurrCode, startDate, endDate))
	{
		/// Initialize the domestic and foreign currencies from the codes
		CurrencyHolder& currHolder = CurrencyHolder::getInstance();
		m_domesticCurrency = currHolder.GetCurrency(domesticCurrCode);
		m_foreignCurrency = currHolder.GetCurrency(foreignCurrCode);
	}

	void HistoricalExchangeRateInfo::BuildExchangeRateInfo(unsigned int source)
	{
		/// Get the EntityMgrUtil fund the object from this object's
		/// name and the given data source
		try
		{
			auto thisObject = EntityMgrUtil::findObject(GetName(), source);
		}
		catch(RegistryException& e)
		{
			LOG(ERROR) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch(...)
		{
			LOG(ERROR) << " Unknown Exception thrown " << endl;
			throw;
		}
	}

	void BuildHistoricalExchangeRateInfo(ushort source, const const std::string& domCurrCode, std::string& foreignCurrCode, \
		                                const dd::date& start, const dd::date& end)
	{
		/// register the exemplar here until the bug in visual C++ is fixed.
		GroupRegister ExchangeRateValGrp(HistoricalExchangeRateInfo::TYPEID,  std::make_shared<HistoricalExchangeRateInfo>(Exemplar())); 				
		
		/// construct HistoricalExchangeRateInfo object
		std::shared_ptr<HistoricalExchangeRateInfo>  histExchangeRateInfo = std::make_shared<HistoricalExchangeRateInfo>\
			                                         (domCurrCode, foreignCurrCode, start, end);
		std::shared_ptr<IObject> obj;
		bool found = false;

		Name nm = histExchangeRateInfo->GetName();
		/// check if this object is already registered with EntityManager
		EntityManager& entMgr= EntityManager::getInstance();
		/// check if the object is already registered with EntityManager
		try
		{
			obj = entMgr.findObject(nm);
		}
		catch(RegistryException& e)
		{
			LOG(INFO) << e.what();
		}
		/// if the object is not found then register
		if (!obj)
		{
			try
			{
				entMgr.registerObject(nm, histExchangeRateInfo);
			}
			catch(RegistryException& e)
			{
				LOG(ERROR) << "Throw exception.. The object not in registry and unable to register or build" << endl;
				LOG(ERROR) << e.what();
			}
			
			/// now build the DailyExchangeRateInfo
			std::shared_ptr<IObject> obj = EntityMgrUtil::fetch(nm, source);
		}
	}

	void StoreHistoricalExchangeRateInfo(ushort source, const std::shared_ptr<HistoricalExchangeRateInfo> & histExchangeRate)
	{
		std::shared_ptr<IDataSource> src = EntityMgrUtil::getDataSourceHandler(histExchangeRate->GetName(), source);
		src->insert(histExchangeRate);
	}
}
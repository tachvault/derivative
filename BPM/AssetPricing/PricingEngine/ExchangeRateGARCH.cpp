/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "ExchangeRateGARCH.hpp"
#include "Global.hpp"
#include "IDataSource.hpp"
#include "PrimaryAssetUtil.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "HistoricalExchangeRateInfo.hpp"

#include "ConstVol.hpp"

namespace derivative
{
	ExchangeRateGARCH::ExchangeRateGARCH(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	ExchangeRateGARCH::ExchangeRateGARCH(const std::string& domestic, const std::string& foreign, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate)
		: GARCH(AssetClassTypeEnum::FOREX, asset, processDate),
		m_name(ConstructName(domestic, foreign, processDate)),
		m_domestic(domestic),
		m_foreign(foreign)
	{			
	}

	void ExchangeRateGARCH::LoadData()
	{		
		/// start date
		dd::date start = m_processedDate - dd::date_duration(365);
		std::shared_ptr<HistoricalExchangeRateInfo> histInfo;
		try
		{
			histInfo = BuildHistoricalExchangeRateInfo(XIGNITE, m_domestic, m_foreign, start, m_processedDate);
			histInfo->BuildExchangeRateInfo(XIGNITE);
			m_exchangeRateValues = histInfo->GetDailyExchangeRateValues();

			/// throw if exchange values are empty
			if (m_exchangeRateValues.empty()) throw std::domain_error("No exchange data found for the given data change");

			// sort the data by trade date
			sort(m_exchangeRateValues.begin(), m_exchangeRateValues.end(),
				[](const std::shared_ptr<IDailyExchangeRateValue> & a, const std::shared_ptr<IDailyExchangeRateValue> & b)
			{
				return a->GetTradeDate() < b->GetTradeDate();
			});

			int i = 0;
			m_values.resize(m_exchangeRateValues.size());
			for (auto& exchangeRateVal :m_exchangeRateValues)
			{
				m_values(i) = exchangeRateVal->GetTradePrice();
				++i;
			}
		}
		catch (std::exception &e)
		{
			LOG(WARNING) << " Unable to retrieve daily equity values " << e.what() << endl;
			throw e;
		}
	}

	std::shared_ptr<ExchangeRateGARCH> BuildExchangeRateGARCH(const std::string& domestic, const std::string& foreign, const dd::date& edate)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister ExchangeRateGARCHGrp(ExchangeRateGARCH::TYPEID, std::make_shared<ExchangeRateGARCH>(Exemplar()));

		Name curName = ExchangeRateGARCH::ConstructName(domestic, foreign, edate);
		/// check if this object is already registered with EntityManager

		std::shared_ptr<IObject> obj;
		EntityManager& entMgr = EntityManager::getInstance();
		/// check if the object is already registered with EntityManager
		try
		{
			obj = entMgr.findObject(curName);
		}
		catch (RegistryException& e)
		{
			LOG(INFO) << e.what();
		}
		/// if the object is found then register return
		if (obj)
		{
			std::shared_ptr<ExchangeRateGARCH>  garch = dynamic_pointer_cast<ExchangeRateGARCH>(obj);
			return garch;
		}

		/// if not in registry, then construct the volatility surface
		std::shared_ptr<IExchangeRateValue> assetVal = PrimaryUtil::getExchangeRateValue(domestic, foreign);
		std::shared_ptr<ExchangeRateGARCH>  garch = std::make_shared<ExchangeRateGARCH>(domestic, foreign, assetVal, edate);

		try
		{
			/// load data and find the garch parameters
			garch->LoadData();
			garch->Build();

			/// if sucessfully loaded all options (without any exception)
			/// register with EntityManager
			entMgr.registerObject(garch->GetName(), garch);
			return garch;
		}
		catch (RegistryException& e)
		{
			LOG(ERROR) << "Throw exception.. The object not in registry and unable to register or build" << endl;
			LOG(ERROR) << e.what();
			throw e;
		}
	}
}
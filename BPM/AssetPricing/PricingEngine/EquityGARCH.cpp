/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "EquityGARCH.hpp"
#include "Global.hpp"
#include "IDataSource.hpp"
#include "PrimaryAssetUtil.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "HistoricalStockInfo.hpp"

#include "ConstVol.hpp"

namespace derivative
{
	EquityGARCH::EquityGARCH(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	EquityGARCH::EquityGARCH(const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate)
		: GARCH(AssetClassTypeEnum::EQUITY, asset, processDate),
		m_name(ConstructName(symbol, processDate)),
		m_symbol(symbol)
	{			
	}

	void EquityGARCH::LoadData()
	{		
		/// start date
		dd::date start = m_processedDate - dd::date_duration(365);
		std::shared_ptr<HistoricalStockInfo> histInfo;
		try
		{
			histInfo = BuildHistoricalStockInfo(YAHOO, m_symbol, start, m_processedDate);
			histInfo->BuildStockInfo(YAHOO);
			m_stockValues = histInfo->GetDailyStockValues();

			/// throw if stock values are empty
			if (m_stockValues.empty()) throw std::domain_error("No stock data found for the given data change");

			// sort the data by trade date
			sort(m_stockValues.begin(), m_stockValues.end(),
				[](const std::shared_ptr<IDailyStockValue> & a, const std::shared_ptr<IDailyStockValue> & b)
			{
				return a->GetTradeDate() < b->GetTradeDate();
			});

			int i = 0;
			m_values.resize(m_stockValues.size());
			for (auto& stockVal :m_stockValues)
			{
				m_values(i) = stockVal->GetTradePrice();
				++i;
			}
		}
		catch (std::exception &e)
		{
			LOG(WARNING) << " Unable to retrieve daily equity values " << e.what() << endl;
			throw e;
		}
	}

	std::shared_ptr<EquityGARCH> BuildEquityGARCH(const std::string& symbol, const dd::date& edate)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister EquityGARCHGrp(EquityGARCH::TYPEID, std::make_shared<EquityGARCH>(Exemplar()));

		Name curName = EquityGARCH::ConstructName(symbol, edate);
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
			std::shared_ptr<EquityGARCH>  garch = dynamic_pointer_cast<EquityGARCH>(obj);
			return garch;
		}

		/// if not in registry, then construct the volatility surface
		std::shared_ptr<IStockValue> assetVal = PrimaryUtil::getStockValue(symbol);
		std::shared_ptr<EquityGARCH>  garch = std::make_shared<EquityGARCH>(symbol, assetVal,edate);

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
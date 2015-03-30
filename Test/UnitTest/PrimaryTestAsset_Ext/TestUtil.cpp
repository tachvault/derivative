/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include "TestUtil.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "IStockValue.hpp"
#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "IIRValue.hpp"
#include "IIR.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "EntityManager.hpp"

namespace derivative
{
	namespace TestUtil
	{
		std::shared_ptr<IObject> GetNamedObject(const Name& nm)
		{
			std::shared_ptr<IObject> obj;
			EntityManager& entMgr= EntityManager::getInstance();

			/// get the concsrete Name for the given alias Name
			grpType concreteType = entMgr.findAlias(nm);
			Name concreteName = nm;
			if (concreteType > 0)
			{
				concreteName = Name(concreteType, nm.GetObjId());
			}

			/// check if the object is already registered with EntityManager
			obj = entMgr.findObject(concreteName);	
			if (obj)
			{
				return obj;
			}

			/// get the concrete types for the given alias name
			std::vector<Name> names;
			entMgr.findAlias(nm, names);
			if (names.empty())
			{
				LOG(ERROR) << " No concrete name found for " << nm << endl;
				throw DataSourceException("Stock not bound with correct alias");
			}

			Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

			/// copy the keys from Alias Name to concrete name
			entityName.SetKeys(nm.GetKeyMap());

			/// get the examplar object for the Stock
			/// Exemplar objects should be initialized
			/// during global initialization time.
			std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

			/// Now we have the exampler object. 
			/// Make the exemplar object to construct given named object		
			obj = dynamic_pointer_cast<IObject>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));

			return obj;
		}

		std::shared_ptr<IStockValue> getStockValue(const std::string& symbol)
		{
			std::shared_ptr<IStock> stock;
			std::shared_ptr<IStockValue> stockVal;		

			/// find the stock from MySQL database
			Name stockName = IStock::ConstructName(symbol);
			stock = dynamic_pointer_cast<IStock>(GetNamedObject(stockName));

			/// find current stock value
			Name stockValName = IStockValue::ConstructName(symbol);
			stockVal = dynamic_pointer_cast<IStockValue>(GetNamedObject(stockValName));
			stockVal->SetStock(stock);
			return stockVal;
		}

		std::shared_ptr<IExchangeRateValue> getExchangeRateValue(const std::string& domestic, const std::string& foreign)
		{
			std::shared_ptr<IExchangeRate> exchangeRate;
			std::shared_ptr<IExchangeRateValue> exchangeRateVal;		

			/// find the exchange rate from MySQL
			Name exchangeRateName = IExchangeRate::ConstructName(domestic, foreign);
			exchangeRate = dynamic_pointer_cast<IExchangeRate>(GetNamedObject(exchangeRateName));

			/// find current exchange rate value
			Name exchangeRateValName = IExchangeRateValue::ConstructName(domestic, foreign);
			exchangeRateVal = dynamic_pointer_cast<IExchangeRateValue>(GetNamedObject(exchangeRateValName));
			exchangeRateVal->SetExchangeRate(exchangeRate);
			return exchangeRateVal;
		}
	}
}
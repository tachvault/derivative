/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "RESTConnectionUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ExchangeRateValue);
	ALIAS_REGISTER(ExchangeRateValue,IAssetValue);
	ALIAS_REGISTER(ExchangeRateValue,IExchangeRateValue);

	ExchangeRateValue::ExchangeRateValue (const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	ExchangeRateValue::ExchangeRateValue (const Name& nm)
		:m_name(nm), m_priceAsk(0.0), m_priceBid(0.0), m_priceOpen(0.0), \
		m_priceClose(0.0), m_dayLow(0.0), m_dayHigh(0.0), m_52WkLow(0.0), m_52WkHigh(0.0)
	{
		// m_vol = std::make_shared<VolatilitySurface>();
	}

	std::shared_ptr<IMake> ExchangeRateValue::Make(const Name &nm)
	{
		/// Construct Stock from given name and register with EntityManager
		std::shared_ptr<ExchangeRateValue> exchangeRateVal = make_shared<ExchangeRateValue>(nm);
		EntityMgrUtil::registerObject(nm, exchangeRateVal);		
		LOG(INFO) << " Stock  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return exchangeRateVal;
	}

	std::shared_ptr<IMake> ExchangeRateValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void ExchangeRateValue::convert( istringstream  &input)
	{ 
		std::string elem;
		if (std::getline(input, elem,',')) 
		{
			/// strip away '"\'
			elem.erase (std::remove(elem.begin(), elem.end(), '\"'), elem.end());
			m_tradeDate = boost::gregorian::from_us_string(elem); 
		}
		else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))
		{
			m_tradeTime = pt::ptime(m_tradeDate) + RESTConnectionUtil::get_duration_from_string(elem);
			LOG(INFO) << "Last trade reported time " << pt::to_simple_string(m_tradeTime) <<endl;
		}
		else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceTrade = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceAsk = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_priceBid = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_priceOpen = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_priceClose = atof(elem.c_str()); else return;	  
		if (std::getline(input, elem,','))  m_dayLow = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_dayHigh = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_52WkLow = atof(elem.c_str()); else return;
		if (std::getline(input, elem,','))  m_52WkHigh = atof(elem.c_str()); else return;
		return;            
	}
}
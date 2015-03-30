/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "StockValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "RESTConnectionUtil.hpp"
#include "DException.hpp"

namespace derivative
{
	GROUP_REGISTER(StockValue);
	ALIAS_REGISTER(StockValue,IAssetValue);
	ALIAS_REGISTER(StockValue,IStockValue);
	
	StockValue::StockValue (const Exemplar &ex)
		:m_name(TYPEID), m_priceAsk(0), m_priceBid(0), m_priceOpen(0), m_priceClose(0),	m_priceTrade(0),
		m_change(0), m_changePct(0),m_divYield(0),m_divShare(0),m_dayLow(0),m_dayHigh(0),m_52WkLow(0),
		m_52WkHigh(0),m_eps(0),	m_pe(0),m_shares(0),m_volume(0),m_beta(0)
	{
	}

	StockValue::StockValue (const Name& nm)
		:m_name(nm),m_priceAsk(0), m_priceBid(0), m_priceOpen(0), m_priceClose(0),	m_priceTrade(0),
		m_change(0), m_changePct(0),m_divYield(0),m_divShare(0),m_dayLow(0),m_dayHigh(0),m_52WkLow(0),
		m_52WkHigh(0),m_eps(0),	m_pe(0),m_shares(0),m_volume(0),m_beta(0)		
	{		
	}

	std::shared_ptr<IMake> StockValue::Make(const Name &nm)
	{
		/// Construct Stock from given name and register with EntityManager
		std::shared_ptr<StockValue> stockVal = make_shared<StockValue>(nm);
		EntityMgrUtil::registerObject(nm, stockVal);		
		LOG(INFO) << " Stock  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return stockVal;
	}

	std::shared_ptr<IMake> StockValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void StockValue::convert( istringstream  &input)
	{ 
		std::string elem;
		if (std::getline(input, elem,','))  m_priceAsk = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceBid = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceOpen = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceClose = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_priceTrade = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_change = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_changePct = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_divYield = (atof(elem.c_str())/100); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_divShare = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_dayLow = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_dayHigh = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_52WkLow = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_52WkHigh = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
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
		if (std::getline(input, elem,','))  m_volume = atoi(elem.c_str()); else throw YahooSrcException("Invalid data");	 
		return;            
	}
}
/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "FuturesValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "RESTConnectionUtil.hpp"
#include "DException.hpp"

namespace derivative
{
	GROUP_REGISTER(FuturesValue);
	ALIAS_REGISTER(FuturesValue, IAssetValue);
	ALIAS_REGISTER(FuturesValue, IFuturesValue);

	FuturesValue::FuturesValue(const Exemplar &ex)
		:m_name(TYPEID), m_priceHigh(0), m_priceLow(0), m_priceOpen(0), m_priceClose(0), m_priceTrade(0),
		m_settledPrice(0), m_volume(0), m_openInterest(0)
	{
	}

	FuturesValue::FuturesValue(const Name& nm)
		: m_name(nm), m_priceHigh(0), m_priceLow(0), m_priceOpen(0), m_priceClose(0), m_priceTrade(0),
		m_settledPrice(0), m_volume(0), m_openInterest(0)
	{
	}

	std::shared_ptr<IMake> FuturesValue::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		
		/// Construct Futures from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<FuturesValue> stockVal = make_shared<FuturesValue>(nm);
		
		/// return constructed object if no exception is thrown
		return stockVal;
	}

	std::shared_ptr<IMake> FuturesValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void FuturesValue::convert(istringstream  &input)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		std::string elem;
		if (std::getline(input, elem, ','))  m_priceHigh = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceLow = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceOpen = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceClose = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceTrade = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_settledPrice = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))
		{
			/// strip away '"\'
			elem.erase(std::remove(elem.begin(), elem.end(), '\"'), elem.end());
			m_tradeDate = boost::gregorian::from_us_string(elem);
		}
		else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_volume = atoi(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_openInterest = atoi(elem.c_str()); else throw YahooSrcException("Invalid data");
		return;
	}
}
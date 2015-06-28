/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "DailyExchangeRateValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyExchangeRateValue);
	ALIAS_REGISTER(DailyExchangeRateValue, IAssetValue);
	ALIAS_REGISTER(DailyExchangeRateValue, IDailyExchangeRateValue);

	DailyExchangeRateValue::DailyExchangeRateValue(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	DailyExchangeRateValue::DailyExchangeRateValue(const Name& nm, double priceOpen, double priceClose, \
		double priceHigh, double priceLow, double adjClose, const dd::date& tradeDate)
		: m_name(nm), m_priceOpen(priceOpen), m_priceClose(priceClose), m_priceHigh(priceHigh), \
		m_priceLow(priceLow), m_adjClose(adjClose), m_tradeDate(tradeDate)

	{
	}

	std::shared_ptr<IMake> DailyExchangeRateValue::Make(const Name &nm)
	{
		throw std::logic_error("Invalid factory method call");
	}

	std::shared_ptr<IMake> DailyExchangeRateValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// first validate the input parameters
		if (!agrs.empty() && agrs.size() != 6)
		{
			throw std::invalid_argument("Invalid number of arguments");
		}

		/// Construct ExchangeRate from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<DailyExchangeRateValue> exchangeRateVal = std::make_shared<DailyExchangeRateValue>(nm, boost::any_cast<double>(agrs[0]), \
			boost::any_cast<double>(agrs[1]), boost::any_cast<double>(agrs[2]), \
			boost::any_cast<double>(agrs[3]), boost::any_cast<double>(agrs[4]), \
			boost::any_cast<dd::date>(agrs[5]));

		/// return constructed object if no exception is thrown
		return exchangeRateVal;
	}

	void DailyExchangeRateValue::convert(istringstream  &input)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		std::string elem;
		if (std::getline(input, elem, ','))  m_tradeDate = boost::gregorian::from_simple_string(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceOpen = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceHigh = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceLow = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceClose = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		return;
	}
}
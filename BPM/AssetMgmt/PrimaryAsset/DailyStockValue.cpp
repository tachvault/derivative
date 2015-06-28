/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "DailyStockValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "IStock.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyStockValue);
	ALIAS_REGISTER(DailyStockValue, IAssetValue);
	ALIAS_REGISTER(DailyStockValue, IDailyStockValue);

	DailyStockValue::DailyStockValue(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	DailyStockValue::DailyStockValue(const Name& nm, double priceOpen, double priceClose, \
		double priceHigh, double priceLow, double priceAdjClose, const dd::date& tradeDate)
		: m_name(nm), m_priceOpen(priceOpen), m_priceClose(priceClose), m_priceHigh(priceHigh), \
		m_priceLow(priceLow), m_priceAdjustedClose(priceAdjClose), m_tradeDate(tradeDate)
	{
	}

	std::shared_ptr<IMake> DailyStockValue::Make(const Name &nm)
	{
		throw std::logic_error("Invalid factory method call");
	}

	std::shared_ptr<IMake> DailyStockValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// first validate the input parameters
		if (!agrs.empty() && agrs.size() != 6)
		{
			throw std::invalid_argument("Invalid number of arguments");
		}

		/// Construct Stock from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<DailyStockValue> stockVal = std::make_shared<DailyStockValue>(nm, boost::any_cast<double>(agrs[0]), \
			boost::any_cast<double>(agrs[1]), boost::any_cast<double>(agrs[2]), boost::any_cast<double>(agrs[3]), \
			boost::any_cast<double>(agrs[4]), boost::any_cast<dd::date>(agrs[5]));

		/// return constructed object if no exception is thrown
		return stockVal;
	}

	void DailyStockValue::convert(istringstream  &input)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		std::string elem;
		if (std::getline(input, elem, ','))  m_tradeDate = boost::gregorian::from_simple_string(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceOpen = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceHigh = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceLow = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceClose = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem, ','))  m_priceAdjustedClose = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		return;
	}
}
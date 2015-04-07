/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "DailyEquityOptionValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"

#include "IStock.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyEquityOptionValue);
	ALIAS_REGISTER(DailyEquityOptionValue,IAssetValue);
	ALIAS_REGISTER(DailyEquityOptionValue, IDailyOptionValue);
	ALIAS_REGISTER(DailyEquityOptionValue,IDailyEquityOptionValue);

	DailyEquityOptionValue::DailyEquityOptionValue (const Exemplar &ex)
		:m_name(TYPEID),m_optType(OPTION_TYPE_UNKNOWN),m_tradePrice(0),m_strikePrice(0),m_askingPrice(0),
		m_bidPrice(0),m_volume(0),m_openInt(0)
	{
	}

	DailyEquityOptionValue::DailyEquityOptionValue (const Name &nm)
		:m_name(nm),m_optType(OPTION_TYPE_UNKNOWN),m_tradePrice(0),m_strikePrice(0),m_askingPrice(0),
		m_bidPrice(0),m_volume(0),m_openInt(0)
	{
	}

	DailyEquityOptionValue::DailyEquityOptionValue (const Name& nm, double price, double strike, const OptionType& optType, \
		const dd::date& maturityDate, double askingPrice, double bidPrice, int volume, \
		int openInt, const dd::date& tradeDate)
		:m_name(nm), m_tradePrice(price), m_optType(optType), m_maturityDate(maturityDate), m_askingPrice(askingPrice), \
		m_bidPrice(bidPrice), m_volume(volume), m_openInt(openInt), m_tradeDate(tradeDate)
	{
	}

	std::shared_ptr<IMake> DailyEquityOptionValue::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		/// Construct DailyEquityOptionValue from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<DailyEquityOptionValue> value = make_shared<DailyEquityOptionValue>(nm);
		/// return constructed object if no exception is thrown
		return value;
	}

	std::shared_ptr<IMake> DailyEquityOptionValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		/// first validate the input parameters
		if (!agrs.empty() &&  agrs.size() != 6)
		{
			throw std::invalid_argument("Invalid number of arguments");
		}

		/// Construct Option from given name and register with EntityManager
		std::shared_ptr<DailyEquityOptionValue> optionVal = std::make_shared<DailyEquityOptionValue>(nm, 
			boost::any_cast<double>(agrs[0]),  /* trade price */ \
			boost::any_cast<double>(agrs[1]), /* strike price */ \
			static_cast<OptionType>(boost::any_cast<int>(agrs[2])), /* option type */ \
			boost::any_cast<dd::date>(agrs[5]), /* maturity date */ \
			boost::any_cast<double>(agrs[4]), /* asking price */
			boost::any_cast<double>(agrs[4]), /* bid price */
			boost::any_cast<int>(agrs[4]), /* volume */
			boost::any_cast<int>(agrs[4]), /* open interest */
			boost::any_cast<dd::date>(agrs[5]) /* trace date */);

		EntityMgrUtil::registerObject(nm,optionVal);		
		LOG(INFO) << " DailyEquityOptionValue  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return optionVal;
	}

	void DailyEquityOptionValue::convert( istringstream  &input)
	{ 
		std::lock_guard<SpinLock> lock(m_lock);
		std::string elem;
		if (std::getline(input, elem,','))  m_tradeDate = boost::gregorian::from_simple_string(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_maturityDate = boost::gregorian::from_simple_string(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_optType = static_cast<OptionType>(atoi(elem.c_str())); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_tradePrice = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_askingPrice = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_bidPrice = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_volume = atoi(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_openInt = atoi(elem.c_str()); else throw YahooSrcException("Invalid data");
		return;            
	}
}
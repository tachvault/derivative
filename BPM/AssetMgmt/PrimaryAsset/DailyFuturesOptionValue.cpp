/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "DailyFuturesOptionValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"

#include "IStock.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyFuturesOptionValue);
	ALIAS_REGISTER(DailyFuturesOptionValue,IAssetValue);
	ALIAS_REGISTER(DailyFuturesOptionValue,IDailyOptionValue);
	ALIAS_REGISTER(DailyFuturesOptionValue, IDailyFuturesOptionValue);

	DailyFuturesOptionValue::DailyFuturesOptionValue (const Exemplar &ex)
		:m_name(TYPEID),m_optType(OPTION_TYPE_UNKNOWN),m_tradePrice(0),m_strikePrice(0),m_highPrice(0),
		m_lowPrice(0),m_settledPrice(0), m_volume(0),m_openInt(0)
	{
	}

	DailyFuturesOptionValue::DailyFuturesOptionValue (const Name &nm)
		:m_name(nm),m_optType(OPTION_TYPE_UNKNOWN),m_tradePrice(0),m_strikePrice(0),m_highPrice(0),
		m_lowPrice(0),m_settledPrice(0), m_volume(0),m_openInt(0)
	{
	}

	DailyFuturesOptionValue::DailyFuturesOptionValue (const Name& nm, double price, double strike, const OptionType& optType, \
		const dd::date& maturityDate, double highPrice, double lowPrice, double settle, int volume, \
		int openInt, const dd::date& tradeDate)
		:m_name(nm), m_tradePrice(price), m_optType(optType), m_maturityDate(maturityDate), m_highPrice(highPrice), \
		m_lowPrice(lowPrice), m_settledPrice(settle), m_volume(volume), m_openInt(openInt), m_tradeDate(tradeDate)
	{
	}

	std::shared_ptr<IMake> DailyFuturesOptionValue::Make(const Name &nm)
	{
		/// Construct DailyFuturesOptionValue from given name and register with EntityManager
		std::shared_ptr<DailyFuturesOptionValue> value = make_shared<DailyFuturesOptionValue>(nm);
		EntityMgrUtil::registerObject(nm, value);		
		LOG(INFO) << " DailyFuturesOptionValue  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return value;
	}

	std::shared_ptr<IMake> DailyFuturesOptionValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Not supported");		
	}

	void DailyFuturesOptionValue::convert( istringstream  &input)
	{
		throw std::logic_error("Not supported");
	}
}
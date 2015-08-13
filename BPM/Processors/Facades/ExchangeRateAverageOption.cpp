/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "ExchangeRateAverageOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "ExchangeRateAverageOptMessage.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "ExchangeRateGARCH.hpp"

#include "ExchangeRateAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(ExchangeRateAverageOption);
	ALIAS_REGISTER(ExchangeRateAverageOption, IMessageSink);
	MSG_REGISTER(ExchangeRateAverageOptMessage, ExchangeRateAverageOption);

	ExchangeRateAverageOption::ExchangeRateAverageOption(const Exemplar &ex)
		:ExchangeRateOption(),
		m_name(TYPEID)
	{}

	ExchangeRateAverageOption::ExchangeRateAverageOption(const Name& nm)
		: ExchangeRateOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> ExchangeRateAverageOption::Make(const Name &nm)
	{
		/// Construct ExchangeRateAverageOption from given name and register with EntityManager
		std::shared_ptr<ExchangeRateAverageOption> optionProc = std::make_shared<ExchangeRateAverageOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> ExchangeRateAverageOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void ExchangeRateAverageOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<ExchangeRateAverageOptMessage> optMsg = std::dynamic_pointer_cast<ExchangeRateAverageOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == ExchangeRateAverageOptMessage::CALL) ? 1 : -1;
		m_averageType = optMsg->GetRequest().averageType;
		m_maturity = optMsg->GetRequest().maturity;
		m_domestic = optMsg->GetRequest().domestic;
		m_foreign = optMsg->GetRequest().foreign;
		
		/// get exchangeRate value.
		m_exchangeRateVal = PrimaryUtil::getExchangeRateValue(m_domestic, m_foreign);
		/// get strike price
		if (optMsg->GetRequest().strike == std::numeric_limits<double>::max())
		{
			optMsg->GetRequest().strike = m_exchangeRateVal->GetTradePrice();
		}
		m_strike = optMsg->GetRequest().strike;

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// get the interest rate
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;

		/// now construct the BlackScholesAdapter from the exchangeRate value.
		m_exchangeRate = std::make_shared<BlackScholesAssetAdapter>(m_exchangeRateVal, m_vol);

		/// for ExchangeRate, the yield is the same as interest rate in black scholes world.
		m_exchangeRate->SetDivYield(m_foreignRate);

		/// get the pricing method and run 
		ExchangeRateAverageOptMessage::AverageOptResponse res;
		res.optPrice = m_exchangeRate->option(t, m_strike, m_localRate, m_optType);
		if (optMsg->GetRequest().method == ExchangeRateAverageOptMessage::MONTE_CARLO)
		{
			res.averageOptPrice = ExchangeRateAverageOptionPricer::ValueWithMC(m_exchangeRate, \
				m_termLocal, m_averageType, m_maturity, m_strike, static_cast<ExchangeRateAverageOptionPricer::VanillaOptionType>(m_optType));
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		res.underlyingTradeDate = m_exchangeRateVal->GetTradeDate();
		res.underlyingTradePrice = m_exchangeRateVal->GetTradePrice();

		/// set the message;
		optMsg->SetResponse(res);
	}
}

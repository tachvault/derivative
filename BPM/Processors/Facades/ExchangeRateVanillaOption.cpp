/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "ExchangeRateVanillaOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "ExchangeRateVanillaOptMessage.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "GeometricBrownianMotion.hpp"
#include "Payoff.hpp"
#include "MCMapping.hpp"
#include "ConstVol.hpp"
#include "FiniteDifference.hpp"
#include "Binomial.hpp"
#include "LongstaffSchwartz.hpp"
#include "MCAmerican.hpp"
#include "MCGeneric.hpp"
#include "MExotics.hpp"
#include "QFRandom.hpp"
#include "PiecewiseVol.hpp"
#include "Exchange.hpp"
#include "ExchangeRateGARCH.hpp"
#include "CountryHolder.hpp"
#include "ExchangeRateAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(ExchangeRateVanillaOption);
	ALIAS_REGISTER(ExchangeRateVanillaOption, IMessageSink);
	MSG_REGISTER(ExchangeRateVanillaOptMessage, ExchangeRateVanillaOption);

	ExchangeRateVanillaOption::ExchangeRateVanillaOption(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	ExchangeRateVanillaOption::ExchangeRateVanillaOption(const Name& nm)
		: m_name(nm)
	{}

	std::shared_ptr<IMake> ExchangeRateVanillaOption::Make(const Name &nm)
	{
		/// Construct ExchangeRateVanillaOption from given name and register with EntityManager
		std::shared_ptr<ExchangeRateVanillaOption> optionProc = std::make_shared<ExchangeRateVanillaOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> ExchangeRateVanillaOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void ExchangeRateVanillaOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		dd::date today = dd::day_clock::local_day();
		std::shared_ptr<ExchangeRateVanillaOptMessage> optMsg = std::dynamic_pointer_cast<ExchangeRateVanillaOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == ExchangeRateVanillaOptMessage::CALL) ? 1 : -1;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		
		/// get ExchangeRate value.
		m_exchangeRateVal = PrimaryUtil::getExchangeRateValue(optMsg->GetRequest().domestic, optMsg->GetRequest().foreign);

		
		/// now construct the BlackScholesAdapter from the ExchangeRate value.
		m_exchangeRate = std::make_shared<BlackScholesAssetAdapter>(m_exchangeRateVal, m_vol);

		/// for ExchangeRate, the yield is the same as interest rate in black scholes world.
		m_exchangeRate->SetDivYield(m_foreignRate);

		/// get the pricing method and run 
		ExchangeRateVanillaOptMessage::Response res;
		if (optMsg->GetRequest().method == ExchangeRateVanillaOptMessage::CLOSED)
		{
			auto mat = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
			res.optPrice = m_exchangeRate->option(mat, m_strike, m_localRate, m_optType);
		}
		else if (optMsg->GetRequest().method == ExchangeRateVanillaOptMessage::LATTICE)
		{
			if (optMsg->GetRequest().style == VanillaOptMessage::EUROPEAN)
			{
				res.optPrice = ExchangeRateVanillaOptionPricer::ValueEuropeanWithBinomial(m_exchangeRate, m_localRate, m_maturity, \
					m_strike, static_cast<ExchangeRateVanillaOptionPricer::VanillaOptionType>(m_optType));
			}
			else
			{
				res.optPrice = ExchangeRateVanillaOptionPricer::ValueAmericanWithBinomial(m_exchangeRate, m_localRate, m_maturity, \
					m_strike, static_cast<ExchangeRateVanillaOptionPricer::VanillaOptionType>(m_optType));
			}
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set underlying info
		res.underlyingTradeDate = m_exchangeRateVal->GetTradeDate();
		res.underlyingTradePrice = m_exchangeRateVal->GetTradePrice();

		/// now get the greeks
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_exchangeRate->delta(mat, m_strike, m_localRate, m_optType);
		res.greeks.gamma = m_exchangeRate->gamma(mat, m_strike, m_localRate, m_optType);
		res.greeks.theta = m_exchangeRate->theta(mat, m_strike, m_localRate, m_optType);
		res.greeks.vega = m_exchangeRate->vega(mat, m_strike, m_localRate, m_optType);

		/// set the message;
		optMsg->SetResponse(res);
	}
}

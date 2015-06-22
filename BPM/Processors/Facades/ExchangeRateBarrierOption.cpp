/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "ExchangeRateBarrierOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "ExchangeRateBarrierOptMessage.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "ExchangeRateGARCH.hpp"

#include "ExchangeRateAssetPricer.hpp"
#include "ExchangeRateOption.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(ExchangeRateBarrierOption);
	ALIAS_REGISTER(ExchangeRateBarrierOption, IMessageSink);
	MSG_REGISTER(ExchangeRateBarrierOptMessage, ExchangeRateBarrierOption);

	ExchangeRateBarrierOption::ExchangeRateBarrierOption(const Exemplar &ex)
		:ExchangeRateOption(),
		m_name(TYPEID)
	{}

	ExchangeRateBarrierOption::ExchangeRateBarrierOption(const Name& nm)
		: ExchangeRateOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> ExchangeRateBarrierOption::Make(const Name &nm)
	{
		/// Construct ExchangeRateBarrierOption from given name and register with EntityManager
		std::shared_ptr<ExchangeRateBarrierOption> optionProc = std::make_shared<ExchangeRateBarrierOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> ExchangeRateBarrierOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void ExchangeRateBarrierOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<ExchangeRateBarrierOptMessage> optMsg = std::dynamic_pointer_cast<ExchangeRateBarrierOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == ExchangeRateBarrierOptMessage::CALL) ? 1 : -1;
		m_barrierType = optMsg->GetRequest().barrierType;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		m_barrier = optMsg->GetRequest().barrier;
		m_domestic = optMsg->GetRequest().domestic;
		m_foreign = optMsg->GetRequest().foreign;

		/// get exchangeRate value.
		m_exchangeRateVal = PrimaryUtil::getExchangeRateValue(m_domestic, m_foreign);
		
		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// now construct the BlackScholesAdapter from the exchangeRate value.
		m_exchangeRate = std::make_shared<BlackScholesAssetAdapter>(m_exchangeRateVal, m_vol);

		/// for ExchangeRate, the yield is the same as interest rate in black scholes world.
		m_exchangeRate->SetDivYield(m_foreignRate);

		/// get the pricing method and run 
		ExchangeRateBarrierOptMessage::BarrierResponse res;
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		res.optPrice = m_exchangeRate->option(t, m_strike, m_localRate, m_optType);
		if (optMsg->GetRequest().method == ExchangeRateBarrierOptMessage::MONTE_CARLO)
		{
			res.barierOptPrice = ExchangeRateBarrierOptionPricer::ValueWithMC(m_exchangeRate, m_termLocal, m_barrierType, m_maturity, \
				m_strike, m_barrier, static_cast<ExchangeRateBarrierOptionPricer::VanillaOptionType>(m_optType));
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

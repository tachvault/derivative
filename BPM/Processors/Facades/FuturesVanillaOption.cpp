/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "FuturesVanillaOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "FuturesVanillaOptMessage.hpp"

#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "FuturesVolatilitySurface.hpp"
#include "Exchange.hpp"
#include "FuturesAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(FuturesVanillaOption);
	ALIAS_REGISTER(FuturesVanillaOption, IMessageSink);
	MSG_REGISTER(FuturesVanillaOptMessage, FuturesVanillaOption);

	FuturesVanillaOption::FuturesVanillaOption(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	FuturesVanillaOption::FuturesVanillaOption(const Name& nm)
		: FuturesOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> FuturesVanillaOption::Make(const Name &nm)
	{
		/// Construct FuturesVanillaOption from given name and register with EntityManager
		std::shared_ptr<FuturesVanillaOption> optionProc = std::make_shared<FuturesVanillaOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> FuturesVanillaOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void FuturesVanillaOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		dd::date today = dd::day_clock::local_day();
		std::shared_ptr<FuturesVanillaOptMessage> optMsg = std::dynamic_pointer_cast<FuturesVanillaOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == FuturesVanillaOptMessage::CALL) ? 1 : -1;
		m_maturity = optMsg->GetRequest().maturity;
		m_delivery = optMsg->GetRequest().deliveryDate;
		m_strike = optMsg->GetRequest().strike;

		/// get futures value.
		m_futuresVal = PrimaryUtil::getFuturesValue(optMsg->GetRequest().underlying, today, m_delivery);
		/// get strike price
		if (optMsg->GetRequest().strike == std::numeric_limits<double>::max())
		{
			optMsg->GetRequest().strike = m_futuresVal->GetTradePrice();
		}
		m_strike = optMsg->GetRequest().strike;

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// now construct the BlackScholesAdapter from the futures value.
		m_futures = std::make_shared<BlackScholesAssetAdapter>(m_futuresVal, m_vol);

		/// for futures, the yield is the same as interest rate in black scholes world.
		m_futures->SetDivYield(m_termRate);

		/// get the pricing method and run 
		FuturesVanillaOptMessage::Response res;
		if (optMsg->GetRequest().method == FuturesVanillaOptMessage::CLOSED)
		{
			auto mat = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
			res.optPrice = m_futures->option(mat, m_strike, m_termRate, m_optType);
		}
		else if (optMsg->GetRequest().method == FuturesVanillaOptMessage::LATTICE)
		{
			if (optMsg->GetRequest().style == FuturesVanillaOptMessage::EUROPEAN)
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueEuropeanWithBinomial(m_futures, m_termRate, m_maturity, \
					m_strike, static_cast<FuturesVanillaOptionPricer::VanillaOptionType>(m_optType));
			}
			else
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueAmericanWithBinomial(m_futures, m_termRate, m_maturity, \
					m_strike, static_cast<FuturesVanillaOptionPricer::VanillaOptionType>(m_optType));
			}
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set underlying info
		res.underlyingTradeDate = m_futuresVal->GetTradeDate();
		res.underlyingTradePrice = m_futuresVal->GetTradePrice();

		/// now get the greeks
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_futures->delta(mat, m_strike, m_termRate, m_optType);
		res.greeks.gamma = m_futures->gamma(mat, m_strike, m_termRate, m_optType);
		res.greeks.theta = m_futures->theta(mat, m_strike, m_termRate, m_optType);
		res.greeks.vega = m_futures->vega(mat, m_strike, m_termRate, m_optType);

		/// set the message;
		optMsg->SetResponse(res);
	}
}

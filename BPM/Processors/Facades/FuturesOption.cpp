/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesOption.hpp"
#include "DException.hpp"

#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "FuturesVolatilitySurface.hpp"
#include "SystemUtil.hpp"
#include "Exchange.hpp"

#include "VanillaOptMessage.hpp"
#include "FuturesAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	FuturesOption::FuturesOption()
		:m_futuresVal(nullptr),
		m_futures(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
	{}

	void FuturesOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void FuturesOption::Passivate()
	{
		m_futuresVal = nullptr;
		m_futures = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
	}

	void FuturesOption::ProcessVol(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		dd::date today = dd::day_clock::local_day();
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			std::shared_ptr<FuturesVolatilitySurface> volSurface = BuildFuturesVolSurface(optMsg->GetRequest().underlying, today, optMsg->GetRequest().deliveryDate);
			try
			{
				// first try Vol surface
				m_vol = volSurface->GetVolatility(m_maturity, m_strike);
			}
			catch (std::domain_error& e)
			{
				/// means for the maturity not enough data in historic vol
				/// we use GramCharlier to construct constant vol for the given maturity and strike
				m_vol = volSurface->GetConstVol(m_maturity, m_strike);
			}
		}
	}

	void FuturesOption::ProcessRate(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		dd::date today = dd::day_clock::local_day();
		/// get the interest rate
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().rateType == VanillaOptMessage::LIBOR)
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = m_term->simple_rate(0, t);
		}
		else
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = m_term->simple_rate(0, t);
		}
	}
}
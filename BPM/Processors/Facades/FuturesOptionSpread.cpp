/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "FuturesOptionSpread.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "FuturesOptionSpreadMessage.hpp"

#include "Exchange.hpp"
#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "FuturesVolatilitySurface.hpp"

#include "FuturesAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(FuturesOptionSpread);
	ALIAS_REGISTER(FuturesOptionSpread, IMessageSink);
	MSG_REGISTER(FuturesOptionSpreadMessage, FuturesOptionSpread);

	FuturesOptionSpread::FuturesOptionSpread(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	FuturesOptionSpread::FuturesOptionSpread(const Name& nm)
		: m_name(nm),
		m_futuresVal(nullptr),
		m_futures(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
	{}

	std::shared_ptr<IMake> FuturesOptionSpread::Make(const Name &nm)
	{
		/// Construct FuturesOptionSpread from given name and register with EntityManager
		std::shared_ptr<FuturesOptionSpread> optionProc = std::make_shared<FuturesOptionSpread>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> FuturesOptionSpread::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void FuturesOptionSpread::Activate(const std::deque<boost::any>& agrs)
	{}

	void FuturesOptionSpread::Passivate()
	{
		m_futuresVal = nullptr;
		m_futures = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
	}

	void FuturesOptionSpread::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<FuturesOptionSpreadMessage> optMsg = std::dynamic_pointer_cast<FuturesOptionSpreadMessage>(msg);
		m_delivery = optMsg->GetRequest().deliveryDate;

		/// transfer request inputs to member variables 
		/// get futures value.
		dd::date today = dd::day_clock::local_day();
		m_futuresVal = PrimaryUtil::getFuturesValue(optMsg->GetRequest().underlying, today, m_delivery);
				
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			m_volSurface = BuildFuturesVolSurface(optMsg->GetRequest().underlying, today, m_delivery);
		}

		/// get the interest rate
		if (optMsg->GetRequest().rateType == FuturesOptionSpreadMessage::LIBOR)
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
		}
		else
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
		}
		
		/// get the pricing method and run for each leg of the spread
		FuturesOptionSpreadMessage::Response res;
		FuturesOptionSpreadMessage::ResponseLeg resLeg;
		for (auto &req : optMsg->GetRequest().legs)
		{
			ProcessSpreadLeg(resLeg, req, optMsg->GetRequest().method, optMsg->GetRequest().style);
			res.legs.push_back(resLeg);
			if (req.pos == FuturesOptionSpreadMessage::PositionTypeEnum::LONG)
			{ 
				res.spreadPrice += resLeg.optPrice*req.units;
				res.greeks.delta += resLeg.greeks.delta*req.units;
				res.greeks.gamma += resLeg.greeks.gamma*req.units;
				res.greeks.theta += resLeg.greeks.theta*req.units;
				res.greeks.vega += resLeg.greeks.vega*req.units;
			}
			else if (req.pos == FuturesOptionSpreadMessage::PositionTypeEnum::SHORT)
			{
				res.spreadPrice -= resLeg.optPrice*req.units;
				res.greeks.delta -= resLeg.greeks.delta*req.units;
				res.greeks.gamma -= resLeg.greeks.gamma*req.units;
				res.greeks.theta -= resLeg.greeks.theta*req.units;
				res.greeks.vega -= resLeg.greeks.vega*req.units;
			}
			else
			{
				throw std::invalid_argument("Invalid option position");
			}
		}

		/// set the futures info
		res.underlyingTradeDate = m_futuresVal->GetTradeDate();
		res.underlyingTradePrice = m_futuresVal->GetTradePrice();

		/// add Naked position price
		if (optMsg->GetRequest().equityPos.pos == FuturesOptionSpreadMessage::PositionTypeEnum::LONG)
		{
			res.spreadPrice += optMsg->GetRequest().equityPos.units*m_futuresVal->GetTradePrice();
			res.greeks.delta += optMsg->GetRequest().equityPos.units;
		}
		else if (optMsg->GetRequest().equityPos.pos == FuturesOptionSpreadMessage::PositionTypeEnum::SHORT)
		{
			res.spreadPrice -= optMsg->GetRequest().equityPos.units*m_futuresVal->GetTradePrice();
			res.greeks.delta -= optMsg->GetRequest().equityPos.units;
		}

		/// set the message;
		optMsg->SetResponse(res);
	}

	void FuturesOptionSpread::ProcessSpreadLeg(FuturesOptionSpreadMessage::ResponseLeg& res, \
		const FuturesOptionSpreadMessage::Leg& req, FuturesOptionSpreadMessage::PricingMethodEnum method, \
		FuturesOptionSpreadMessage::OptionStyleEnum style)
	{
		auto t = double((req.maturity - dd::day_clock::local_day()).days()) / 365;
		auto rate = PrimaryUtil::getDFToCompoundRate((*m_term)(t), t);
		auto optType = (req.option == FuturesOptionSpreadMessage::CALL) ? 1 : -1;

		if (m_vol == nullptr)
		{
			try
			{
				/// first try Vol surface
				m_vol = m_volSurface->GetVolatility(m_delivery, req.strike);
			}
			catch (std::domain_error& e)
			{
				/// means for the maturity not enough data in historic vol
				/// we use GramCharlier to construct constant vol for the given maturity and strike
				m_vol = m_volSurface->GetConstVol(m_delivery, req.strike);
			}
		}

		/// now construct the BlackScholesAdapter from the futures value.
		if (m_futures == nullptr)
		{
			m_futures = std::make_shared<BlackScholesAssetAdapter>(m_futuresVal, m_vol);
			/// for futures, the yield is the same as interest rate in black scholes world.
			m_futures->SetDivYield(rate);
		}
		else
		{
			m_futures->SetDivYield(rate);
		}

		if (method == FuturesOptionSpreadMessage::CLOSED)
		{
			res.optPrice = m_futures->option(t, req.strike, rate, optType);
		}
		else
		{
			if (style == FuturesOptionSpreadMessage::EUROPEAN)
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueEuropeanWithBinomial(m_futures, rate, req.maturity, req.strike, optType, 100);
			}
			else
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueAmericanWithBinomial(m_futures, rate, req.maturity, req.strike, optType, 100);
			}
		}

		/// now get the greeks
		double mat = (double((req.maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_futures->delta(mat, req.strike, rate, optType);
		res.greeks.gamma = m_futures->gamma(mat, req.strike, rate, optType);
		res.greeks.vega = m_futures->vega(mat, req.strike, rate, optType);
		res.greeks.theta = m_futures->theta(mat, req.strike, rate, optType);
		res.greeks.vega = m_futures->vega(mat, req.strike, rate, optType);	
	}
}

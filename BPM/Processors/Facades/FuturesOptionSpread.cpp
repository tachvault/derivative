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
		m_futuresVal(nullptr)
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
	}

	void FuturesOptionSpread::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<FuturesOptionSpreadMessage> optMsg = std::dynamic_pointer_cast<FuturesOptionSpreadMessage>(msg);
		m_symbol = optMsg->GetRequest().underlying;

		/// get the pricing method and run for each leg of the spread
		FuturesOptionSpreadMessage::Response res;
		FuturesOptionSpreadMessage::ResponseLeg resLeg;
		for (auto &req : optMsg->GetRequest().legs)
		{
			ProcessSpreadLeg(resLeg, req, optMsg->GetRequest().method, optMsg->GetRequest().style, optMsg->GetRequest().rateType);
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

		if (optMsg->GetRequest().equityPos.pos == FuturesOptionSpreadMessage::PositionTypeEnum::LONG)
		{
			/// add Naked position price
			dd::date today = dd::day_clock::local_day();
			m_futuresVal = PrimaryUtil::getFuturesValue(m_symbol, today, optMsg->GetRequest().deliveryDate);

			/// set the futures info
			res.underlyingTradeDate = m_futuresVal->GetTradeDate();
			res.underlyingTradePrice = m_futuresVal->GetTradePrice();

			res.spreadPrice += optMsg->GetRequest().equityPos.units*m_futuresVal->GetTradePrice();
			res.greeks.delta += optMsg->GetRequest().equityPos.units;
		}
		else if (optMsg->GetRequest().equityPos.pos == FuturesOptionSpreadMessage::PositionTypeEnum::SHORT)
		{
			/// add Naked position price
			dd::date today = dd::day_clock::local_day();
			m_futuresVal = PrimaryUtil::getFuturesValue(m_symbol, today, optMsg->GetRequest().deliveryDate);

			/// set the futures info
			res.underlyingTradeDate = m_futuresVal->GetTradeDate();
			res.underlyingTradePrice = m_futuresVal->GetTradePrice();

			res.spreadPrice -= optMsg->GetRequest().equityPos.units*m_futuresVal->GetTradePrice();
			res.greeks.delta -= optMsg->GetRequest().equityPos.units;
		}

		/// set the message;
		optMsg->SetResponse(res);
	}

	void FuturesOptionSpread::ProcessSpreadLeg(FuturesOptionSpreadMessage::ResponseLeg& res, \
		const FuturesOptionSpreadMessage::Leg& req, FuturesOptionSpreadMessage::PricingMethodEnum method, \
		FuturesOptionSpreadMessage::OptionStyleEnum style, FuturesOptionSpreadMessage::RateTypeEnum rateType)
	{
		dd::date today = dd::day_clock::local_day();
		int optType = (req.option == FuturesOptionSpreadMessage::CALL) ? 1 : -1;
		std::shared_ptr<IFuturesValue> futuresVal = PrimaryUtil::getFuturesValue(m_symbol, today, req.delivery);

		/// first try Vol surface
		std::shared_ptr<TermStructure> term;
		/// get the interest rate
		if (rateType == FuturesOptionSpreadMessage::LIBOR)
		{
			/// Get domestic interest rate of the futures
			auto exchange = futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, exchange.GetCountry().GetCode(), today);
			term = irCurve->GetTermStructure();
		}
		else
		{
			/// Get domestic interest rate of the futures
			auto exchange = futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, exchange.GetCountry().GetCode(), today);
			term = irCurve->GetTermStructure();
		}
		auto t = double((req.maturity - dd::day_clock::local_day()).days()) / 365;
		auto rate = term->simple_rate(0, t);

		std::shared_ptr<FuturesVolatilitySurface> volSurface = BuildFuturesVolSurface(m_symbol, today, req.delivery);
		std::shared_ptr<DeterministicAssetVol>  vol;
		try
		{
			vol = volSurface->GetVolatility(req.delivery, req.strike);
		}
		catch (std::domain_error& e)
		{
			/// means for the maturity not enough data in historic vol
			/// we use GramCharlier to construct constant vol for the given maturity and strike
			vol = volSurface->GetConstVol(req.delivery, req.strike);
		}

		/// now construct the BlackScholesAdapter from the futures value.
		std::shared_ptr<BlackScholesAssetAdapter> futures = std::make_shared<BlackScholesAssetAdapter>(futuresVal, vol);
		/// for futures, the yield is the same as interest rate in black scholes world.
		futures->SetDivYield(rate);

		/// get strike price
		if (req.strike == std::numeric_limits<double>::max())
		{
			req.strike = futuresVal->GetTradePrice();
		}

		if (method == FuturesOptionSpreadMessage::CLOSED)
		{
			res.optPrice = futures->option(t, req.strike, rate, optType);
		}
		else
		{
			if (style == FuturesOptionSpreadMessage::EUROPEAN)
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueEuropeanWithBinomial(futures, rate, req.maturity, \
					req.strike, static_cast<FuturesVanillaOptionPricer::VanillaOptionType>(optType));
			}
			else
			{
				res.optPrice = FuturesVanillaOptionPricer::ValueAmericanWithBinomial(futures, rate, req.maturity, \
					req.strike, static_cast<FuturesVanillaOptionPricer::VanillaOptionType>(optType));
			}
		}

		/// now get the greeks
		double mat = (double((req.maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = futures->delta(mat, req.strike, rate, optType);
		res.greeks.gamma = futures->gamma(mat, req.strike, rate, optType);
		res.greeks.vega = futures->vega(mat, req.strike, rate, optType);
		res.greeks.theta = futures->theta(mat, req.strike, rate, optType);
		res.greeks.vega = futures->vega(mat, req.strike, rate, optType);

		/// set the futures info
		res.underlyingTradeDate = futuresVal->GetTradeDate();
		res.underlyingTradePrice = futuresVal->GetTradePrice();
	}
}

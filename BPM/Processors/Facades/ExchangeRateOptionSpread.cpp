/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "ExchangeRateOptionSpread.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "ExchangeRateOptionSpreadMessage.hpp"

#include "CountryHolder.hpp"
#include "Exchange.hpp"
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
	GROUP_REGISTER(ExchangeRateOptionSpread);
	ALIAS_REGISTER(ExchangeRateOptionSpread, IMessageSink);
	MSG_REGISTER(ExchangeRateOptionSpreadMessage, ExchangeRateOptionSpread);
	
	void ExchangeRateOptionSpread::GetRates(const std::shared_ptr<ExchangeRateOptionSpreadMessage>& optMsg, IRCurve::DataSourceType src)
	{
		/// Get domestic interest rate of the ExchangeRate
		dd::date today = dd::day_clock::local_day();
		CountryHolder& cntryHolder = CountryHolder::getInstance();
		std::vector<Country> vec;
		cntryHolder.GetCountry(optMsg->GetRequest().domestic, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid domestic country code");

		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termLocal = irCurve->GetTermStructure();

		/// Get foreign interest rate of the ExchangeRate
		cntryHolder.GetCountry(optMsg->GetRequest().foreign, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid foreign country code");

		irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termForeign = irCurve->GetTermStructure();
	}

	ExchangeRateOptionSpread::ExchangeRateOptionSpread(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	ExchangeRateOptionSpread::ExchangeRateOptionSpread(const Name& nm)
		: m_name(nm),
		m_exchangeRateVal(nullptr),
		m_exchangeRate(nullptr),
		m_vol(nullptr),
		m_termLocal(nullptr),
		m_termForeign(nullptr)
	{}

	std::shared_ptr<IMake> ExchangeRateOptionSpread::Make(const Name &nm)
	{
		/// Construct ExchangeRateOptionSpread from given name and register with EntityManager
		std::shared_ptr<ExchangeRateOptionSpread> optionProc = std::make_shared<ExchangeRateOptionSpread>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> ExchangeRateOptionSpread::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void ExchangeRateOptionSpread::Activate(const std::deque<boost::any>& agrs)
	{}

	void ExchangeRateOptionSpread::Passivate()
	{
		m_exchangeRateVal = nullptr;
		m_exchangeRate = nullptr;
		m_vol = nullptr;
		m_termLocal = nullptr;
		m_termForeign = nullptr;
	}

	void ExchangeRateOptionSpread::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<ExchangeRateOptionSpreadMessage> optMsg = std::dynamic_pointer_cast<ExchangeRateOptionSpreadMessage>(msg);
		m_domestic = optMsg->GetRequest().domestic;
		m_foreign = optMsg->GetRequest().foreign;
		
		/// transfer request inputs to member variables 
		/// get exchangeRate value.
		dd::date today = dd::day_clock::local_day();
		m_exchangeRateVal = PrimaryUtil::getExchangeRateValue(m_domestic, m_foreign);
				
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}

		/// get the interest rate
		if (optMsg->GetRequest().rateType == ExchangeRateOptionSpreadMessage::LIBOR)
		{
			GetRates(optMsg, IRCurve::LIBOR);
		}
		else
		{
			GetRates(optMsg, IRCurve::YIELD);
		}
		
		/// get the pricing method and run for each leg of the spread
		ExchangeRateOptionSpreadMessage::Response res;
		ExchangeRateOptionSpreadMessage::ResponseLeg resLeg;
		for (auto &req : optMsg->GetRequest().legs)
		{
			ProcessSpreadLeg(resLeg, req, optMsg->GetRequest().method, optMsg->GetRequest().style);
			res.legs.push_back(resLeg);
			if (req.pos == ExchangeRateOptionSpreadMessage::PositionTypeEnum::LONG)
			{ 
				res.spreadPrice += resLeg.optPrice*req.units;
				res.greeks.delta += resLeg.greeks.delta*req.units;
				res.greeks.gamma += resLeg.greeks.gamma*req.units;
				res.greeks.theta += resLeg.greeks.theta*req.units;
				res.greeks.vega += resLeg.greeks.vega*req.units;
			}
			else if (req.pos == ExchangeRateOptionSpreadMessage::PositionTypeEnum::SHORT)
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

		/// set the exchangeRate info
		res.underlyingTradeDate = m_exchangeRateVal->GetTradeDate();
		res.underlyingTradePrice = m_exchangeRateVal->GetTradePrice();

		/// add Naked position price
		if (optMsg->GetRequest().equityPos.pos == ExchangeRateOptionSpreadMessage::PositionTypeEnum::LONG)
		{
			res.spreadPrice += optMsg->GetRequest().equityPos.units*m_exchangeRateVal->GetTradePrice();
			res.greeks.delta += optMsg->GetRequest().equityPos.units;
		}
		else if (optMsg->GetRequest().equityPos.pos == ExchangeRateOptionSpreadMessage::PositionTypeEnum::SHORT)
		{
			res.spreadPrice -= optMsg->GetRequest().equityPos.units*m_exchangeRateVal->GetTradePrice();
			res.greeks.delta -= optMsg->GetRequest().equityPos.units;
		}

		/// set the message;
		optMsg->SetResponse(res);
	}

	void ExchangeRateOptionSpread::ProcessSpreadLeg(ExchangeRateOptionSpreadMessage::ResponseLeg& res, \
		const ExchangeRateOptionSpreadMessage::Leg& req, ExchangeRateOptionSpreadMessage::PricingMethodEnum method, \
		ExchangeRateOptionSpreadMessage::OptionStyleEnum style)
	{
		auto t = double((req.maturity - dd::day_clock::local_day()).days()) / 365;
		auto rateLocal = PrimaryUtil::getDFToCompoundRate((*m_termLocal)(t), t);
		auto rateForeign = PrimaryUtil::getDFToCompoundRate((*m_termForeign)(t), t);
		auto optType = (req.option == ExchangeRateOptionSpreadMessage::CALL) ? 1 : -1;

		if (m_vol == nullptr)
		{
			/// get the historical volatility
			dd::date today = dd::day_clock::local_day();
			std::shared_ptr<ExchangeRateGARCH> garch = BuildExchangeRateGARCH(m_domestic, m_foreign, today);
			try
			{
				m_vol = garch->GetVolatility();
			}
			catch (std::domain_error& e)
			{
				cout << "Error " << e.what() << endl;
				throw e;
			}
		}

		/// now construct the BlackScholesAdapter from the exchangeRate value.
		if (m_exchangeRate == nullptr)
		{
			m_exchangeRate = std::make_shared<BlackScholesAssetAdapter>(m_exchangeRateVal, m_vol);
		}
		
		/// for exchangeRate, the yield is the same as interest rate in black scholes world.
		m_exchangeRate->SetDivYield(rateForeign);
		
		if (method == ExchangeRateOptionSpreadMessage::CLOSED)
		{
			res.optPrice = m_exchangeRate->option(t, req.strike, rateLocal, optType);
		}
		else
		{
			if (style == ExchangeRateOptionSpreadMessage::EUROPEAN)
			{
				res.optPrice = ExchangeRateVanillaOptionPricer::ValueEuropeanWithBinomial(m_exchangeRate, rateLocal, req.maturity, req.strike, optType, 100);
			}
			else
			{
				res.optPrice = ExchangeRateVanillaOptionPricer::ValueAmericanWithBinomial(m_exchangeRate, rateLocal, req.maturity, req.strike, optType, 100);
			}
		}

		/// now get the greeks
		double mat = (double((req.maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_exchangeRate->delta(mat, req.strike, rateLocal, optType);
		res.greeks.gamma = m_exchangeRate->gamma(mat, req.strike, rateLocal, optType);
		res.greeks.vega = m_exchangeRate->vega(mat, req.strike, rateLocal, optType);
		res.greeks.theta = m_exchangeRate->theta(mat, req.strike, rateLocal, optType);
		res.greeks.vega = m_exchangeRate->vega(mat, req.strike, rateLocal, optType);	
	}
}

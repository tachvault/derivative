/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityOptionSpread.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityOptionSpreadMessage.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "EquityVolatilitySurface.hpp"
#include "EquityGARCH.hpp"

#include "EquityAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(EquityOptionSpread);
	ALIAS_REGISTER(EquityOptionSpread, IMessageSink);
	MSG_REGISTER(EquityOptionSpreadMessage, EquityOptionSpread);

	EquityOptionSpread::EquityOptionSpread(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	EquityOptionSpread::EquityOptionSpread(const Name& nm)
		: m_name(nm),
		m_stockVal(nullptr),
		m_stock(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
	{}

	std::shared_ptr<IMake> EquityOptionSpread::Make(const Name &nm)
	{
		/// Construct EquityOptionSpread from given name and register with EntityManager
		std::shared_ptr<EquityOptionSpread> optionProc = std::make_shared<EquityOptionSpread>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityOptionSpread::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void EquityOptionSpread::Activate(const std::deque<boost::any>& agrs)
	{}

	void EquityOptionSpread::Passivate()
	{
		m_stockVal = nullptr;
		m_stock = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
	}

	void EquityOptionSpread::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityOptionSpreadMessage> optMsg = std::dynamic_pointer_cast<EquityOptionSpreadMessage>(msg);

		/// transfer request inputs to member variables 
		/// get stock value.
		m_symbol = optMsg->GetRequest().underlying;
		m_stockVal = PrimaryUtil::getStockValue(m_symbol);

		dd::date today = dd::day_clock::local_day();
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}

		/// get the interest rate
		if (optMsg->GetRequest().rateType == EquityOptionSpreadMessage::LIBOR)
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
		}
		else
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
		}

		/// get the pricing method and run for each leg of the spread
		EquityOptionSpreadMessage::Response res;
		EquityOptionSpreadMessage::ResponseLeg resLeg;
		for (auto &req : optMsg->GetRequest().legs)
		{
			ProcessSpreadLeg(resLeg, req, optMsg->GetRequest().method, optMsg->GetRequest().style, optMsg->GetRequest().volType);
			res.legs.push_back(resLeg);
			if (req.pos == EquityOptionSpreadMessage::PositionTypeEnum::LONG)
			{
				res.spreadPrice += resLeg.optPrice*req.units;
				res.greeks.delta += resLeg.greeks.delta*req.units;
				res.greeks.gamma += resLeg.greeks.gamma*req.units;
				res.greeks.theta += resLeg.greeks.theta*req.units;
				res.greeks.vega += resLeg.greeks.vega*req.units;
			}
			else if (req.pos == EquityOptionSpreadMessage::PositionTypeEnum::SHORT)
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
		res.underlyingTradeDate = m_stockVal->GetTradeDate();
		res.underlyingTradePrice = m_stockVal->GetTradePrice();

		/// add Naked position price
		if (optMsg->GetRequest().equityPos.pos == EquityOptionSpreadMessage::PositionTypeEnum::LONG)
		{
			res.spreadPrice += optMsg->GetRequest().equityPos.units*m_stockVal->GetTradePrice();
			res.greeks.delta += optMsg->GetRequest().equityPos.units;
		}
		else if (optMsg->GetRequest().equityPos.pos == EquityOptionSpreadMessage::PositionTypeEnum::SHORT)
		{
			res.spreadPrice -= optMsg->GetRequest().equityPos.units*m_stockVal->GetTradePrice();
			res.greeks.delta -= optMsg->GetRequest().equityPos.units;
		}

		/// set the message;
		optMsg->SetResponse(res);
	}

	void EquityOptionSpread::ProcessSpreadLeg(EquityOptionSpreadMessage::ResponseLeg& res, \
		const EquityOptionSpreadMessage::Leg& req, EquityOptionSpreadMessage::PricingMethodEnum method, \
		EquityOptionSpreadMessage::OptionStyleEnum style, EquityOptionSpreadMessage::VolatilityTypeEnum volType)
	{
		auto t = double((req.maturity - dd::day_clock::local_day()).days()) / 365;
		auto rate = m_term->simple_rate(0, t);
		auto optType = (req.option == EquityOptionSpreadMessage::CALL) ? 1 : -1;

		if (m_vol == nullptr)
		{
			if (volType == EquityOptionSpreadMessage::IV)
			{
				dd::date today = dd::day_clock::local_day();
				m_volSurface = BuildEquityVolSurface(m_symbol, today);
				try
				{
					/// first try Vol surface
					m_vol = m_volSurface->GetVolatility(req.maturity, req.strike);
				}
				catch (std::domain_error& e)
				{
					/// means for the maturity not enough data in historic vol
					/// we use GramCharlier to construct constant vol for the given maturity and strike
					m_vol = m_volSurface->GetConstVol(req.maturity, req.strike);
				}
			}
			else
			{
				dd::date today = dd::day_clock::local_day();
				std::shared_ptr<EquityGARCH> garch = BuildEquityGARCH(m_symbol, today);
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
		}
		
		if (req.strike == std::numeric_limits<double>::max())
		{
			req.strike = m_stockVal->GetTradePrice();
		}

		/// now construct the BlackScholesAdapter from the stock value.
		if (m_stock == nullptr)
		{
			m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);
		}

		if (method == EquityOptionSpreadMessage::CLOSED)
		{
			res.optPrice = m_stock->option(t, req.strike, rate, optType);
		}
		else if (method == EquityOptionSpreadMessage::LATTICE)
		{
			if (style == EquityOptionSpreadMessage::EUROPEAN)
			{
				res.optPrice = VanillaOptionPricer::ValueEuropeanWithBinomial(m_stock, rate, req.maturity, \
					req.strike, static_cast<VanillaOptionPricer::VanillaOptionType>(optType), 100);
			}
			else
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithBinomial(m_stock, rate, req.maturity, \
					req.strike, static_cast<VanillaOptionPricer::VanillaOptionType>(optType), 100);
			}
		}
		else if (method == EquityOptionSpreadMessage::MONTE_CARLO)
		{
			if (style == EquityOptionSpreadMessage::EUROPEAN)
			{
				res.optPrice = VanillaOptionPricer::ValueEuropeanWithMC(m_stock, \
					m_term, req.maturity, req.strike, static_cast<VanillaOptionPricer::VanillaOptionType>(optType));
			}
			else
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithMC(m_stock, \
					m_term, req.maturity, req.strike, static_cast<VanillaOptionPricer::VanillaOptionType>(optType));
			}
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// now get the greeks
		double mat = (double((req.maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_stock->delta(mat, req.strike, rate, optType);
		res.greeks.gamma = m_stock->gamma(mat, req.strike, rate, optType);
		res.greeks.vega = m_stock->vega(mat, req.strike, rate, optType);
		res.greeks.theta = m_stock->theta(mat, req.strike, rate, optType);
		res.greeks.vega = m_stock->vega(mat, req.strike, rate, optType);	
	}
}

/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityVanillaOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityVanillaOptMessage.hpp"

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
	GROUP_REGISTER(EquityVanillaOption);
	ALIAS_REGISTER(EquityVanillaOption, IMessageSink);
	MSG_REGISTER(EquityVanillaOptMessage, EquityVanillaOption);

	EquityVanillaOption::EquityVanillaOption(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	EquityVanillaOption::EquityVanillaOption(const Name& nm)
		: m_name(nm),
		m_stockVal(nullptr),
		m_stock(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
	{}

	std::shared_ptr<IMake> EquityVanillaOption::Make(const Name &nm)
	{
		/// Construct EquityVanillaOption from given name and register with EntityManager
		std::shared_ptr<EquityVanillaOption> optionProc = std::make_shared<EquityVanillaOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityVanillaOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void EquityVanillaOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void EquityVanillaOption::Passivate()
	{
		m_stockVal = nullptr;
		m_stock = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
	}

	void EquityVanillaOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityVanillaOptMessage> optMsg = std::dynamic_pointer_cast<EquityVanillaOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityVanillaOptMessage::CALL) ? 1 : -1;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);

		dd::date today = dd::day_clock::local_day();
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			if (optMsg->GetRequest().volType == EquityVanillaOptMessage::IV)
			{
				std::shared_ptr<EquityVolatilitySurface> volSurface = BuildEquityVolSurface(optMsg->GetRequest().underlying, today);

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
			else
			{
				std::shared_ptr<EquityGARCH> garch = BuildEquityGARCH(optMsg->GetRequest().underlying, today);
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

		/// get the interest rate
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().rateType == EquityVanillaOptMessage::LIBOR)
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = PrimaryUtil::getDFToCompoundRate((*m_term)(t), t);
		}
		else
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = PrimaryUtil::getDFToCompoundRate((*m_term)(t), t);
		}

		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityVanillaOptMessage::Response res;
		if (optMsg->GetRequest().method == EquityVanillaOptMessage::CLOSED)
		{
			res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		}
		else if (optMsg->GetRequest().method == EquityVanillaOptMessage::LATTICE)
		{
			if (optMsg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN)
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithBinomial(m_stock, m_termRate, m_maturity, m_strike, m_optType);
			}
			else
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithBinomial(m_stock, m_termRate, m_maturity, m_strike, m_optType);
			}
		}
		else if (optMsg->GetRequest().method == EquityVanillaOptMessage::MONTE_CARLO)
		{
			if (optMsg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN)
			{
				std::vector<VanillaOptionPricer::MCValueType>  ret = VanillaOptionPricer::ValueEuropeanWithMC(m_stock, \
					m_term, m_maturity, m_strike, m_optType);
				res.optPrice = ret.begin()->value;
			}
			else
			{
				std::vector<VanillaOptionPricer::MCValueType> ret = VanillaOptionPricer::ValueAmericanWithMC(m_stock, \
					m_term, m_maturity, m_strike, m_optType);
				res.optPrice = ret.begin()->value;
			}
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		res.underlyingTradeDate = m_stockVal->GetTradeDate();
		res.underlyingTradePrice = m_stockVal->GetTradePrice();
		/// now get the greeks
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_stock->delta(mat, m_strike, m_termRate);
		res.greeks.gamma = m_stock->gamma(mat, m_strike, m_termRate);
		res.greeks.vega = m_stock->vega(mat, m_strike, m_termRate);
		/// res.greeks.theta = m_stock->theta(...);
		/// res.greeks.vega = m_stock->vega(...);

		/// set the message;
		optMsg->SetResponse(res);
	}
}

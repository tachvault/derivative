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
#include "FuturesVolatilitySurface.hpp"
#include "Exchange.hpp"

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
		: m_name(nm),
		m_futuresVal(nullptr),
		m_futures(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
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

	void FuturesVanillaOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void FuturesVanillaOption::Passivate()
	{
		m_futuresVal = nullptr;
		m_futures = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
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

		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			std::shared_ptr<FuturesVolatilitySurface> volSurface = BuildFuturesVolSurface(optMsg->GetRequest().underlying, today, optMsg->GetRequest().deliveryDate);
			m_vol = volSurface->GetVolatility(m_maturity,m_strike);
		}

		/// get the interest rate for the delivery time (Not for the option maturity)
		auto t = double((m_delivery - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().rateType == FuturesVanillaOptMessage::LIBOR)
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = PrimaryUtil::getDFToCompoundRate((*m_term)(t), t);
		}
		else
		{
			/// Get domestic interest rate of the futures
			auto exchange = m_futuresVal->GetFutures()->GetExchange();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, exchange.GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = PrimaryUtil::getDFToCompoundRate((*m_term)(t), t);
		}

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
				ValueEuropeanWithBinomial();
			}
			else
			{
				ValueAmericanWithBinomial();
			}
			res.optPrice = m_binomial;
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
		res.greeks.delta = m_futures->delta(mat, m_strike, m_termRate);
		res.greeks.gamma = m_futures->gamma(mat, m_strike, m_termRate);
		/// res.greeks.theta = m_futures->theta(...);
		res.greeks.vega = m_futures->vega(mat, m_strike, m_termRate);

		/// set the message;
		optMsg->SetResponse(res);
	}

	void FuturesVanillaOption::ValueAmericanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_futures, m_termRate, mat, N);
			Payoff optPayoff(m_strike, m_optType);
			std::function<double(double)> f;
			f = std::bind(std::mem_fun(&Payoff::operator()), &optPayoff, std::placeholders::_1);
			btree.apply_payoff(N - 1, f);
			EarlyExercise amOpt(optPayoff);
			std::function<double(double, double)> g;
			g = std::bind(std::mem_fn(&EarlyExercise::operator()), &amOpt, std::placeholders::_1, std::placeholders::_2);
			btree.set_CoxRossRubinstein();
			btree.apply_payoff(N - 1, f);
			btree.rollback(N - 1, 0, g);
			m_binomial = btree.result();

		} // end of try block
		catch (std::logic_error& e)
		{
			LOG(ERROR) << e.what() << endl;
			throw e;
		}
		catch (std::runtime_error& e)
		{
			LOG(ERROR) << e.what() << endl;
			throw e;
		}
	};

	void FuturesVanillaOption::ValueEuropeanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_futures, m_termRate, mat, N);
			Payoff optPayoff(m_strike, m_optType);
			std::function<double(double)> f;
			f = std::bind(std::mem_fun(&Payoff::operator()), &optPayoff, std::placeholders::_1);
			btree.apply_payoff(N - 1, f);
			btree.rollback(N - 1, 0);
			m_binomial = btree.result();

		} // end of try block
		catch (std::logic_error& e)
		{
			LOG(ERROR) << e.what() << endl;
			throw e;
		}
		catch (std::runtime_error& e)
		{
			LOG(ERROR) << e.what() << endl;
			throw e;
		}
	};
}

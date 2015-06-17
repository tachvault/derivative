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

	void ExchangeRateVanillaOption::GetRates(const std::shared_ptr<ExchangeRateVanillaOptMessage>& optMsg, IRCurve::DataSourceType src)
	{
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		/// Get domestic interest rate of the ExchangeRate
		CountryHolder& cntryHolder = CountryHolder::getInstance();
		std::vector<Country> vec;
		cntryHolder.GetCountry(optMsg->GetRequest().domestic, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid domestic country code");

		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termLocal = irCurve->GetTermStructure();
		m_localRate = PrimaryUtil::getDFToCompoundRate((*m_termLocal)(t), t);

		/// Get foreign interest rate of the ExchangeRate
		cntryHolder.GetCountry(optMsg->GetRequest().foreign, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid foreign country code");

		irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termForeign = irCurve->GetTermStructure();
		m_foreignRate = PrimaryUtil::getDFToCompoundRate((*m_termForeign)(t), t);
	}

	ExchangeRateVanillaOption::ExchangeRateVanillaOption(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	ExchangeRateVanillaOption::ExchangeRateVanillaOption(const Name& nm)
		: m_name(nm),
		m_ExchangeRateVal(nullptr),
		m_ExchangeRate(nullptr),
		m_vol(nullptr),
		m_termLocal(nullptr),
		m_termForeign(nullptr),
		m_localRate(0.0),
		m_foreignRate(0.0)
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

	void ExchangeRateVanillaOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void ExchangeRateVanillaOption::Passivate()
	{
		m_ExchangeRateVal = nullptr;
		m_ExchangeRate = nullptr;
		m_vol = nullptr;
		m_termLocal = nullptr;
		m_termForeign == nullptr;
		m_localRate = 0.0;
		m_foreignRate = 0.0;
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
		m_ExchangeRateVal = PrimaryUtil::getExchangeRateValue(optMsg->GetRequest().domestic, optMsg->GetRequest().foreign);

		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			/// get the historical volatility
			std::shared_ptr<ExchangeRateGARCH> garch = BuildExchangeRateGARCH(optMsg->GetRequest().domestic, optMsg->GetRequest().foreign, today);
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

		/// get the interest rate for the delivery time (Not for the option maturity)
		if (optMsg->GetRequest().rateType == ExchangeRateVanillaOptMessage::LIBOR)
		{
			GetRates(optMsg, IRCurve::LIBOR);
		}
		else
		{
			GetRates(optMsg, IRCurve::YIELD);
		}

		/// now construct the BlackScholesAdapter from the ExchangeRate value.
		m_ExchangeRate = std::make_shared<BlackScholesAssetAdapter>(m_ExchangeRateVal, m_vol);

		/// for ExchangeRate, the yield is the same as interest rate in black scholes world.
		m_ExchangeRate->SetDivYield(m_foreignRate);

		/// get the pricing method and run 
		ExchangeRateVanillaOptMessage::Response res;
		if (optMsg->GetRequest().method == ExchangeRateVanillaOptMessage::CLOSED)
		{
			auto mat = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
			res.optPrice = m_ExchangeRate->option(mat, m_strike, m_localRate, m_optType);
		}
		else if (optMsg->GetRequest().method == ExchangeRateVanillaOptMessage::LATTICE)
		{
			if (optMsg->GetRequest().style == ExchangeRateVanillaOptMessage::EUROPEAN)
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
		res.underlyingTradeDate = m_ExchangeRateVal->GetTradeDate();
		res.underlyingTradePrice = m_ExchangeRateVal->GetTradePrice();

		/// now get the greeks
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_ExchangeRate->delta(mat, m_strike, m_localRate, m_optType);
		res.greeks.gamma = m_ExchangeRate->gamma(mat, m_strike, m_localRate, m_optType);
		res.greeks.theta = m_ExchangeRate->theta(mat, m_strike, m_localRate, m_optType);
		res.greeks.vega = m_ExchangeRate->vega(mat, m_strike, m_localRate, m_optType);

		/// set the message;
		optMsg->SetResponse(res);
	}

	void ExchangeRateVanillaOption::ValueAmericanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_ExchangeRate, m_localRate, mat, N);
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

	void ExchangeRateVanillaOption::ValueEuropeanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_ExchangeRate, m_localRate, mat, N);
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

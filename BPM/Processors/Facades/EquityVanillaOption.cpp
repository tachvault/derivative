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
#include "EquityVolatilitySurface.hpp"

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
			std::shared_ptr<EquityVolatilitySurface> volSurface = BuildEquityVolSurface(optMsg->GetRequest().underlying, today);
			/// we use GramCharlier to construct constant vol for the given maturity and strike
		    m_vol = volSurface->GetConstVol(m_maturity, m_strike);
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
				ValueEuropeanWithBinomial();
			}
			else
			{
				ValueAmericanWithBinomial();
			}
			res.optPrice = m_binomial;
		}
		else if (optMsg->GetRequest().method == EquityVanillaOptMessage::MONTE_CARLO)
		{
			if (optMsg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN)
			{
				ValueEuropeanWithMC();
			}
			else
			{
				ValueAmericanWithMC();
			}
			res.optPrice = m_mc.begin()->value;
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

	void EquityVanillaOption::ValueAmericanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_stock, m_termRate, mat, N);
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

	void EquityVanillaOption::ValueEuropeanWithBinomial(int N)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			BinomialLattice btree(m_stock, m_termRate, mat, N);
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

	void EquityVanillaOption::ValueEuropeanWithMC(size_t minpaths, size_t maxpaths, size_t N, size_t train, int degree, double ci)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			double strike = m_strike;
			int numeraire_index = -1;
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = mat / N;
			T = idx*dt;
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
			underlying.push_back(m_stock);
			TermStructure& ts = *m_term;
			unsigned long n = minpaths;
			// instantiate random number generator
			ranlib::NormalUnit<double> normalRNG;
			RandomArray<ranlib::NormalUnit<double>, double> random_container2(normalRNG, 1, 1); // 1 factor, 1 time step
			// instantiate stochastic process
			GeometricBrownianMotion gbm(underlying);
			// 95% quantile for confidence interval
			boost::math::normal normal;
			double d = boost::math::quantile(normal, 0.95);
			// boost functor to convert random variates to their antithetics (instantiated from template)
			std::function<Array<double, 2>(Array<double, 2>)> antithetic = normal_antithetic < Array<double, 2> > ;
			// instantiate MCGatherer objects to collect simulation results
			MCGatherer<double> mcgatherer;
			MCGatherer<double> mcgatherer_antithetic;
			// instantiate MCPayoff object
			MCEuropeanVanilla mc_opt(0, mat, 0, strike, m_optType);
			// instantiate MCMapping and bind to functor
			MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping2(mc_opt, gbm, ts, numeraire_index);
			std::function<double(Array<double, 2>)> func2 = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping2, std::placeholders::_1);
			// instantiate generic Monte Carlo algorithm object
			MCGeneric<Array<double, 2>, double, RandomArray<ranlib::NormalUnit<double>, double> > mc2(func2, random_container2);
			MCGeneric<Array<double, 2>, double, RandomArray<ranlib::NormalUnit<double>, double> > mc2_antithetic(func2, random_container2, antithetic);
			// run Monte Carlo for different numbers of simulations
			while (mcgatherer.number_of_simulations() < maxpaths)
			{
				mc2.simulate(mcgatherer, n);
				// half as many paths for antithetic
				mc2_antithetic.simulate(mcgatherer_antithetic, n / 2);
				MCValueType value(mcgatherer.number_of_simulations(), mcgatherer.mean(), mcgatherer.mean() - d*mcgatherer.stddev(), mcgatherer.mean() + d*mcgatherer.stddev());
				m_mc.push_back(value);
				n = mcgatherer.number_of_simulations();
			}
		} // end of try block
		catch (std::logic_error& xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (std::runtime_error xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (...)
		{
			LOG(ERROR) << "Other exception caught" << endl;
			throw std::exception("unknown error in MCVanillaEuropean");
		}
	}

	void EquityVanillaOption::ValueAmericanWithMC(size_t minpaths, size_t maxpaths, size_t N, size_t train, int degree, double ci)
	{
		try
		{
			double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
			int numeraire_index = -1;
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = mat / N;
			T = idx*dt;

			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
			underlying.push_back(m_stock);
			TermStructure& ts = *m_term;
			bool include_put = false;
			exotics::StandardOption Mput(m_stock, T(0), mat, m_strike, ts, m_optType);
			GeometricBrownianMotion gbm(underlying);
			gbm.set_timeline(T);
			ranlib::NormalUnit<double> normalRNG;
			RandomArray<ranlib::NormalUnit<double>, double> random_container(normalRNG, gbm.factors(), gbm.number_of_steps());
			MCTrainingPaths<GeometricBrownianMotion, RandomArray<ranlib::NormalUnit<double>, double> >
				training_paths(gbm, T, train, random_container, ts, numeraire_index);
			Payoff put(m_strike, -1);
			std::function<double(double)> f;
			f = std::bind(std::mem_fun(&Payoff::operator()), &put, std::placeholders::_1);
			std::function<double(const Array<double, 1>&, const Array<double, 2>&)> payoff = std::bind(REBAdapter, std::placeholders::_1, std::placeholders::_2, f, 0);
			std::vector<std::function<double(const Array<double, 1>&, const Array<double, 2>&)> > basisfunctions;
			Array<int, 1> p(1);
			for (int i = 0; i <= degree; i++)
			{
				p(0) = i;
				add_polynomial_basis_function(basisfunctions, p);
			}
			std::function<double(double, double)> put_option;
			put_option = std::bind((static_cast<double (exotics::StandardOption::*)(double, double)>(&exotics::StandardOption::price)), &Mput, std::placeholders::_1, std::placeholders::_2);
			std::function<double(const Array<double, 1>&, const Array<double, 2>&)> put_option_basis_function = std::bind(REBAdapterT, std::placeholders::_1, std::placeholders::_2, put_option, 0);
			if (include_put) basisfunctions.push_back(put_option_basis_function);
			RegressionExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
			LSExerciseStrategy<RegressionExerciseBoundary> exercise_strategy(boundary);
			MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
			std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
			MCGeneric<Array<double, 2>, double, RandomArray<ranlib::NormalUnit<double>, double> > mc(func, random_container);
			MCGatherer<double> mcgatherer;
			size_t n = minpaths;
			boost::math::normal normal;
			double d = boost::math::quantile(normal, 0.95);
			while (mcgatherer.number_of_simulations() < maxpaths)
			{
				mc.simulate(mcgatherer, n);
				MCValueType value(mcgatherer.number_of_simulations(), mcgatherer.mean(), mcgatherer.mean() - d*mcgatherer.stddev(), mcgatherer.mean() + d*mcgatherer.stddev());
				m_mc.push_back(value);
				n = mcgatherer.number_of_simulations();
			}
		} // end of try block

		catch (std::logic_error xcpt)
		{
			std::cerr << xcpt.what() << endl;
		}
		catch (std::runtime_error xcpt)
		{
			std::cerr << xcpt.what() << endl;
		}
		catch (...)
		{
			std::cerr << "Other exception caught" << endl;
		}
	}

}

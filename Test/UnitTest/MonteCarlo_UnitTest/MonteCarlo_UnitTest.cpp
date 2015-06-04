/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// MonteCarlo_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <cstdlib>
#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>
#include <boost/filesystem.hpp> 
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include "gtest/gtest.h"

#include "BlackScholesAssetAdapter.hpp"
#include "LongstaffSchwartz.hpp"
#include "PiecewiseVol.hpp"
#include "GeometricBrownianMotion.hpp"
#include "Payoff.hpp"
#include "StringForm.hpp"
#include "MBinary.hpp"
#include "MExotics.hpp"
#include "MCMapping.hpp"
#include "MCControlVariateMapping.hpp"
#include "MCPayoff.hpp"
#include "MCGeneric.hpp"
#include "QFRandom.hpp"
#include "ConstVol.hpp"
#include "Global.hpp"
#include "LongstaffSchwartz.hpp"
#include "QFRandom.hpp"
#include "MCAmerican.hpp"
#include "MCGeneric.hpp"
#include "MExotics.hpp"
#include "QFRandom.hpp"
#include "PiecewiseVol.hpp"
#include "QMCEngine.hpp"
#include "QFQuasiRandom.hpp"

#include "EntityManager.hpp"
#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "gtest/gtest.h"
#include "Windows.h"
#include "EntityMgrUtil.hpp"
#include "CurrencyHolder.hpp"
#include "ExchangeHolder.hpp"
#include "Global.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "MockStock.hpp"
#include "MockStockValue.hpp"
#include "HistoricalStockInfo.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "HistoricalExchangeRateInfo.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "Payoff.hpp"
#include "Binomial.hpp"
#include "ConstVol.hpp"
#include "IStock.hpp"
#include "PiecewiseVol.hpp"
#include "StringForm.hpp"
#include "MBinary.hpp"
#include "MExotics.hpp"
#include "IRCurve.hpp"

#include "IIR.hpp"
#include "IIRValue.hpp"

#undef min;
#undef max;

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

//using namespace std::placeholders;

// Declare a new test fixture for MonteCarloTest, deriving from testing::Test.
class MonteCarloTest : public testing::Test
{
protected:

	/// virtual void SetUp() will be called before each test is run.  
	/// That is, we have load EntityMgmt DLL and initialize
	virtual void SetUp()
	{
		std::cout << " Setup completed in Setup() routine" << std::endl;
	}

	/// virtual void TearDown() will be called after each test is run.
	/// You should define it if there is cleanup work to do.  Otherwise,
	/// you don't have to provide it.
	///
	virtual void TearDown()
	{
		std::cout << " Tests completed: TearDown() called " << std::endl;
	}

};

/// Direct implementation of class representing discounted payoff of a down-and-out barrier call option.
class down_and_out_call : public MCPayoff
{
public:
	double strike, barrier;
	down_and_out_call(const Array<double, 1>& T, int underlying_index, double xstrike, double xbarrier);
	/// Calculate discounted payoff. 
	virtual double operator()(const Array<double, 1>& underlying_values, ///< Underlying values for the (asset,time) combinations in index Array.
		const Array<double, 1>& numeraire_values   ///< Numeraire values for the dates in timeline Array.
		);
};

/// Constructor.
down_and_out_call::down_and_out_call(const Array<double, 1>& T, int underlying_index, double xstrike, double xbarrier)
	: MCPayoff(T, T.extent(firstDim) - 1), strike(xstrike), barrier(xbarrier)
{
	firstIndex  idx;
	secondIndex jdx;
	index = idx * (jdx + 1);
}

/// Calculated discounted payoff for a given path and numeraire.
double down_and_out_call::operator()(const Array<double, 1>& underlying_values, const Array<double, 1>& numeraire_values)
{
	int i;
	bool indicator = true;
	double result = 0.0;
	for (i = 0; i<underlying_values.extent(firstDim); i++) if (underlying_values(i) <= barrier) indicator = false;
	if (indicator) result = numeraire_values(0) / numeraire_values(numeraire_values.extent(firstDim) - 1) * std::max(0.0, underlying_values(underlying_values.extent(firstDim) - 1) - strike);
	return result;
}

inline double positivePart(double x)
{
	return (x>0.0) ? x : 0.0;
}

class objective_class {
private:
	double initial_stock_price;
	double maturity;
	double interest_rate;
	double volatility;
	double strike;
	double discount_factor;
	double sqrt_maturity;
public:
	inline objective_class(double S, double T, double r, double sgm, double K)
		: initial_stock_price(S), maturity(T), interest_rate(r), volatility(sgm), strike(K), discount_factor(std::exp(-r*T)), sqrt_maturity(std::sqrt(maturity)) { };
	Array<double, 1> func(double x);
	double func2(double x);
};

// Return discounted payoff of call and put
Array<double, 1> objective_class::func(double x)
{
	Array<double, 1> payoff(2);
	payoff(0) = discount_factor * positivePart(initial_stock_price*std::exp((interest_rate - 0.5*volatility*volatility)*maturity + volatility*sqrt_maturity*x) - strike);
	payoff(1) = discount_factor * positivePart(strike - initial_stock_price*std::exp((interest_rate - 0.5*volatility*volatility)*maturity + volatility*sqrt_maturity*x));
	return payoff;
}

double objective_class::func2(double x)
{
	return discount_factor * positivePart(initial_stock_price*std::exp((interest_rate - 0.5*volatility*volatility)*maturity + volatility*sqrt_maturity*x) - strike);
}

std::shared_ptr<IStockValue> getStockValue(const std::string& symbol, double price, double vol, double yield)
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// find an object that is in the registry
	std::shared_ptr<IStockValue> stockVal;
	std::shared_ptr<IStock> stock;
	Name nm = IStockValue::ConstructName(symbol);
	std::vector<std::shared_ptr<IObject> > objs;
	try
	{
		bool retValue = entMgr.findAlias(nm, objs);
	}
	catch (...)
	{
	}
	if (objs.empty())
	{
		/// construct a new objects and register
		Name stockValName(MockStockValue::TYPEID, std::hash<std::string>()(symbol));
		stockValName.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
		stockVal = std::make_shared<MockStockValue>(stockValName);
		entMgr.registerObject(stockValName, stockVal);

		Name stockName(MockStock::TYPEID, std::hash<std::string>()(symbol));
		stockName.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
		stock = std::make_shared<MockStock>(stockName);
		entMgr.registerObject(nm, stock);
		stockVal->SetStock(stock);
	}
	else
	{
		/// return the stock val found in registry
		stockVal = dynamic_pointer_cast<IStockValue>(*objs.begin());
	}

	stockVal->SetTradePrice(price);
	stock = stockVal->GetStock();
	stock->SetImpliedVol(vol);
	stockVal->SetDivYield(yield);

	return stockVal;
}

TEST_F(MonteCarloTest, CVTest)
{
	std::cout << "================================================================" << std::endl;
	std::cout << " Control Variates Test" << std::endl;

	try
	{
		std::string symbol("AAPL");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);

		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		Array<double, 1> sgm1(2);
		sgm1 = 0.3;

		/// obtain Black Scholes option pricing
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal);

		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " \
			<< mat << endl;

		double CFcall = stock->option(mat, K, r);
		double CFput = stock->option(mat, K, r, -1);
		double CFiput = stock->option(T(N / 2), K, r, -1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);
		FlatTermStructure ts(r, 0.0, mat + 10.0);

		// Margrabe option price by Monte Carlo
		MCGatherer<double> mcgatherer;
		// boost functor to convert random variates to their antithetics (instantiated from template)
		std::function<Array<double, 2>(Array<double, 2>)> antithetic = normal_antithetic < Array<double, 2> > ;
		MCGatherer<double> mcgatherer_antithetic;
		unsigned long n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);

		// Geometric average option price by Monte Carlo 

		std::shared_ptr<BlackScholesAssetAdapter> gasset = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal);
		gasset->SetDivYield(0);

		// Fixed strike
		exotics::DiscreteGeometricMeanFixedStrike geo(gasset, T, call_strike, ts, 1, stockVal->GetTradePrice());
		double CFgeo = geo.price();
		double CFgeoFixedStrike = CFgeo; // used as control variate for arithmetic average option below
		cout << "Closed form value of geometric average option: " << CFgeo << endl;
		std::shared_ptr<MCPayoffList> geopayoff = geo.get_payoff();
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > g_underlying;
		g_underlying.push_back(gasset);
		GeometricBrownianMotion ggbm(g_underlying);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container_g(randomWrapper, ggbm.factors(), ggbm.number_of_steps());

		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_g(*geopayoff, ggbm, ts, numeraire_index);
		
		// Arithmetic average option price by Monte Carlo - including using geometric mean as control variate
		MCDiscreteArithmeticMeanFixedStrike avgpayoff(0, T, call_strike, 1, stockVal->GetTradePrice());
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_avg(avgpayoff, ggbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func_g = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping_avg, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg(func_g, random_container_g);
		mcgatherer.reset();
		// Build objects for antithetic simulation
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_antithetic(func_g, random_container_g, antithetic);
		mcgatherer_antithetic.reset();
		// Build objects for control variate simulation
		Array<double, 1> cv_values(1);
		cv_values = CFgeoFixedStrike;
		MCControlVariateMapping<GeometricBrownianMotion, GeometricBrownianMotion, Array<double, 2> > mc_cvmapping(mc_mapping_avg, mc_mapping_g, cv_values);
		MCGatherer<double> mcgathererCV;
		MCGatherer<double> mcgathererCV_antithetic;
		func_g = std::bind(&MCControlVariateMapping<GeometricBrownianMotion, GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_cvmapping, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_cv(func_g, random_container_g);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_cv_antithetic(func_g, random_container_g, antithetic);
		n = minpaths;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,Antithetic MC value,95% CI lower bound,95% CI upper bound,CI width,Antithetic CI width,";
		cout << "CV MC value,95% CI lower bound,95% CI upper bound,Antithetic CV MC value,95% CI lower bound,95% CI upper bound,CI width,Antithetic CI width\n";
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mcavg.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << ',';
			mcavg_antithetic.simulate(mcgatherer_antithetic, n / 2);
			cout << mcgatherer_antithetic.mean() << ',' << mcgatherer_antithetic.mean() - d*mcgatherer_antithetic.stddev() << ',' << mcgatherer_antithetic.mean() + d*mcgatherer_antithetic.stddev() << ',';
			cout << 2.0*d*mcgatherer.stddev() << ',' << 2.0*d*mcgatherer_antithetic.stddev() << ',' << std::flush;
			mcavg_cv.simulate(mcgathererCV, n);
			cout << mcgathererCV.mean() << ',' << mcgathererCV.mean() - d*mcgathererCV.stddev() << ',' << mcgathererCV.mean() + d*mcgathererCV.stddev() << ',';
			mcavg_cv_antithetic.simulate(mcgathererCV_antithetic, n / 2);
			cout << mcgathererCV_antithetic.mean() << ',' << mcgathererCV_antithetic.mean() - d*mcgathererCV_antithetic.stddev() << ',' << mcgathererCV_antithetic.mean() + d*mcgathererCV_antithetic.stddev() << ',';
			cout << 2.0*d*mcgathererCV.stddev() << ',' << 2.0*d*mcgathererCV_antithetic.stddev() << ',' << endl;
			n = mcgatherer.number_of_simulations();
		}

		// using MCGatherer<Array<double,1> > specialisation
		MCGatherer<Array<double, 1> > cvgatherer(2);
		MCPayoffList cvlist;
		cvgatherer.set_control_variate(true);
		std::shared_ptr<MCDiscreteArithmeticMeanFixedStrike> pavgpayoff(new MCDiscreteArithmeticMeanFixedStrike(0, T, call_strike, 1, stockVal->GetTradePrice()));
		cvlist.push_back(pavgpayoff);
		cvlist.push_back(geopayoff);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_cv(cvlist, ggbm, ts, numeraire_index);
		std::function<Array<double, 1>(Array<double, 2>)> func_cv = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_cv, std::placeholders::_1);

		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mccv(func_cv, random_container_g);
		n = minpaths;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,";
		cout << "CV MC value,95% CI lower bound,95% CI upper bound,Optimal CV weight\n";
		while (cvgatherer.number_of_simulations() < maxpaths)
		{
			mccv.simulate(cvgatherer, n);
			cout << cvgatherer.number_of_simulations() << ',' << cvgatherer.mean(0) << ',' << cvgatherer.mean(0) - d*cvgatherer.stddev(0) << ',' << cvgatherer.mean(0) + d*cvgatherer.stddev(0) << ',';
			cout << cvgatherer.CVestimate(0, 1, CFgeoFixedStrike) << ',' << cvgatherer.CVestimate(0, 1, CFgeoFixedStrike) - d*cvgatherer.CVestimate_stddev(0, 1) << ',' << cvgatherer.CVestimate(0, 1, CFgeoFixedStrike) + d*cvgatherer.CVestimate_stddev(0, 1) << ',';
			cout << cvgatherer.CVweight(0, 1) << endl;
			n = cvgatherer.number_of_simulations();
		}

		// using MCGatherer<Array<double,1> > specialisation with two control variates
		MCGatherer<Array<double, 1> > cvgatherer2(3);
		cvgatherer2.set_control_variate(true);
		std::shared_ptr<MCEuropeanCall> callpayoff(new MCEuropeanCall(T(0), T(T.extent(firstDim) - 1), 0, call_strike));
		std::cerr << "European call parameters: " << T(0) << ',' << T(T.extent(firstDim) - 1) << ',' << call_strike << endl;
		cvlist.push_back(callpayoff);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_cv2(cvlist, ggbm, ts, numeraire_index);
		func_cv = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_cv2, std::placeholders::_1);
		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mccv2(func_cv, random_container_g);
		Array<int, 1> CVidx(2);
		CVidx = 1, 2;
		Array<double, 1> CV_expectation(2);
		Array<double, 2> CV_weights(2, 1);
		CV_expectation = CFgeoFixedStrike, gasset->option(T(T.extent(firstDim) - 1) - T(0), call_strike, r);
		std::cerr << "European call parameters: " << T(T.extent(firstDim) - 1) - T(0) << ',' << call_strike << endl;
		n = minpaths;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,";
		cout << "CV MC value,95% CI lower bound,95% CI upper bound,Optimal CV weight,";
		cout << "2 CV MC value,Optimal CV weight 1,Optimal CV weight 2\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << cvgatherer2.mean(0) << ',' << cvgatherer2.mean(0) - d*cvgatherer2.stddev(0) << ',' << cvgatherer2.mean(0) + d*cvgatherer2.stddev(0) << ',';
			cout << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) << ',' << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) - d*cvgatherer2.CVestimate_stddev(0, 1) << ',' << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) + d*cvgatherer2.CVestimate_stddev(0, 1) << ',';
			cout << cvgatherer2.CVweight(0, 1) << ',';
			cout << cvgatherer2.CVestimate(0, CVidx, CV_expectation) << ',';
			CV_weights = cvgatherer2.CVweight(0, CVidx);
			cout << CV_weights(0, 0) << ',' << CV_weights(1, 0) << endl;
			n = cvgatherer2.number_of_simulations();
		}
		cvgatherer2.fix_weights(0, CVidx, CV_expectation);
		n = minpaths;
		cout << "Paths,Fixed weight CV MC value,95% CI lower bound,95% CI upper bound\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << cvgatherer2.mean(0) << ',' << cvgatherer2.mean(0) - d*cvgatherer2.stddev(0) << ',' << cvgatherer2.mean(0) + d*cvgatherer2.stddev(0) << endl;
			n = cvgatherer2.number_of_simulations();
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
};

TEST_F(MonteCarloTest, AntitheticExample)
{
	std::cout << "================================================================" << std::endl;
	std::cout << " Test and demonstration for Monte Carlo simulation." << std::endl;

	try
	{
		std::string symbol("AAPL");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0.03);
		int N = 100;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;

		/// obtain Black Scholes option pricing
		Array<double, 1> sgm1(1);
		sgm1 = sgm;
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm1);
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);

		// --------------- closed form option price ---------------
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << mat << endl;
		double CFcall = stock->option(mat, K, r);
		cout << "Closed form call: " << CFcall << endl;
		double CFprice = CFcall;
		FlatTermStructure ts(r, 0.0, mat + 10.0);  // flat term structure for discounting
		// --------------- European call option price by Monte Carlo ---------------
		unsigned long n = minpaths;
		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container2(randomWrapper, 1, 1); // 1 factor, 1 time step

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
		MCEuropeanCall mc_call(0, mat, 0, call_strike);
		// instantiate MCMapping and bind to functor
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping2(mc_call, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func2 = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping2, std::placeholders::_1);
		// instantiate generic Monte Carlo algorithm object
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc2(func2, random_container2);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc2_antithetic(func2, random_container2, antithetic);
		cout << "European call option\nPaths,Closed form value,MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors,";
		cout << "Antithetic MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors,CI width,Antithetic CI width\n";
		// run Monte Carlo for different numbers of simulations
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mc2.simulate(mcgatherer, n);
			// half as many paths for antithetic
			mc2_antithetic.simulate(mcgatherer_antithetic, n / 2);
			cout << mcgatherer.number_of_simulations() << ',' << CFprice << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << ',' << (mcgatherer.mean() - CFprice) / mcgatherer.stddev() << ',';
			cout << mcgatherer_antithetic.mean() << ',' << mcgatherer_antithetic.mean() - d*mcgatherer_antithetic.stddev() << ',' << mcgatherer_antithetic.mean() + d*mcgatherer_antithetic.stddev() << ',' << (mcgatherer_antithetic.mean() - CFprice) / mcgatherer_antithetic.stddev() << ',';
			cout << 2.0*d*mcgatherer.stddev() << ',' << 2.0*d*mcgatherer_antithetic.stddev() << ',' << endl;
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

TEST_F(MonteCarloTest, CVTestPower)
{
	std::cout << "================================================================" << std::endl;
	std::cout << " Test and demonstration for Monte Carlo simulation." << std::endl;

	try
	{
		std::string symbol("AAPL");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 0.5;
		K *= S;
		double call_strike = K;
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);

		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		FlatTermStructure ts(r, 0.0, mat + 10.0);
		Array<double, 1> sgm1(2);
		sgm1 = sgm;
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm1);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		
		MCGatherer<double> mcgatherer;
		// boost functor to convert random variates to their antithetics (instantiated from template)
		boost::function<Array<double, 2>(Array<double, 2>)> antithetic = normal_antithetic < Array<double, 2> > ;
		MCGatherer<double> mcgatherer_antithetic;
		unsigned long n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);

		// Power option price by Monte Carlo 
		Array<double, 1> gsgm(1);
		gsgm = sgm;
		std::shared_ptr<DeterministicAssetVol> gvol = std::make_shared<ConstVol>(gsgm);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> gasset = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, gvol);

		// Fixed strike
		double alpha = 0.1;
		call_strike = std::pow(call_strike, alpha);
		exotics::PowerOption powopt(gasset, alpha, T(0), T(T.extent(firstDim) - 1), call_strike, ts);
		double CFpow = powopt.price();
		cout << "Closed form value of power option: " << CFpow << "\nStrike: " << call_strike << "\nPower: " << alpha << endl;
		std::shared_ptr<MCPayoffList> powpayoff = powopt.get_payoff();
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > g_underlying;
		g_underlying.push_back(gasset);
		GeometricBrownianMotion ggbm(g_underlying);
		ggbm.set_timeline(T);
		cout << ggbm.number_of_steps() << endl;
		
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container_g(randomWrapper, ggbm.factors(), ggbm.number_of_steps());
		
		// Power option price by Monte Carlo - including using standard options as control variates
		// Build objects for control variate simulation
		Array<double, 1> cv_values(5), cv_strikes(5);
		cv_strikes = 0.75, 1.0, 1.25, 1.5, 2.5;
		cv_strikes *= K;
		for (int i = 0; i < 5; i++)
		{
			cv_values(i) = gasset->option(T(T.extent(firstDim) - 1) - T(0), cv_strikes(i), r);
		}
		cout << "Strikes: " << cv_strikes << endl;

		// using MCGatherer<Array<double,1> > specialisation
		MCGatherer<Array<double, 1> > cvgatherer2(6);
		MCPayoffList                        cvlist;
		cvgatherer2.set_control_variate(true);
		cvlist.push_back(powpayoff);
		for (int i = 0; i < 5; i++)
		{
			std::shared_ptr<MCEuropeanCall> callpayoff = std::make_shared<MCEuropeanCall>(T(0), T(T.extent(firstDim) - 1), 0, cv_strikes(i));
			cvlist.push_back(callpayoff);
		}
		// Test list
		Array<double, 1> test_underlying(2), test_numeraire(2);
		test_underlying = 1000.0, 1000.0;
		test_numeraire = 1.0, 1.0;
		Array<double, 1> test_strikes(cvlist.payoffArray(test_underlying, test_numeraire));
		std::cout << "Testing strikes: " << test_strikes << std::endl;
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_cv2(cvlist, ggbm, ts, numeraire_index);
		std::function<Array<double, 1>(Array<double, 2>)> func_cv = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_cv2, std::placeholders::_1);
		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mccv2(func_cv, random_container_g);
		Array<int, 1> CVidx(5);
		CVidx = 1, 2, 3, 4, 5;
		Array<double, 2> CV_weights(5, 1);
		n = minpaths;
		StringForm mystringform(12);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,";
		cout << "5 CV MC value,Optimal CV weight 1,Optimal CV weight 2\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << mystringform(cvgatherer2.mean(0)) << ',';
			cout << mystringform(cvgatherer2.mean(0) - d*cvgatherer2.stddev(0)) << ',';
			cout << mystringform(cvgatherer2.mean(0) + d*cvgatherer2.stddev(0)) << ',';
			cout << mystringform(cvgatherer2.CVestimate(0, CVidx, cv_values)) << ',';
			CV_weights = cvgatherer2.CVweight(0, CVidx);
			for (int i = 0; i < 5; i++) cout << mystringform(CV_weights(i, 0)) << ',';
			cout << endl;
			n = cvgatherer2.number_of_simulations();
		}
		cvgatherer2.fix_weights(0, CVidx, cv_values);
		n = minpaths;
		cout << "Paths,Fixed weight CV MC value,95% CI lower bound,95% CI upper bound\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << mystringform(cvgatherer2.mean(0)) << ',';
			cout << mystringform(cvgatherer2.mean(0) - d*cvgatherer2.stddev(0)) << ',';
			cout << mystringform(cvgatherer2.mean(0) + d*cvgatherer2.stddev(0)) << endl;
			n = cvgatherer2.number_of_simulations();
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

TEST_F(MonteCarloTest, LSTest)
{
	std::cout << "================================================================" << std::endl;
	std::cout << " Test and demonstration program for Longstaff/Schwartz" << std::endl;

	int i;
	try
	{
		Array<double, 2> paths(8, 4);
		paths = 1.00, 1.09, 1.08, 1.34,
			1.00, 1.16, 1.26, 1.54,
			1.00, 1.22, 1.07, 1.03,
			1.00, 0.93, 0.97, 0.92,
			1.00, 1.11, 1.56, 1.52,
			1.00, 0.76, 0.77, 0.90,
			1.00, 0.92, 0.84, 1.01,
			1.00, 0.88, 1.22, 1.34;
		Array<double, 1> T(4);
		T = 0.0, 1.0, 2.0, 3.0;
		double K = 1.10;
		Payoff put(K, -1);
		std::function<double(double)> f;
		f = std::bind(&Payoff::operator(), &put, std::placeholders::_1);
		FlatTermStructure ts(0.06, 0.0, 10.0);
		// Simplest version: one state variable, polynomial basis function
		LongstaffSchwartzExerciseBoundary1D ls(T, ts, paths, f, 2);
		/* Now do the valuation as in the example in the paper - note that properly the valuation paths should
		be independent of the paths used to estimate the exercise boundary. */
		MCGatherer<double> MCestimate;
		Array<double, 1> path(4);
		for (i = 0; i < 8; i++)
		{
			path = paths(i, Range::all());
			std::cout << "Apply: " << ls.apply(path) << ' ';
			MCestimate += ls.apply(path);
		}
		std::cout << MCestimate.mean() << std::endl;
		// Test using general version (this version would allow for multiple state variables and arbitrary basis functions)
		Array<double, 3> genpaths(8, 4, 1);
		genpaths(Range::all(), Range::all(), 0) = paths;
		Array<double, 2> numeraire_values(8, 4);
		for (i = 0; i < 4; i++) numeraire_values(Range::all(), i) = 1.0 / ts(T(i));
		std::function<double(double, const Array<double, 1>&)> payoff = std::bind(LSArrayAdapter, std::placeholders::_1, std::placeholders::_2, f, 0);
		std::vector<std::function<double(double, const Array<double, 1>&)> > basisfunctions;
		int degree = 2;
		Array<int, 1> p(1);
		for (i = 0; i <= degree; i++)
		{
			p(0) = i;
			add_polynomial_basis_function(basisfunctions, p);
		}
		LongstaffSchwartzExerciseBoundary genls(T, genpaths, numeraire_values, payoff, basisfunctions);
		MCestimate.reset();
		Array<double, 2> genpath(4, 1);
		Array<double, 1> num(4);
		for (i = 0; i < 8; i++)
		{
			genpath = genpaths(i, Range::all(), Range::all());
			num = numeraire_values(i, Range::all());
			std::cout << "Apply: " << genls.apply(genpath, num) << ' ';
			MCestimate += genls.apply(genpath, num);
		}
		std::cout << MCestimate.mean() << std::endl;
		MCestimate.reset();
		for (i = 0; i < 8; i++)
		{
			genpath = genpaths(i, Range::all(), Range::all());
			num = numeraire_values(i, Range::all()) * ts(T(3));
			std::cout << "Apply: " << genls.apply(genpath, num) << ' ';
			MCestimate += genls.apply(genpath, num);
		}
		std::cout << MCestimate.mean() << std::endl;
		// Most general version: would allow payoffs to depend on state variable history (e.g. barriers, lookbacks, average options)
		std::function<double(const Array<double, 1>&, const Array<double, 2>&)> rebpayoff = std::bind(REBAdapter, std::placeholders::_1, std::placeholders::_2, f, 0);
		std::vector<std::function<double(const Array<double, 1>&, const Array<double, 2>&)> > rebbasis_functions;
		for (i = 0; i <= degree; i++)
		{
			p(0) = i;
			add_polynomial_basis_function(rebbasis_functions, p);
		}
		RegressionExerciseBoundary reb(T, genpaths, numeraire_values, rebpayoff, rebbasis_functions);
		MCestimate.reset();
		for (i = 0; i < 8; i++)
		{
			genpath = genpaths(i, Range::all(), Range::all());
			num = numeraire_values(i, Range::all());
			std::cout << "REB apply: " << reb.apply(genpath, num) << ' ';
			MCestimate += reb.apply(genpath, num);
		}
		std::cout << MCestimate.mean() << std::endl;
		MCestimate.reset();
		for (i = 0; i < 8; i++)
		{
			genpath = genpaths(i, Range::all(), Range::all());
			num = numeraire_values(i, Range::all()) * ts(T(3));
			std::cout << "REB apply: " << reb.apply(genpath, num) << ' ';
			MCestimate += reb.apply(genpath, num);
		}
		std::cout << MCestimate.mean() << std::endl;

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

TEST_F(MonteCarloTest, LSMC)
{
	std::cout << "================================================================" << std::endl;
	std::cout << " Test and demonstrate Longstaff/Schwartz for american options." << std::endl;

	try
	{
		std::string symbol("AAPL");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);

		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 10000;
		size_t train = 1000;
		int degree = 2;
		size_t maxpaths = 10000;
		bool include_put = false;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;

		/// obtain Black Scholes option pricing
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm);
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);
		FlatTermStructure ts(r, 0.0, mat + 10.0);
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << endl;
		double CFcall = stock->option(mat, K, r);
		double CFput = stock->option(mat, K, r, -1);
		double CFiput = stock->option(T(N / 2), K, r, -1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		exotics::StandardOption Mput(stock, T(0), mat, K, ts, -1);
		cout << "Closed form put via MBinary: " << Mput.price() << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;

		std::string tmpSymbol("TAAPL");
		std::shared_ptr<IStockValue> tempStockVal = getStockValue(tmpSymbol, S*1.5, sgm, 0);

		std::shared_ptr<BlackScholesAssetAdapter> tmpstock = \
			std::make_shared<BlackScholesAssetAdapter>(tempStockVal, vol);

		double tmp_t = (T(0) + mat) / 2.0;
		exotics::StandardOption tmpMput(tmpstock, tmp_t, mat, K, ts, -1);
		cout << "Testing: " << tmpMput.price() << " == ";
		cout << Mput.price(tmp_t, S*1.5) << endl;
		GeometricBrownianMotion gbm(underlying);
		gbm.set_timeline(T);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container(randomWrapper, gbm.factors(), gbm.number_of_steps());
		MCTrainingPaths<GeometricBrownianMotion, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> >
			training_paths(gbm, T, train, random_container, ts, numeraire_index);
		Payoff put(K, -1);
		std::function<double(double)> f;
		f = std::bind(&Payoff::operator(), &put, std::placeholders::_1);
		std::function<double(double, const Array<double, 1>&)> payoff = std::bind(LSArrayAdapter, std::placeholders::_1, std::placeholders::_2, f, 0);
		std::vector<std::function<double(double, const Array<double, 1>&)> > basisfunctions;
		Array<int, 1> p(1);
		for (int i = 0; i <= degree; i++)
		{
			p(0) = i;
			add_polynomial_basis_function(basisfunctions, p);
		}

		std::function<double(double, double)> put_option;
		put_option = std::bind((static_cast<double (exotics::StandardOption::*)(double, double)>(&exotics::StandardOption::price)), &Mput, std::placeholders::_1, std::placeholders::_2);
		std::function<double(double, const Array<double, 1>&)> put_option_basis_function = std::bind(LSArrayAdapterT, std::placeholders::_1, std::placeholders::_2, put_option, 0);
		if (include_put) basisfunctions.push_back(put_option_basis_function);
		LongstaffSchwartzExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
		LSExerciseStrategy<LongstaffSchwartzExerciseBoundary> exercise_strategy(boundary);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound" << endl;
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mc.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << endl;
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

TEST_F(MonteCarloTest, LSMCMargrabe)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for American Margrabe option by Monte Carlo. " << std::endl;

	try
	{
		std::string symbol1("S1");
		std::string symbol2("S2");
		std::string tsymbol1("TS1");
		std::string tsymbol2("TS2");
		double S1 = 100.0;
		double S2 = 100.0;
		double r = 0.05;
		double sgm1 = 0.3;
		double sgm2 = 0.3;
		double rho = 0.0;
		double mat = 1.5;
		double K = 1.0;
		K *= S1 / S2;
		int N = 10;
		size_t minpaths = 100;
		size_t train = 100;
		int degree = 2;
		size_t maxpaths = 100;
		bool include_put = false;
		int numeraire_index = -1;
		double dividend1 = 0.0;
		double dividend2 = 0.0;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		Array<double, 1> sgm_1(2), sgm_2(2);
		sgm_1 = sgm1, 0.0;
		sgm_2 = rho, std::sqrt(1 - rho*rho);
		sgm_2 *= sgm2;
		std::shared_ptr<DeterministicAssetVol> vol1 = std::make_shared<ConstVol>(sgm_1);
		std::shared_ptr<DeterministicAssetVol> vol2 = std::make_shared<ConstVol>(sgm_2);

		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<IStockValue> stockVal1 = getStockValue(symbol1, S1, sgm1, dividend1);
		std::shared_ptr<BlackScholesAssetAdapter> stock1 = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal1, vol1);

		std::shared_ptr<IStockValue> stockVal2 = getStockValue(symbol2, S2, sgm2, dividend2);
		std::shared_ptr<BlackScholesAssetAdapter> stock2 = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal2, vol2);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock1);
		underlying.push_back(stock2);
		FlatTermStructure ts(r, 0.0, mat + 10.0);
		double CFcall = stock1->Margrabe(stock2, mat, K);
		cout << "Closed form price: " << CFcall << endl;
		exotics::Margrabe Mopt(stock1, stock2, T(0), mat, 1.0, K, ts);
		cout << "Closed form price via MBinary: " << Mopt.price() << endl;

		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<IStockValue> tmpstockVal1 = getStockValue(tsymbol1, S1*1.5, sgm1, dividend1);
		std::shared_ptr<BlackScholesAssetAdapter> tmpstock1 = \
			std::make_shared<BlackScholesAssetAdapter>(tmpstockVal1, vol1);

		std::shared_ptr<IStockValue> tmpstockVal2 = getStockValue(tsymbol2, S2*1.6, sgm2, dividend2);
		std::shared_ptr<BlackScholesAssetAdapter> tmpstock2 = \
			std::make_shared<BlackScholesAssetAdapter>(tmpstockVal2, vol2);

		double tmp_t = T(N / 2);
		// history as timepoints x assets
		Array<double, 2> S_history(2, N / 2 + 1);
		// in the case of the Margrabe option only the last timepoint in the history matters
		S_history(0, N / 2) = S1*1.5;
		S_history(1, N / 2) = S2*1.6;
		exotics::Margrabe tmpMopt(tmpstock1, tmpstock2, tmp_t, mat, 1.0, K, ts);
		cout << "Testing: " << tmpMopt.price() << " == ";
		cout << Mopt.price(T(Range(fromStart, N / 2)), S_history) << endl;
		cout << "Dividend 1: " << dividend1 << "   Dividend 2: " << dividend2 << endl;

		// American option by Monte Carlo
		GeometricBrownianMotion gbm(underlying);
		gbm.set_timeline(T);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNGp = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNGp);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container(randomWrapper, gbm.factors(), gbm.number_of_steps());

		MCTrainingPaths<GeometricBrownianMotion, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> >
			training_paths(gbm, T, train, random_container, ts, numeraire_index);
		cout << "Training paths created." << endl;
		// payoff requires (time points) x (state variables) Array as second argument
		std::function<double(const Array<double, 1>&, const Array<double, 2>&)> payoff = std::bind(&exotics::Margrabe::early_exercise_payoff, &Mopt, std::placeholders::_1, std::placeholders::_2);
		std::vector<std::function<double(const Array<double, 1>&, const Array<double, 2>&)> > basisfunctions;
		Array<int, 1> p(2);
		p(1) = 0.0;
		for (int i = 0; i <= degree; i++)
		{
			p(0) = i;
			add_polynomial_basis_function(basisfunctions, p);
		}
		p(0) = 0.0;
		for (int i = 1; i <= degree; i++)
		{
			p(1) = i;
			add_polynomial_basis_function(basisfunctions, p);
		}

		auto put_option = [&](const Array<double, 1>& T, const Array<double, 2>& history) -> double
		{
			return Mopt.price(T, history);
		};

		if (include_put) basisfunctions.push_back(put_option);
		cout << "Fitting exercise boundary..." << endl;
		// training_paths is currently a paths x (time points) x (state variables) Array
		RegressionExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
		cout << "Creating exercise strategy..." << endl;
		LSExerciseStrategy<RegressionExerciseBoundary> exercise_strategy(boundary);
		cout << "Setting up Monte Carlo simulation..." << endl;
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound" << endl;
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mc.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << endl;
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

TEST_F(MonteCarloTest, LSMCPath)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for American Margrabe option by Monte Carlo. " << std::endl;

	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		int N = 10;
		size_t minpaths = 100;
		size_t train = 100;
		int degree = 2;
		size_t maxpaths = 100;
		bool include_put = false;
		int numeraire_index = -1;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;

		/// obtain Black Scholes option pricing
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm);
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);

		FlatTermStructure ts(r, 0.0, mat + 10.0);
		cout << "S: " << S << "\nK: " << K << "\nr: " << r << "\nT: " << mat << "\nsgm: " << sgm << endl;
		double CFcall = stock->option(mat, K, r);
		double CFput = stock->option(mat, K, r, -1);
		double CFiput = stock->option(T(N / 2), K, r, -1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		exotics::StandardOption Mput(stock, T(0), mat, K, ts, -1);
		cout << "Closed form put via MBinary: " << Mput.price() << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;

		/// obtain Black Scholes option pricing
		std::string tsymbol("TS1");
		std::shared_ptr<IStockValue> tmpstockVal = getStockValue(tsymbol, S*1.5, sgm, 0);
		std::shared_ptr<BlackScholesAssetAdapter> tmpstock = \
			std::make_shared<BlackScholesAssetAdapter>(tmpstockVal, vol);

		double tmp_t = (T(0) + mat) / 2.0;
		exotics::StandardOption tmpMput(tmpstock, tmp_t, mat, K, ts, -1);
		cout << "Testing: " << tmpMput.price() << " == ";
		cout << Mput.price(tmp_t, S*1.5) << endl;
		GeometricBrownianMotion gbm(underlying);
		gbm.set_timeline(T);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);

		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container(randomWrapper, gbm.factors(), gbm.number_of_steps());
		MCTrainingPaths<GeometricBrownianMotion, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> >
			training_paths(gbm, T, train, random_container, ts, numeraire_index);
		Payoff put(K, -1);
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
		boost::function<double(Array<double, 2>)> func = boost::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, _1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound" << endl;
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mc.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << endl;
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

TEST_F(MonteCarloTest, MCDownAndOut)
{
	std::cout << "=================================================================================" << endl;
	std::cout << " Down and Out option pricing by MC" << endl;
	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double maturity = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 100;
		size_t maxpaths = 100;
		// set up timeline - N is the number of time steps, mat is the maturity of the option
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = maturity / N;
		T = idx*dt;
		// set up asset - volatility level is sgm on first factor only
		Array<double, 1> sgm1(2);
		sgm1 = sgm, 0.0;

		/// obtain Black Scholes option pricing
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);
		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(sgm1);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);

		cout << "S: " << S << "\nK: " << K << "\nr: " << r << "\nT: " << maturity << "\nsgm: " << sgm << endl;
		// closed-form vanilla option prices
		double CFcall = stock->option(maturity, K, r);
		double CFput = stock->option(maturity, K, r, -1);
		double CFiput = stock->option(T(N / 2), K, r, -1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;
		// initialise vector of underlying assets
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);
		// flat term structure for discounting
		FlatTermStructure ts(r, 0.0, maturity + 10.0);

		// Down and out call option price by Monte Carlo and in "closed form"
		MCGatherer<Array<double, 1> > mcgatherer(2);
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		// univariate standard normal pseudo-random number generator
		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		// collection of independent random variates required to generate a single path

		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container(randomWrapper, 2, N);  // 2 factors, N time steps
		// asset price process
		GeometricBrownianMotion gbm(underlying);
		double barrier = 0.95*K;
		exotics::DiscreteBarrierOut down_and_out_call_option(stock, T, K, barrier, ts, 1, -1);
		// Convergence of "closed form" price
		cout << "DiscreteBarrierOut closed-form formula\nPrice: " << endl;
		int i = minpaths;
		while (i <= maxpaths)
		{
			// i is the number of simulations in the evaluation of the multivariate normal CDF by quasi-random Monte Carlo
			cout << i << ',' << down_and_out_call_option.price(i) << endl;
			i *= 2;
		}
		cout << "Covariance matrix dimension: " << down_and_out_call_option.covariance_matrix().extent(firstDim);
		cout << "\nEigenvalues: " << down_and_out_call_option.eigenvalues() << endl;
		// create Monte Carlo payoff objects - one using MBinaries and the other using the down_and_out_call class defined above
		std::shared_ptr<MCPayoffList> dopayoff = down_and_out_call_option.get_payoff();
		std::shared_ptr<MCPayoff> down_and_out_call_instance(new down_and_out_call(T, 0, K, barrier));
		MCPayoffList both_options;
		both_options.push_back(down_and_out_call_instance);
		both_options.push_back(dopayoff);
		// MCMapping to map random numbers to asset price realisations to discounted payoffs
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_do(both_options, gbm, ts, numeraire_index);
		// mapping functor
		std::function<Array<double, 1>(Array<double, 2>)> func_do = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_do, std::placeholders::_1);
		// collection of independent random variates required to generate a single path
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container_do(randomWrapper, gbm.factors(), gbm.number_of_steps());
		// generic Monte Carlo algorithm object
		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcg(func_do, random_container_do);
		mcgatherer.reset();
		size_t n = minpaths;
		cout << "Paths,MC value 1,MC value 2\n";
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mcg.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << (mcgatherer.mean())(0) << ',' << (mcgatherer.mean())(1) << ',';
			cout << (mcgatherer.mean())(0) - d*(mcgatherer.stddev())(0) << ',' << (mcgatherer.mean())(0) + d*(mcgatherer.stddev())(0) << ',' << endl;
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

TEST_F(MonteCarloTest, MCGenericArrayTest)
{
	std::cout << "==================================================================" << std::endl;

	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double minacc = 1e-2;
		double maxacc = 1e-2;
		size_t minpaths = 100;
		size_t maxpaths = 100;

		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol, S, sgm, 0);
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockValuePtr, vol);

		cout << "S: " << S << "\nK: " << K << "\nr: " << r << "\nT: " \
			<< mat << "\nsgm: " << sgm << endl << endl;
		double CFcall = stock->option(mat, K, r);
		double CFput = stock->option(mat, K, r, -1);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		RandomWrapper<ranlib::NormalUnit<double>, double> randomWrapper(normalRNG);

		objective_class obj(S, mat, r, sgm, K);
		std::function<Array<double, 1>(double)> func = std::bind(&objective_class::func, &obj, std::placeholders::_1);
		std::function<double(double)> antithetic = normal_antithetic < double > ;

		MCGeneric<double, Array<double, 1>, RandomWrapper<ranlib::NormalUnit<double>, double> > mc(func, randomWrapper);
		MCGeneric<double, Array<double, 1>, RandomWrapper<ranlib::NormalUnit<double>, double> > mc_antithetic(func, randomWrapper, antithetic);
		MCGatherer<Array<double, 1> > mcgatherer(2), mcgatherer_antithetic(2);
		double acc = minacc * CFcall;
		maxacc *= CFcall;
		cout << "Aiming for maximum accuracy of " << maxacc << endl;
		cout << "Method,Option,Required accuracy,Actual accuracy,Paths,Black/Scholes value,MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors\n";
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		while (acc >= maxacc)
		{
			double curr_acc = mc.simulate(mcgatherer, 100, acc);
			Array<double, 1> mean(mcgatherer.mean());
			Array<double, 1> stddev(mcgatherer.stddev());
			cout << "Standard,Call," << acc << ',' << curr_acc << ',' << mcgatherer.number_of_simulations() << ',' << CFcall << ',' << mean(0) << ',' << mean(0) - d*stddev(0) << ',' << mean(0) + d*stddev(0) << ',' << (mean(0) - CFcall) / stddev(0) << endl;
			cout << "Standard,Put," << acc << ',' << curr_acc << ',' << mcgatherer.number_of_simulations() << ',' << CFput << ',' << mean(1) << ',' << mean(1) - d*stddev(1) << ',' << mean(1) + d*stddev(1) << ',' << (mean(1) - CFput) / stddev(1) << endl;
			double curr_acc_antithetic = mc_antithetic.simulate(mcgatherer_antithetic, 100, acc);
			Array<double, 1> mean_antithetic(mcgatherer_antithetic.mean());
			Array<double, 1> stddev_antithetic(mcgatherer_antithetic.stddev());
			cout << "Antithetic,Call," << acc << ',' << curr_acc_antithetic << ',' << mcgatherer_antithetic.number_of_simulations() << ',' << CFcall << ',' << mean_antithetic(0) << ',' << mean_antithetic(0) - d*stddev_antithetic(0) << ',' << mean_antithetic(0) + d*stddev_antithetic(0) << ',' << (mean_antithetic(0) - CFcall) / stddev_antithetic(0) << endl;
			cout << "Antithetic,Put," << acc << ',' << curr_acc_antithetic << ',' << mcgatherer_antithetic.number_of_simulations() << ',' << CFput << ',' << mean_antithetic(1) << ',' << mean_antithetic(1) - d*stddev_antithetic(1) << ',' << mean_antithetic(1) + d*stddev_antithetic(1) << ',' << (mean_antithetic(1) - CFput) / stddev_antithetic(1) << endl;
			acc /= 2;
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

TEST_F(MonteCarloTest, MCMargrabe)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for quasirandom Monte Carlo. " << std::endl;

	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		// set up timeline - N is the number of time steps, mat is the maturity of the option
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		// set up first asset - volatility level is sgm on both factors
		Array<double, 1> sgm1(2);
		sgm1 = sgm;
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm1);
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0.03);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);
		// vector containing underlying assets
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);
		// term structure of interest rates
		FlatTermStructure ts(r, 0.0, mat + 10.0);
		// set up second asset
		Array<double, 1> sgm2(2);
		sgm2 = 0.1, 0.4;

		std::string symbol2("S2");
		std::shared_ptr<DeterministicAssetVol> vol2 = std::make_shared<ConstVol>(sgm2);
		std::shared_ptr<IStockValue> stockVal2 = getStockValue(symbol2, S, sgm, 0.04);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock2 = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal2, vol2);

		underlying.push_back(stock2);
		// closed form price via formula implemented as member function of BlackScholesAsset - K/S is strike factor
		double margrabe_price = stock->Margrabe(stock2, mat, K / S);
		cout << "Closed form Margrabe option: " << margrabe_price << endl;
		// set up MBinaries
		Array<int, 2> xS(1, 1);
		xS = 1;
		Array<int, 2> mindex(2, 2);
		mindex = 0, 1,
			N, N;
		Array<double, 1> malpha(2);
		malpha = 1.0, 0.0;
		Array<double, 2> mA(1, 2);
		mA = 1.0, -1.0;
		Array<double, 1> ma(1);
		ma = K / S;
		MBinary M1(underlying, ts, T, mindex, malpha, xS, mA, ma);
		double M1price = M1.price();
		Array<double, 1> malpha2(2);
		malpha2 = 0.0, 1.0;
		MBinary M2(underlying, ts, T, mindex, malpha2, xS, mA, ma);
		cout << "MBinary price of Margrabe option: " << M1price - K / S*M2.price() << std::endl;
		exotics::Margrabe margrabe(stock, stock2, 0.0, mat, 1.0, K / S, ts);
		cout << "Price of Margrabe option via MExotics: " << margrabe.price() << std::endl;

		//---------------- Margrabe option price by Monte Carlo -------------
		unsigned long n = minpaths;
		// asset price process
		GeometricBrownianMotion gbm(underlying);
		gbm.set_timeline(T);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container(randomWrapper, gbm.factors(), gbm.number_of_steps());

		// create MCPayoffList from MBinaries
		std::shared_ptr<MBinaryPayoff> M1payoff = M1.get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2.get_payoff();
		MCPayoffList mcpayofflist;
		mcpayofflist.push_back(M1payoff);
		mcpayofflist.push_back(M2payoff, -K / S);
		cout << "Strike coefficient: " << K / S << endl;
		// MCMapping to map random numbers to asset price realisations to discounted payoffs
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(mcpayofflist, gbm, ts, numeraire_index);
		// mapping functor
		boost::function<double(Array<double, 2>)> func = boost::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, _1);
		// generic Monte Carlo algorithm object
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc_pseudo(func, random_container);
		MCGatherer<double> mcgathererpseudo;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);

		// run pseudo-random Monte Carlo simulation and compare to closed form value
		cout << "Minimum number of paths: " << minpaths << "\nMaximum number of paths: " << maxpaths << endl;
		cout << "Paths,Closed-form value,MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors\n";
		double CFcall = margrabe.price();
		while (n <= maxpaths)
		{
			// simulate
			mc_pseudo.simulate(mcgathererpseudo, n);
			// output Monte Carlo result
			cout << mcgathererpseudo.number_of_simulations() << ',' << CFcall << ',' << mcgathererpseudo.mean() << ',' << mcgathererpseudo.mean() - d*mcgathererpseudo.stddev() << ',' << mcgathererpseudo.mean() + d*mcgathererpseudo.stddev() << ',' << (mcgathererpseudo.mean() - CFcall) / mcgathererpseudo.stddev() << endl;
			n = mcgathererpseudo.number_of_simulations();
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

TEST_F(MonteCarloTest, QRMargrabeExampleReduced)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for quasirandom Monte Carlo. " << std::endl;

	try
	{
		std::string symbol("AAPL");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		// set up timeline - N is the number of time steps, mat is the maturity of the option
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		// set up first asset - volatility level is sgm on both factors
		Array<double, 1> sgm1(2);
		sgm1 = sgm;

		/// BlackScholes object for google stock
		std::shared_ptr<IStockValue> stockVal = getStockValue(symbol, S, sgm, 0);
		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(sgm1);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, vol);

		// initialise vector of underlying assets
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);

		// term structure of interest rates
		FlatTermStructure ts(r, 0.0, mat + 10.0);
		// set up second asset
		Array<double, 1> sgm2(2);
		sgm2 = 0.1, 0.4;

		/// BlackScholes object for google stock
		std::shared_ptr<DeterministicAssetVol>  vol2 = std::make_shared<ConstVol>(sgm2);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock2 = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal, std::move(vol2));
		underlying.push_back(stock2);
		// closed form price via formula implemented as member function of BlackScholesAsset - K/S is strike factor
		double margrabe_price = stock->Margrabe(stock2, mat, K / (stockVal->GetTradePrice()));
		cout << "Closed form Margrabe option: " << margrabe_price << endl;
		// set up MBinaries
		Array<int, 2> xS(1, 1);
		xS = 1;
		Array<int, 2> mindex(2, 2);
		mindex = 0, 1,
			N, N;
		Array<double, 1> malpha(2);
		malpha = 1.0, 0.0;
		Array<double, 2> mA(1, 2);
		mA = 1.0, -1.0;
		Array<double, 1> ma(1);
		ma = K / (stockVal->GetTradePrice());
		MBinary M1(underlying, ts, T, mindex, malpha, xS, mA, ma);
		double M1price = M1.price();
		Array<double, 1> malpha2(2);
		malpha2 = 0.0, 1.0;
		MBinary M2(underlying, ts, T, mindex, malpha2, xS, mA, ma);
		cout << "MBinary price of Margrabe option: " << M1price - K / (stockVal->GetTradePrice())*M2.price() << std::endl;
		exotics::Margrabe margrabe(stock, stock2, 0.0, mat, 1.0, K / (stockVal->GetTradePrice()), ts);
		cout << "Price of Margrabe option via MExotics: " << margrabe.price() << std::endl;

		//---------------- Margrabe option price by quasi-random Monte Carlo -------------
		// quasi-random number generator
		unsigned long n = minpaths;
		SobolArrayNormal sobol(2, N, maxpaths);
		// asset price process
		GeometricBrownianMotion gbm(underlying);
		// create MCPayoffList from MBinaries
		std::shared_ptr<MBinaryPayoff> M1payoff = M1.get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2.get_payoff();
		MCPayoffList mcpayofflist;
		mcpayofflist.push_back(M1payoff);
		mcpayofflist.push_back(M2payoff, -K / (stockVal->GetTradePrice()));
		cout << "Strike coefficient: " << K / (stockVal->GetTradePrice()) << endl;
		// MCMapping to map random numbers to asset price realisations to discounted payoffs
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(mcpayofflist, gbm, ts, numeraire_index);
		// mapping functor
		std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
		// generic Monte Carlo algorithm object
		MCGeneric<Array<double, 2>, double, SobolArrayNormal> mc_QR(func, sobol);
		MCGatherer<double> mcgathererQR;

		// run quasi-random Monte Carlo simulation and compare to closed form value
		cout << "Minimum number of paths: " << minpaths << "\nMaximum number of paths: " << maxpaths << endl;
		cout << "Margrabe option\nPaths,Closed form value,";
		cout << "QR MC value\n";
		std::cerr << "Margrabe option\nPaths,Closed form value,";
		std::cerr << "QR MC value\n";
		double CFprice = margrabe.price();
		while (n <= maxpaths)
		{
			cout << n << ',' << CFprice << ',';
			std::cerr << n << ',' << CFprice << ',';
			// simulate
			mc_QR.simulate(mcgathererQR, n);
			// output Monte Carlo result
			cout << mcgathererQR.mean() << ',' << endl;
			std::cerr << mcgathererQR.mean() << ',' << endl;
			n = mcgathererQR.number_of_simulations();
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

TEST_F(MonteCarloTest, QRMargrabeExample)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for quasirandom Monte Carlo. " << std::endl;

	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 40;
		size_t maxpaths = 40;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		Array<double, 1> sgm1(2);
		sgm1 = sgm;

		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm1);
		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol, S, sgm, 0.03);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockValuePtr, vol);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);

		FlatTermStructure ts(r, 0.0, mat + 10.0);
		Array<int, 2> xS(1, 1);
		xS = 1;
		Array<double, 1> sgm2(2);
		sgm2 = 0.1, 0.4;

		/// BlackScholes object for google stock
		std::shared_ptr<DeterministicAssetVol>  vol2 = std::make_shared<ConstVol>(sgm2);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock2 = \
			std::make_shared<BlackScholesAssetAdapter>(stockValuePtr, vol2);

		K = 1.0;
		double margrabe_price = stock->Margrabe(stock2, mat, K);
		cout << "Closed form Margrabe option: " << margrabe_price << endl;
		underlying.push_back(stock2);
		Array<int, 2> mindex(2, 2);
		mindex = 0, 1,
			N, N;
		Array<double, 1> malpha(2);
		malpha = 1.0, 0.0;
		Array<double, 2> mA(1, 2);
		mA = 1.0, -1.0;
		Array<double, 1> ma(1);
		ma = K;
		MBinary M1(underlying, ts, T, mindex, malpha, xS, mA, ma);
		double M1price = M1.price(1000000000);
		Array<double, 1> malpha2(2);
		malpha2 = 0.0, 1.0;
		MBinary M2(underlying, ts, T, mindex, malpha2, xS, mA, ma);
		cout << "MBinary price of Margrabe option: " << M1price - K*M2.price(1000000000) << std::endl;
		exotics::Margrabe margrabe(stock, stock2, 0.0, mat, 1.0, K, ts);
		cout << "Price of Margrabe option via MExotics: " << margrabe.price() << std::endl;

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);
		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container2(randomWrapper, 2, N);  // 2 factors, N time steps

		SobolArrayNormal sobol(2, N, maxpaths * 2);
		GeometricBrownianMotion gbm(underlying);
		std::shared_ptr<MBinaryPayoff> M1payoff = M1.get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2.get_payoff();
		MCPayoffList mcpayofflist;
		mcpayofflist.push_back(M1payoff);
		mcpayofflist.push_back(M2payoff, -K);
		cout << "Strike coefficient: " << K << endl;
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(mcpayofflist, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container2);
		MCGatherer<double> mcgatherer;
		// std functor to convert random variates to their antithetics (instantiated from template)
		std::function<Array<double, 2>(Array<double, 2>)> antithetic = normal_antithetic < Array<double, 2> >;
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mc_antithetic(func, random_container2, antithetic);
		MCGatherer<double> mcgatherer_antithetic;
		MCGeneric<Array<double, 2>, double, SobolArrayNormal> mc_QR(func, sobol);
		MCGatherer<double> mcgathererQR;
		// For nested construction of randomised QMC using random shift
		MCGatherer<double> mcgathererQRran;

		// instantiate random number generator
		std::shared_ptr<ranlib::Uniform<double> > ugen = std::make_shared<ranlib::Uniform<double> >();
		std::shared_ptr<RandomWrapper<ranlib::Uniform<double>, double> > unigenWrapper = \
			std::make_shared<RandomWrapper<ranlib::Uniform<double>, double> >(ugen);
		RandomArray<RandomWrapper<ranlib::Uniform<double>, double>, double> unigen(unigenWrapper, 2, N);

		unsigned long n = minpaths;
		cout << "Minimum number of paths: " << minpaths << "\nMaximum number of paths: " << maxpaths << endl;
		cout << "Margrabe option\nPaths,Closed form value,MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors,";
		cout << "Antithetic MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors,CI width,Antithetic CI width,";
		cout << "QR MC value,randomised QR MC value,95% CI lower bound,95% CI upper bound,Difference in standard errors,CI width\n";
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);
		double CFprice = margrabe.price();
		while (mcgatherer.number_of_simulations() < maxpaths)
		{
			mc.simulate(mcgatherer, n);
			cout << mcgatherer.number_of_simulations() << ',' << CFprice << ',' << mcgatherer.mean() << ',' << mcgatherer.mean() - d*mcgatherer.stddev() << ',' << mcgatherer.mean() + d*mcgatherer.stddev() << ',' << (mcgatherer.mean() - CFprice) / mcgatherer.stddev() << ',' << std::flush;
			mc_antithetic.simulate(mcgatherer_antithetic, n / 2);
			cout << mcgatherer_antithetic.mean() << ',' << mcgatherer_antithetic.mean() - d*mcgatherer_antithetic.stddev() << ',' << mcgatherer_antithetic.mean() + d*mcgatherer_antithetic.stddev() << ',' << (mcgatherer_antithetic.mean() - CFprice) / mcgatherer_antithetic.stddev() << ',' << std::flush;
			cout << 2.0*d*mcgatherer.stddev() << ',' << 2.0*d*mcgatherer_antithetic.stddev() << ',';
			mc_QR.simulate(mcgathererQR, n - 1);
			cout << mcgathererQR.mean() << ',';
			// Nested construction of randomised QMC using random shift
			SobolArrayNormal sobolr(2, N, n / 32);
			RandomShiftQMCMapping<GeometricBrownianMotion, Array<double, 2>, SobolArrayNormal> QR_mapping(sobolr, mcpayofflist, gbm, ts, numeraire_index);
			std::function<double(Array<double, 2>)> QRfunc = std::bind(&RandomShiftQMCMapping<GeometricBrownianMotion, Array<double, 2>, SobolArrayNormal>::mapping, &QR_mapping, std::placeholders::_1);
			MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::Uniform<double>, double>, double> > randomQMC(QRfunc, unigen);
			mcgathererQRran.reset();
			randomQMC.simulate(mcgathererQRran, 32);
			cout << mcgathererQRran.mean() << ',' << mcgathererQRran.mean() - d*mcgathererQRran.stddev() << ',' << mcgathererQRran.mean() + d*mcgathererQRran.stddev() << ',' << (mcgathererQRran.mean() - CFprice) / mcgathererQRran.stddev() << ',' << 2.0*d*mcgathererQRran.stddev() << endl;
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

TEST_F(MonteCarloTest, QRCVTest)
{
	std::cout << "==================================================================" << std::endl;
	std::cout << "Test and demonstration for quasi-random Monte Carlo with control variates.. " << std::endl;

	try
	{
		std::string symbol("S1");
		double S = 100.0;
		double r = 0.05;
		double sgm = 0.3;
		double mat = 1.5;
		double K = 1.0;
		K *= S;
		double call_strike = K;
		int N = 10;
		int numeraire_index = -1;
		size_t minpaths = 1024;
		size_t maxpaths = 1024;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		Array<double, 1> sgm1(2);
		sgm1 = sgm;
		std::shared_ptr<DeterministicAssetVol> vol = std::make_shared<ConstVol>(sgm1);

		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol, S, sgm, 0.03);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockValuePtr, vol);

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);

		cout << "S: " << S << "\nK: " << K << "\nr: " << r \
			<< "\nT: " << mat << endl;
		double CFcall = stock->option(mat, K, r);
		double CFput = stock->option(mat, K, r, -1);
		double CFiput = stock->option(T(N / 2), K, r, -1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;

		FlatTermStructure ts(r, 0.0, mat + 10.0);
		MCGatherer<double> mcgatherer;
		// std functor to convert random variates to their antithetics (instantiated from template)
		std::function<Array<double, 2>(Array<double, 2>)> antithetic = normal_antithetic < Array<double, 2> >;
		MCGatherer<double> mcgatherer_antithetic;
		unsigned long n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal, 0.95);

		// Geometric average option price by Monte Carlo
		Array<double, 1> gsgm(1);
		gsgm = sgm;

		std::shared_ptr<DeterministicAssetVol>  gvol = std::make_shared<ConstVol>(gsgm);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> gasset = \
			std::make_shared<BlackScholesAssetAdapter>(stockValuePtr, std::move(gvol));

		// Fixed strike
		exotics::DiscreteGeometricMeanFixedStrike geo(gasset, T, call_strike, ts, 1, S);
		double CFgeo = geo.price();
		double CFgeoFixedStrike = CFgeo; // used as control variate for arithmetic average option below
		cout << "Closed form value of geometric average option: " << CFgeo << endl;
		std::shared_ptr<MCPayoffList> geopayoff = geo.get_payoff();

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > g_underlying;
		g_underlying.push_back(gasset);
		GeometricBrownianMotion ggbm(g_underlying);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_g(*geopayoff, ggbm, ts, numeraire_index);

		// instantiate random number generator
		std::shared_ptr<ranlib::NormalUnit<double> > normalRNG = std::make_shared<ranlib::NormalUnit<double> >();
		std::shared_ptr<RandomWrapper<ranlib::NormalUnit<double>, double> > randomWrapper = \
			std::make_shared<RandomWrapper<ranlib::NormalUnit<double>, double> >(normalRNG);

		RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> random_container_g(randomWrapper, ggbm.factors(), ggbm.number_of_steps());

		// Arithmetic average option price by Monte Carlo - including using geometric mean as control variate
		MCDiscreteArithmeticMeanFixedStrike avgpayoff(0, T, call_strike, 1, S);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_avg(avgpayoff, ggbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func_g = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping_avg, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg(func_g, random_container_g);
		mcgatherer.reset();
		// Build objects for antithetic simulation
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_antithetic(func_g, random_container_g, antithetic);
		mcgatherer_antithetic.reset();
		// Build objects for control variate simulation
		Array<double, 1> cv_values(1);
		cv_values = CFgeoFixedStrike;
		MCControlVariateMapping<GeometricBrownianMotion, GeometricBrownianMotion, Array<double, 2> > mc_cvmapping(mc_mapping_avg, mc_mapping_g, cv_values);
		MCGatherer<double> mcgathererCV;
		MCGatherer<double> mcgathererCV_antithetic;
		func_g = std::bind(&MCControlVariateMapping<GeometricBrownianMotion, GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_cvmapping, std::placeholders::_1);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_cv(func_g, random_container_g);
		MCGeneric<Array<double, 2>, double, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mcavg_cv_antithetic(func_g, random_container_g, antithetic);

		// using MCGatherer<Array<double,1> > specialisation
		MCGatherer<Array<double, 1> > cvgatherer(2);
		MCPayoffList                        cvlist;
		cvgatherer.set_control_variate(true);
		std::shared_ptr<MCDiscreteArithmeticMeanFixedStrike> pavgpayoff(new MCDiscreteArithmeticMeanFixedStrike(0, T, call_strike, 1, S));
		cvlist.push_back(pavgpayoff);
		cvlist.push_back(geopayoff);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_cv(cvlist, ggbm, ts, numeraire_index);
		std::function<Array<double, 1>(Array<double, 2>)> func_cv = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_cv, std::placeholders::_1);
		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mccv(func_cv, random_container_g);
		// For nested construction of randomised QMC using random shift
		MCGatherer<Array<double, 1> > mcgathererQRran(2);
		mcgathererQRran.set_control_variate(true);

		// instantiate random number generator
		std::shared_ptr<ranlib::Uniform<double> > ugen = std::make_shared<ranlib::Uniform<double> >();
		std::shared_ptr<RandomWrapper<ranlib::Uniform<double>, double> > ugenWrapper = \
			std::make_shared<RandomWrapper<ranlib::Uniform<double>, double> >(ugen);
		RandomArray<RandomWrapper<ranlib::Uniform<double>, double>, double> unigen(ugenWrapper, ggbm.factors(), ggbm.number_of_steps());

		n = minpaths;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,";
		cout << "CV MC value,95% CI lower bound,95% CI upper bound,Optimal CV weight,";
		cout << "randomised QR MC value,95% CI lower bound,95% CI upper bound,";
		cout << "CV randomised QR MC value,95% CI lower bound,95% CI upper bound,Optimal CV weight\n";
		StringForm mystringform(12);
		while (cvgatherer.number_of_simulations() < maxpaths)
		{
			mccv.simulate(cvgatherer, n);
			cout << cvgatherer.number_of_simulations() << ',' << mystringform(cvgatherer.mean(0)) << ',';
			cout << mystringform(cvgatherer.mean(0) - d*cvgatherer.stddev(0)) << ',' << mystringform(cvgatherer.mean(0) + d*cvgatherer.stddev(0)) << ',';
			cout << mystringform(cvgatherer.CVestimate(0, 1, CFgeoFixedStrike)) << ',';
			cout << mystringform(cvgatherer.CVestimate(0, 1, CFgeoFixedStrike) - d*cvgatherer.CVestimate_stddev(0, 1)) << ',';
			cout << mystringform(cvgatherer.CVestimate(0, 1, CFgeoFixedStrike) + d*cvgatherer.CVestimate_stddev(0, 1)) << ',';
			cout << mystringform(cvgatherer.CVweight(0, 1)) << ',';
			// Nested construction of randomised QMC using random shift
			SobolArrayNormal sobolr(ggbm.factors(), ggbm.number_of_steps(), n / 64);
			RandomShiftQMCMapping<GeometricBrownianMotion, Array<double, 2>, SobolArrayNormal> QR_mapping(sobolr, cvlist, ggbm, ts, numeraire_index);
			std::function<Array<double, 1>(Array<double, 2>)> QRfunc = std::bind(&RandomShiftQMCMapping<GeometricBrownianMotion, Array<double, 2>, SobolArrayNormal>::mappingArray, &QR_mapping, std::placeholders::_1);
			MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::Uniform<double>, double>, double> > randomQMC(QRfunc, unigen);
			mcgathererQRran.reset();
			randomQMC.simulate(mcgathererQRran, 64);
			cout << mystringform(mcgathererQRran.mean(0)) << ',';
			cout << mystringform(mcgathererQRran.mean(0) - d*mcgathererQRran.stddev(0)) << ',';
			cout << mystringform(mcgathererQRran.mean(0) + d*mcgathererQRran.stddev(0)) << ',';
			cout << mystringform(mcgathererQRran.CVestimate(0, 1, CFgeoFixedStrike)) << ',';
			cout << mystringform(mcgathererQRran.CVestimate(0, 1, CFgeoFixedStrike) - d*mcgathererQRran.CVestimate_stddev(0, 1)) << ',';
			cout << mystringform(mcgathererQRran.CVestimate(0, 1, CFgeoFixedStrike) + d*mcgathererQRran.CVestimate_stddev(0, 1)) << ',';
			cout << mystringform(mcgathererQRran.CVweight(0, 1)) << endl;
			n = cvgatherer.number_of_simulations();
		}

		// using MCGatherer<Array<double,1> > specialisation with two control variates
		MCGatherer<Array<double, 1> > cvgatherer2(3);
		cvgatherer2.set_control_variate(true);
		std::shared_ptr<MCEuropeanCall> callpayoff(new MCEuropeanCall(T(0), T(T.extent(firstDim) - 1), 0, call_strike));
		std::cerr << "European call parameters: " << T(0) << ',' << T(T.extent(firstDim) - 1) << ',' << call_strike << endl;
		cvlist.push_back(callpayoff);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping_cv2(cvlist, ggbm, ts, numeraire_index);
		func_cv = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mappingArray, &mc_mapping_cv2, std::placeholders::_1);
		MCGeneric<Array<double, 2>, Array<double, 1>, RandomArray<RandomWrapper<ranlib::NormalUnit<double>, double>, double> > mccv2(func_cv, random_container_g);
		Array<int, 1> CVidx(2);
		CVidx = 1, 2;
		Array<double, 1> CV_expectation(2);
		Array<double, 2> CV_weights(2, 1);
		CV_expectation = CFgeoFixedStrike, gasset->option(T(T.extent(firstDim) - 1) - T(0), call_strike, r);
		std::cerr << "European call parameters: " << T(T.extent(firstDim) - 1) - T(0) << ',' << call_strike << endl;
		n = minpaths;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,";
		cout << "CV MC value,95% CI lower bound,95% CI upper bound,Optimal CV weight,";
		cout << "2 CV MC value,Optimal CV weight 1,Optimal CV weight 2\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << cvgatherer2.mean(0) << ',' << cvgatherer2.mean(0) - d*cvgatherer2.stddev(0) << ',' << cvgatherer2.mean(0) + d*cvgatherer2.stddev(0) << ',';
			cout << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) << ',' << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) - d*cvgatherer2.CVestimate_stddev(0, 1) << ',' << cvgatherer2.CVestimate(0, 1, CFgeoFixedStrike) + d*cvgatherer2.CVestimate_stddev(0, 1) << ',';
			cout << cvgatherer2.CVweight(0, 1) << ',';
			cout << cvgatherer2.CVestimate(0, CVidx, CV_expectation) << ',';
			CV_weights = cvgatherer2.CVweight(0, CVidx);
			cout << CV_weights(0, 0) << ',' << CV_weights(1, 0) << endl;
			n = cvgatherer2.number_of_simulations();
		}
		cvgatherer2.fix_weights(0, CVidx, CV_expectation);
		n = minpaths;
		cout << "Paths,Fixed weight CV MC value,95% CI lower bound,95% CI upper bound\n";
		while (cvgatherer2.number_of_simulations() < maxpaths)
		{
			mccv2.simulate(cvgatherer2, n);
			cout << cvgatherer2.number_of_simulations() << ',' << cvgatherer2.mean(0) << ',' << cvgatherer2.mean(0) - d*cvgatherer2.stddev(0) << ',' << cvgatherer2.mean(0) + d*cvgatherer2.stddev(0) << endl;
			n = cvgatherer2.number_of_simulations();
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

int main(int argc, char **argv)
{
	// change the current working directory
	std::cout << boost::filesystem::current_path() << std::endl;
	_chdir("..\\..\\common\\CSVInputs");
	std::cout << boost::filesystem::current_path() << std::endl;

	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}

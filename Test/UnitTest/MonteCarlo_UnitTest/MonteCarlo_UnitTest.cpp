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
		size_t minpaths = 10000000;
		size_t maxpaths = 10000000;
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
		std::shared_ptr<GeometricBrownianMotion>  gbm = std::make_shared<GeometricBrownianMotion>(underlying);

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
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping2(mc_call, *gbm, ts, numeraire_index);
		auto func2 = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping2, std::placeholders::_1);
		
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

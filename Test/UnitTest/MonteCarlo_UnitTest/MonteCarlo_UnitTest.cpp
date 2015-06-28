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

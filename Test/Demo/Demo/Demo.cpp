/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// MonteCarlo_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <cstdlib>
#include <limits>
#include <windows.h>
#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>
#include <boost/filesystem.hpp> 
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

#include "Global.hpp"
#include "EntityManager.hpp"
#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "Windows.h"
#include "EntityMgrUtil.hpp"
#include "CurrencyHolder.hpp"
#include "ExchangeHolder.hpp"
#include "StringForm.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "HistoricalStockInfo.hpp"
#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "HistoricalExchangeRateInfo.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "IIR.hpp"
#include "IIRValue.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

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
#include "FiniteDifference.hpp"
#include "Binomial.hpp"
#include "LongstaffSchwartz.hpp"
#include "QFRandom.hpp"
#include "MCAmerican.hpp"
#include "MCGeneric.hpp"
#include "MExotics.hpp"
#include "QFRandom.hpp"
#include "PiecewiseVol.hpp"
#include "QMCEngine.hpp"
#include "QFQuasiRandom.hpp"
#include "EquityVolatilitySurface.hpp"

#undef min;
#undef max;

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

/// load thelibraries first
void LoadLibraries()
{
	/// load DataSource_MySQL component explicity
	bool retValue = LoadSharedLibrary("DataSource_MySQL");
	/// Cannot continue if not load correctly.
	assert(retValue == true);

	/// load DataSource_Yahoo component explicity
	retValue = LoadSharedLibrary("DataSource_REST");
	/// Cannot continue if not load correctly.
	assert(retValue == true);

	/// load PrimaryAsset  component explicity
	retValue = LoadSharedLibrary("PrimaryAsset");
	/// Cannot continue if not load correctly.
	assert(retValue == true);

	/// load MySQLDataAccess  component explicity
	retValue = LoadSharedLibrary("MySQLDataAccess");
	/// Cannot continue if not load correctly.
	assert(retValue == true);

	/// load YahooDataAccess  component explicity
	retValue = LoadSharedLibrary("YahooDataAccess");
	/// Cannot continue if not load correctly.
	assert(retValue == true);

	/// load XigniteDataAccess  component explicity
	retValue = LoadSharedLibrary("XigniteDataAccess");
	/// Cannot continue if not load correctly.
	assert(retValue == true);
}

/// Create the Demo class
class Demo
{
public:

	enum OptionType { CALL, PUT };

	/// define option price resulted from Lattice
	struct BinomialValue
	{
		double crr;
		double jr;
		double tian;
		double lr;
	};

	//// define option price resulted from FD
	struct FDValue
	{
		double fd;
		double cn;
	};

	//// define option price resulted from MC
	struct MCValue
	{
		int path;
		double value;
		double lowerBound;
		double upperBound;

		MCValue(int p, double v, double l, double u)
			:path(p), value(v), lowerBound(l), upperBound(u)
		{};
	};

	typedef BinomialValue BinomialValueType;
	typedef FDValue FDValueType;
	typedef MCValue MCValueType;

	/// In constructor, initialize stock, interest rate, and volatilities
	Demo(std::string& symbol, double strike, dd::date maturity, OptionType opt);

	/// Destructor
	virtual ~Demo()
	{}

	/// output results
	void OutputValue(std::string optionName);

	/// Evalue with Monte-Carlo
	virtual void MC(size_t minpaths = 10000, size_t maxpaths = 10000, size_t N = 10, size_t train = 100, int degree = 2, double ci = 0.95) = 0;

	/// Evalue American call option with Binomial
	virtual void Binomial(int N = 1000) = 0;

	//// Evalute call option with Finite Difference 
	virtual  void FD(int N = 1000, int Nj = 10) = 0;

	BinomialValue GetBinomialVal() const
	{
		return m_binomial;
	}

protected:

	/// stock symbol
	std::string m_symbol;

	/// stock value
	std::shared_ptr<IStockValue> m_stockVal;

	/// BlackScholesAssetAdapter class
	std::shared_ptr<BlackScholesAssetAdapter> m_stock;

	/// term strucure corresponding to the
	/// country of underlying.
	std::shared_ptr<TermStructure> m_term;

	std::shared_ptr<EquityVolatilitySurface> m_volSurface;

	/// interest rate resulted from term structure interpolation
	double m_termRate;

	/// strike price
	double m_strike;

	/// maturity date
	dd::date m_maturity;

	/// option name. Only used for display
	OptionType m_opt;

	/// option values resulted from different
	/// pricing models.
	BinomialValueType m_binomial;
	FDValueType m_fd;
	std::vector<MCValueType> m_mc;
	double m_binaryValue;
};

/// Create the DemoAmerican class
class DemoEuropeanCall : public Demo
{
public:

	/// In constructor, initialize stock, interest rate, and volatilities
	DemoEuropeanCall(std::string& symbol, double strike, dd::date maturity);

	/// Destructor
	virtual ~DemoEuropeanCall()
	{}

	/// Evalue American call option with Binomial
	virtual void Binomial(int N = 1000);

	//// Evalute call option with Finite Difference 
	virtual void FD(int N = 1000, int Nj = 1000);

	/// evaluate with Monte-Carlo
	void MC(size_t minpaths = 100000, size_t maxpaths = 100000, size_t N = 10, size_t train = 100, int degree = 2, double ci = 0.95);
};

/// Create the DemoAmerican class
class DemoAmericanPut : public Demo
{
public:

	/// In constructor, initialize stock, interest rate, and volatilities
	DemoAmericanPut(std::string& symbol, double strike, dd::date maturity);

	/// Destructor
	virtual ~DemoAmericanPut()
	{}

	/// Evaluate with Lattice
	virtual void Binomial(int N = 1000);

	//// Evalute option with Finite Difference 
	virtual void FD(int N = 1000, int Nj = 1000);

	/// evaluate with Monte-Carlo
	void MC(size_t minpaths = 100000, size_t maxpaths = 100000, size_t N = 10, size_t train = 100, int degree = 2, double ci = 0.95);
};

Demo::Demo(std::string& symbol, double strike, dd::date maturity, OptionType opt)
	:m_symbol(symbol), m_strike(strike), m_maturity(maturity), m_opt(opt)
{
	/// get stock value.
	m_stockVal = PrimaryUtil::getStockValue(symbol);

	/// Get domestic interest rate of the stock
	dd::date today = dd::day_clock::local_day();
	auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
	std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, m_stockVal->GetStock()->GetCountry().GetCode(), today);
	m_term = irCurve->GetTermStructure();
	m_termRate = PrimaryUtil::FindInterestRate(m_stockVal->GetStock()->GetCountry().GetCode(), t, IRCurve::LIBOR);
	
	/// construct the implied vol retried.
	m_volSurface = BuildEquityVolSurface(symbol, today);
	std::shared_ptr<DeterministicAssetVol>  vol = m_volSurface->GetVolatility(m_maturity, m_strike);
	
	/// now construct the BlackScholesAdapter from the stock value.
	m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, vol);
	try
	{
		double T = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		double sgm = m_stock->GetVolatility(0.0, T);
	}
	catch (...)
	{
		vol = std::make_shared<ConstVol>(m_stockVal->GetStock()->GetImpliedVol());
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, vol);
	}
	m_stock->SetDivYield(m_stockVal->GetDivYield());	
}

void Demo::OutputValue(std::string optionName)
{
	/// Output stockValue attributes
	std::cout << "\n\n***************************************************************************" << endl;
	std::cout << "                      Output  " << optionName << " Option" << endl;
	std::cout << "***************************************************************************" << endl;
	std::cout << "Underlying equity - " << m_stockVal->GetStock()->GetDescription() << endl;
	std::cout << "Last reported price - " << m_stockVal->GetTradePrice() << endl;
	std::cout << "Last trade date - " << m_stockVal->GetTradeDate() << endl;

	std::cout << "Maturity date - " << m_maturity << endl;
	std::cout << "Strike price - " << m_strike << endl;

	std::cout << "\n         Pricing with Binomial asset pricing model " << endl;
	std::cout << "==============================================================" << endl;
	std::cout << "Valuing with Cox, Ross and Rubinstein market model = " << m_binomial.crr << endl;
	std::cout << "Valuing with Jarrow and Rudd market model = " << m_binomial.jr << endl;
	std::cout << "Valuing with Leisen and Reimer market model = " << m_binomial.lr << endl;

	std::cout << "\n      Pricing with Finite difference asset pricing model " << endl;
	std::cout << "==============================================================" << endl;
	std::cout << "Valuing with Implicit Finite Difference model = " << m_fd.fd << endl;
	std::cout << "Valuing with Crank Nicolson model = " << m_fd.cn << endl;

	std::cout << "\n               Pricing with Mote Carlo simulation " << endl;
	std::cout << "==============================================================" << endl;
	std::cout << "Paths		MC value     Lower bound CI      Upper bound CI" << endl;
	for (auto &val : m_mc)
	{
		std::cout << val.path << "		" << val.value << "		" << val.lowerBound << "		" << val.upperBound << endl;
	}

	std::cout << "\n      Pricing with Closed form asset pricing model " << endl;
	std::cout << "==============================================================" << endl;
	std::cout << "Valuing with MBinary = " << m_binaryValue << endl;
}

DemoEuropeanCall::DemoEuropeanCall(std::string& symbol, double strike, dd::date maturity)
	:Demo(symbol, strike, maturity, OptionType::CALL)
{}

DemoAmericanPut::DemoAmericanPut(std::string& symbol, double strike, dd::date maturity)
	: Demo(symbol, strike, maturity, OptionType::PUT)
{}

void DemoEuropeanCall::Binomial(int N)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		double dt = mat / N;

		BinomialLattice btree(m_stock, m_termRate, mat, N);
		Payoff call(m_strike);
		std::function<double(double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()), &call, std::placeholders::_1);
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0);
		m_binomial.crr = btree.result();
		btree.set_JarrowRudd();
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0);
		m_binomial.jr = btree.result();
		btree.set_Tian();
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0);
		m_binomial.tian = btree.result();
		btree.set_LeisenReimer(m_strike);
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0);
		m_binomial.lr = btree.result();
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

void DemoEuropeanCall::FD(int N, int Nj)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;

		FiniteDifference fd(m_stock, m_termRate, mat, N, Nj);
		CrankNicolson cn(m_stock, m_termRate, mat, N, Nj);
		Payoff call(m_strike);
		std::function<double(double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()), &call, std::placeholders::_1);
		fd.apply_payoff(N - 1, f);
		fd.rollback(N - 1, 0);
		m_fd.fd = fd.result();

		cn.apply_payoff(N - 1, f);
		cn.rollback(N - 1, 0);
		m_fd.cn = cn.result();
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

void DemoEuropeanCall::MC(size_t minpaths, size_t maxpaths, size_t N, size_t train, int degree, double ci)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		double call_strike = m_strike;
		int numeraire_index = -1;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;
		m_binaryValue = m_stock->option(mat, m_strike, m_termRate);
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
		MCEuropeanCall mc_call(0, mat, 0, call_strike);
		// instantiate MCMapping and bind to functor
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping2(mc_call, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func2 = boost::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping2, _1);
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

void DemoAmericanPut::Binomial(int N)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		BinomialLattice btree(m_stock, m_termRate, mat, N);
		Payoff put(m_strike, -1);
		std::function<double(double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()), &put, std::placeholders::_1);
		EarlyExercise amput(put);
		std::function<double(double, double)> g;
		g = std::bind(std::mem_fn(&EarlyExercise::operator()), &amput, std::placeholders::_1, std::placeholders::_2);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0, g);
		m_binomial.crr = btree.result();
		btree.set_JarrowRudd();
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0, g);
		m_binomial.jr = btree.result();
		btree.set_LeisenReimer(m_strike);
		btree.apply_payoff(N - 1, f);
		btree.rollback(N - 1, 0, g);
		m_binomial.lr = btree.result();
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

void DemoAmericanPut::FD(int N, int Nj)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;

		FiniteDifference fd(m_stock, m_termRate, mat, N, Nj);
		CrankNicolson cn(m_stock, m_termRate, mat, N, Nj);
		Payoff put(m_strike, -1);
		std::function<double(double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()), &put, std::placeholders::_1);
		EarlyExercise amput(put);
		std::function<double(double, double)> g;
		g = std::bind(boost::mem_fn(&EarlyExercise::operator()), &amput, std::placeholders::_1, std::placeholders::_2);
		fd.apply_payoff(N - 1, f);
		fd.rollback(N - 1, 0, g);
		double fdamput = fd.result();
		m_fd.fd = fd.result();
		cn.apply_payoff(N - 1, f);
		cn.rollback(N - 1, 0, g);
		m_fd.cn = cn.result();

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

void DemoAmericanPut::MC(size_t minpaths, size_t maxpaths, size_t N, size_t train, int degree, double ci)
{
	try
	{
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		double call_strike = m_strike;
		int numeraire_index = -1;
		Array<double, 1> T(N + 1);
		firstIndex idx;
		double dt = mat / N;
		T = idx*dt;

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(m_stock);
		TermStructure& ts = *m_term;
		bool include_put = false;
		exotics::StandardOption Mput(m_stock, T(0), mat, m_strike, ts, -1);
		m_binaryValue = Mput.price();
		GeometricBrownianMotion gbm(underlying);
		gbm.set_timeline(T);
		ranlib::NormalUnit<double> normalRNG;
		RandomArray<ranlib::NormalUnit<double>, double> random_container(normalRNG, gbm.factors(), gbm.number_of_steps());
		MCTrainingPaths<GeometricBrownianMotion, RandomArray<ranlib::NormalUnit<double>, double> >
			training_paths(gbm, T, train, random_container, ts, numeraire_index);
		Payoff put(m_strike, -1);
		std::function<double(double)> f;
		f = boost::bind(std::mem_fun(&Payoff::operator()), &put, _1);
		std::function<double(const Array<double, 1>&, const Array<double, 2>&)> payoff = boost::bind(REBAdapter, _1, _2, f, 0);
		std::vector<std::function<double(const Array<double, 1>&, const Array<double, 2>&)> > basisfunctions;
		Array<int, 1> p(1);
		for (int i = 0; i <= degree; i++)
		{
			p(0) = i;
			add_polynomial_basis_function(basisfunctions, p);
		}
		std::function<double(double, double)> put_option;
		put_option = boost::bind(&exotics::StandardOption::price, &Mput, _1, _2);
		std::function<double(const Array<double, 1>&, const Array<double, 2>&)> put_option_basis_function = boost::bind(REBAdapterT, _1, _2, put_option, 0);
		if (include_put) basisfunctions.push_back(put_option_basis_function);
		RegressionExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
		LSExerciseStrategy<RegressionExerciseBoundary> exercise_strategy(boundary);
		MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
		std::function<double(Array<double, 2>)> func = boost::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, _1);
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

void ClearScreen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}
void TestPricingModels()
{
	//DemoEuropeanCall(std::string& symbol, double strike, dd::date maturity);
	dd::date mat = dd::date(2015, 6, 30);
	std::string symbol("AAPL");

	/// Evaluate American call option
	DemoEuropeanCall call(symbol, 120.0, mat);
	call.Binomial();
	call.FD();
	call.MC();
	call.OutputValue(std::string("European Call"));

	/// Evaluate American put option
	DemoAmericanPut put(symbol, 110.0, mat);
	put.Binomial();
	put.FD();
	put.MC();
	put.OutputValue(std::string("American Put"));
};

void OptionSpreads(const std::shared_ptr<IStockValue>& stock, const DemoEuropeanCall& iCall, const DemoAmericanPut& iPut, const DemoEuropeanCall& oCall, const DemoAmericanPut& oPut)
{
	cout << "\t\tOption Spreads" << endl;
	cout << "\t\t==============" << endl;
	std::cout << "\tUnderlying equity - " << stock->GetStock()->GetDescription() << endl;
	std::cout << "\tLast reported price - " << stock->GetTradePrice() << endl;
	std::cout << "\tLast trade date - " << stock->GetTradeDate() << endl;

	cout << "\n\t1: Bull Call Spread " << endl;
	cout << "\t2: Bull Put Spread " << endl;
	cout << "\t3: Iron Butterfly " << endl;
	cout << "\tEnter your selection ";
	int selection;
	std::cin >> selection;
	switch (selection)
	{
	case 1:
	{
		// Buy 1 ATM Call, Sell 1 OTM Call
		cout << "\n\tSpread consists of Buy 1 ATM Call, Sell 1 OTM Call" << endl;
		double spreadVal = -iCall.GetBinomialVal().crr + oCall.GetBinomialVal().crr;
		cout << "\n\tBull Call Spread Value " << spreadVal;
	}
	break;
	case 2:
	{
		// Buy 1 OTM Put, Sell 1 ATM Put
		cout << "\n\tSpread consists of Buy 1 OTM Put, Sell 1 ITM Put" << endl;
		double spreadVal = -oPut.GetBinomialVal().crr + iPut.GetBinomialVal().crr;
		cout << "\n\tBull Put Spread Value " << spreadVal;
	}
	break;
	case 3:
	{
		// Buy 1 OTM Put, Sell 1 ATM Put, Sell 1 ATM Call, Buy 1 OTM Call
		cout << "\n\tSpread consists of Buy 1 OTM Put, Sell 1 ATM Put, Sell 1 ATM Call, Buy 1 OTM Call" << endl;
		double spreadVal = -oPut.GetBinomialVal().crr + iPut.GetBinomialVal().crr + iCall.GetBinomialVal().crr - oCall.GetBinomialVal().crr;
		cout << "\n\tIron Butterfly Spread Value " << spreadVal;
	}
	break;
	default:
		cout << "Invalid Selection ";
	}
	cout << "\n\t";
	system("pause");
	std::cin.ignore();
}

int main(int argc, char **argv)
{
	google::InitGoogleLogging("Derivative");

	/// load the libraries first
	LoadLibraries();
	std::string symbol("AAPL");
	/// get stock value.
	std::shared_ptr<IStockValue> stockVal = PrimaryUtil::getStockValue(symbol);
	auto stockPrice = stockVal->GetTradePrice();
	auto instrincPnct = stockVal->GetTradePrice()/10;
	dd::date mat = dd::date(2015, 6, 30);
	DemoEuropeanCall ATMCall(symbol, stockVal->GetTradePrice(), mat);
	ATMCall.Binomial();
	DemoEuropeanCall OUTCall(symbol, stockVal->GetTradePrice() + instrincPnct, mat);
	OUTCall.Binomial();
	DemoAmericanPut ATMPut(symbol, stockVal->GetTradePrice(), mat);
	ATMPut.Binomial();
	DemoAmericanPut OUTPut(symbol, stockVal->GetTradePrice() - instrincPnct, mat);
	OUTPut.Binomial();
	while (1)
	{
		ClearScreen();
		OptionSpreads(stockVal, ATMCall, ATMPut, OUTCall, OUTPut);
	}
}


/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// Binomial_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream> 
#include <cstdlib>
#include <memory>
#include <functional>

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
#include "HistoricalStockInfo.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "HistoricalExchangeRateInfo.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "Payoff.hpp"
#include "Binomial.hpp"
#include "ConstVol.hpp"
#include "IStock.hpp"
#include "IIR.hpp"
#include "IIRValue.hpp"
#include "Maturity.hpp"
#include "PrimaryAssetUtil.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;
using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;
using namespace std::placeholders;

#undef max;
#undef min;

// Declare a new test fixture for BinomialTest, deriving from testing::Test.
class BinomialTest : public testing::Test 
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

	/// Declares the variables your tests want to use.
	/// stock object ptrs
	std::shared_ptr<IObject> stockPtr;
	std::shared_ptr<IObject> stockValuePtr;

	/// exchange rate ptrs
	std::shared_ptr<IObject> exchangeRatePtr;
	std::shared_ptr<IObject> exchangeRateValuePtr;

};


TEST_F(BinomialTest, LoadLibraries) {

	/// load DataSource_MySQL component explicity
	bool retValue = LoadSharedLibrary("DataSource_MySQL");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load DataSource_Yahoo component explicity
	retValue = LoadSharedLibrary("DataSource_REST");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load PrimaryAsset  component explicity
	retValue = LoadSharedLibrary("PrimaryAsset");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load MySQLDataAccess  component explicity
	retValue = LoadSharedLibrary("MySQLDataAccess");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);	

	/// load YahooDataAccess  component explicity
	retValue = LoadSharedLibrary("YahooDataAccess");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load XigniteDataAccess  component explicity
	retValue = LoadSharedLibrary("XigniteDataAccess");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);
}

std::shared_ptr<IStockValue> getStockValue(const std::string& symbol)
{
	std::shared_ptr<IObject> stockPtr;
	std::shared_ptr<IObject> stockValuePtr;

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();
	auto stockId  = std::hash<std::string>()(symbol);

	/// find apple stock object through registry.
	Name stockName(IStock::TYPEID, stockId);
	bool foundFlag = true;
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(stockName, names);

	/// This call should find the named object in
	/// registry. If not found then it should fetch
	/// the stock data from database, construct stock
	/// and register with entity manager.
	stockPtr = EntityMgrUtil::findObject(stockName);

	/// find current stock value for apple
	Name stockValName(IStockValue::TYPEID, stockId);
	stockValName.AppendKey(string("symbol"), symbol);
	foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> valNames;
	bool retStockValValue = entMgr.findAlias(stockValName, valNames);
	/// This call should find the named object in
	/// registry. If not found then it should fetch
	/// the stock value from yahoo, construct stock value
	/// and register with entity manager.
	stockValuePtr = EntityMgrUtil::findObject(stockValName, YAHOO);

	std::shared_ptr<IStockValue> stock = dynamic_pointer_cast<IStockValue>(stockValuePtr);
	return stock;
}

TEST_F(BinomialTest, BinomialTest) 
{
	try
	{
		std::string symbol("AAPL");
		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol);

		/// If we are here without any assert failures mean, we got apple stock
		/// object and current apple stock value.
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);
		double divYield = stockVal->GetDivYield();

		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(stockVal->GetAsset()->GetImpliedVol());
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(vol));
		stock->SetDivYield(0);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		dd::date today = dd::day_clock::local_day();		
		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::YIELD);
		double K = 1.0;
		K *= stockVal->GetTradePrice();
		int N = 10;
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << mat << endl;

		double CFcall = stock->option(mat,K,r);
		cout << "Closed for call " << CFcall << endl;
		double CFput  = stock->option(mat,K,r,-1);
		cout << "Closed for put " << CFput << endl;
		double CFiput = stock->option(T(N/2),K,r,-1);
		BinomialLattice btree(stock,r,mat,N);  
		Payoff call(K);
		Payoff put(K,-1);
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (CRR): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (JR): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
		f = std::bind(std::mem_fun(&Payoff::operator()),&put,_1);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (CRR): " << btree.result() << "\nDifference to closed form: " << CFput - btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (JR): " << btree.result() << "\nDifference to closed form: " << CFput - btree.result() << endl;
		btree.set_Tian();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (Tian): " << btree.result() << "\nDifference to closed form: " << CFput - btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (LR): " << btree.result() << "\nDifference to closed form: " << CFput - btree.result() << endl;
		EarlyExercise amput(put);
		std::function<double (double,double)> g;
		g = std::bind(std::mem_fn(&EarlyExercise::operator()),&amput,_1,_2);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (JR): " << btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (LR): " << btree.result() << endl;

		// with dividends
		stock->SetDivYield(divYield);
		CFcall = stock->option(mat,K,r);
		CFput  = stock->option(mat,K,r,-1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (JR): " << btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (LR): " << btree.result() << endl;
		f = std::bind(std::mem_fun(&Payoff::operator()),&put,_1);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (JR): " << btree.result() << endl;
		btree.set_Tian();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (Tian): " << btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial put (LR): " << btree.result() << endl;
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (JR): " << btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (LR): " << btree.result() << endl;
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


TEST_F(BinomialTest, BinomialCall) 
{
	std::string symbol("AAPL");
	std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol);
	/// If we are here without any assert failures mean, we got apple stock
	/// object and current apple stock value.
	std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);

	/// override the div yield to have 0 yield.
	double divYield = stockVal->GetDivYield();

	try 
	{
		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(stockVal->GetAsset()->GetImpliedVol());
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(vol));
		stock->SetDivYield(0);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		dd::date today = dd::day_clock::local_day();
		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::YIELD);
		double K = 1.0;
		K *= stockVal->GetTradePrice();

		int N = 100;
		double dt = mat/N;

		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << mat << endl;
		double CFcall = stock->option(mat,K,r);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Time line refinement: " << N << endl;
		cout << "Creating BinomialLattice object" << endl;
		BinomialLattice btree(stock,r,mat,N);  
		Payoff call(K);
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (CRR): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (JR): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
		btree.set_Tian();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (Tian): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0);
		cout << "Binomial call (LR): " << btree.result() << "\nDifference to closed form: " << CFcall - btree.result() << endl;
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

TEST_F(BinomialTest, BinomialAmPut) 
{
	try
	{
		std::string symbol("AAPL");
		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol);

		/// If we are here without any assert failures mean, we got apple stock
		/// object and current apple stock value.
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);

		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(stockVal->GetAsset()->GetImpliedVol());
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(vol));
		stock->SetDivYield(0);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		dd::date today = dd::day_clock::local_day();

		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::YIELD);
		double K = 1.0;
		K *= stockVal->GetTradePrice();		
		int N = 20;
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " \
			<< mat << endl;
		cout << "Time line refinement: " << N << endl;
		cout << "Creating BinomialLattice object" << endl;
		BinomialLattice btree(stock,r,mat,N);  
		Payoff put(K,-1);
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()),&put,_1);
		EarlyExercise amput(put);
		std::function<double (double,double)> g;
		g = std::bind(std::mem_fn(&EarlyExercise::operator()),&amput,_1,_2);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (JR): " << btree.result() << endl;
		btree.set_LeisenReimer(K);
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial American put (LR): " << btree.result() << endl;
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

TEST_F(BinomialTest, BinomialDownOut) 
{
	try
	{
		std::string symbol("AAPL");
		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol);

		/// If we are here without any assert failures mean, we got apple stock
		/// object and current apple stock value.
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);

		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(stockVal->GetAsset()->GetImpliedVol());
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(vol));
		stock->SetDivYield(0);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		dd::date today = dd::day_clock::local_day();

		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::YIELD);
		double K = 1.0;
		K *= stockVal->GetTradePrice();
		double barrier = 0.95;
		barrier *= stockVal->GetTradePrice();
		int N = 10;
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " \
			<< mat  << endl;
		cout << "Time line refinement: " << N << endl;
		cout << "Creating BinomialLattice object" << endl;
		BinomialLattice btree(stock,r,mat,N);  
		Payoff call(K);
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		KnockOut down_and_out(barrier);
		std::function<double (double,double)> g;
		g = std::bind(std::mem_fn(&KnockOut::operator()),&down_and_out,_1,_2);
		btree.set_CoxRossRubinstein();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial down and out call (CRR): " << btree.result() << endl;
		btree.set_JarrowRudd();
		btree.apply_payoff(N-1,f);
		btree.rollback(N-1,0,g);
		cout << "Binomial down and out call (JR): " << btree.result() << endl;
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
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}

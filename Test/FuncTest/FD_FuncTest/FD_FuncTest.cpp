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
#include "FiniteDifference.hpp"
#include "MCGatherer.hpp"
#include "Payoff.hpp"
#include "IIR.hpp"
#include "IIRValue.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;
using namespace std::placeholders;

// Declare a new test fixture for FDTest, deriving from testing::Test.
class FDTest : public testing::Test 
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

TEST_F(FDTest, LoadLibraries) {

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

	/// load YahooDataAccess  component explicity
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

TEST_F(FDTest, FDTest) 
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
			std::make_shared<BlackScholesAssetAdapter>(stockVal,vol);
		stock->SetDivYield(0);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		auto maturity = Maturity::MaturityType::Y2;
		dd::date today = dd::day_clock::local_day();
		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::YIELD);
		double K = 1.0;
		K *= stockVal->GetTradePrice();
		int N = 10;
		int Nj = 10;
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << mat << endl;
		double CFcall = stock->option(mat,K,r);
		double CFput  = stock->option(mat,K,r,-1);
		double CFiput = stock->option(T(N/2),K,r,-1);
		cout << "Closed form call: " << CFcall << "  delta: " << stock->delta(mat,K,r) << "  gamma: " << stock->gamma(mat,K,r) << endl;
		cout << "Closed form put: " << CFput << "  delta: " << stock->delta(mat,K,r,-1) << "  gamma: " << stock->gamma(mat,K,r,-1) << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;
		cout << "Time line refinement: " << N << "\nState space refinement: " << Nj << endl;
		cout << "Creating FiniteDifference object" << endl;
		FiniteDifference fd(stock,r,mat,N,Nj);
		cout << "Creating CrankNicolson object" << endl;
		CrankNicolson cn(stock,r,mat,N,Nj);  
		Payoff call(K);
		Payoff put(K,-1);
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		fd.apply_payoff(N-1,f);
		fd.rollback(N-1,0);
		cout << "FD call: " << fd.result() << "\nDifference to closed form: " << CFcall - fd.result() << endl;
		cout << "FD call delta: " << fd.delta() << "  FD call gamma: " << fd.gamma() << endl;
		cn.apply_payoff(N-1,f);
		cn.rollback(N-1,0);
		cout << "CrankNicolson call: " << cn.result() << "\nDifference to closed form: " << CFcall - cn.result() << endl;
		cout << "CrankNicolson call delta: " << cn.delta() << "  CrankNicolson call gamma: " << cn.gamma() << endl;
		f = std::bind(std::mem_fun(&Payoff::operator()),&put,_1);
		fd.apply_payoff(N-1,f);
		fd.rollback(N-1,0);
		cout << "FD put: " << fd.result() << "\nDifference to closed form: " << CFput - fd.result() << endl;
		cout << "FD put delta: " << fd.delta() << "  FD put gamma: " << fd.gamma() << endl;
		cn.apply_payoff(N-1,f);
		cn.rollback(N-1,0);
		cout << "CrankNicolson put: " << cn.result() << "\nDifference to closed form: " << CFput - cn.result() << endl;
		cout << "CrankNicolson put delta: " << cn.delta() << "  CrankNicolson put gamma: " << cn.gamma() << endl;
		EarlyExercise amput(put);
		std::function<double (double,double)> g;
		g = std::bind(boost::mem_fn(&EarlyExercise::operator()),&amput,_1,_2);
		fd.apply_payoff(N-1,f);
		fd.rollback(N-1,0,g);
		double fdamput = fd.result();
		cout << "FD American put: " << fdamput << endl;
		cout << "FD American put delta: " << fd.delta() << "  \nFD American put gamma: " << fd.gamma() << endl;
		cn.apply_payoff(N-1,f);
		cn.rollback(N-1,0,g);
		cout << "CrankNicolson American put: " << cn.result() << endl;
		cout << "CrankNicolson American put delta: " << cn.delta() << "  \nCrankNicolson American put gamma: " << cn.gamma() << endl;

		// with dividends
		stock->SetDivYield(divYield);
		FiniteDifference fd2(stock,r,mat,N,Nj);
		CFcall = stock->option(mat,K,r);
		CFput  = stock->option(mat,K,r,-1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		f = std::bind(std::mem_fun(&Payoff::operator()),&call,_1);
		fd2.apply_payoff(N-1,f);
		fd2.rollback(N-1,0);
		cout << "FD call: " << fd2.result() << endl;
		CrankNicolson cn2(stock,r,mat,N,Nj);  
		cn2.apply_payoff(N-1,f);
		cn2.rollback(N-1,0);
		cout << "CrankNicolson call: " << cn2.result() << endl;
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

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}

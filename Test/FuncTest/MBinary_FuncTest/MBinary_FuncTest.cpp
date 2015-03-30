/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// MBinary_unittest.cpp : Defines the entry point for the console application.
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
#include "PiecewiseVol.hpp"
#include "StringForm.hpp"
#include "MBinary.hpp"
#include "MExotics.hpp"
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

#undef max;
#undef min;

// Declare a new test fixture for MBinaryTest, deriving from testing::Test.
class MBinaryTest : public testing::Test 
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
	/// Declares the variables your tests want to use.
	/// stock object ptrs
	std::shared_ptr<IObject> stockPtr;
	std::shared_ptr<IObject> stockValuePtr;

	/// exchange rate ptrs
	std::shared_ptr<IObject> exchangeRatePtr;
	std::shared_ptr<IObject> exchangeRateValuePtr;
};


TEST_F(MBinaryTest, LoadLibraries) {

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

TEST_F(MBinaryTest, MBinaryExample) 
{
	try
	{
		std::string symbol("AAPL");
		std::shared_ptr<IStockValue> stockValuePtr = getStockValue(symbol);

		/// If we are here without any assert failures mean, we got apple stock
		/// object and current apple stock value.
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);
		double divYield = stockVal->GetDivYield();

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		dd::date today = dd::day_clock::local_day();
		double mat = 1.5;
		double r = PrimaryUtil::FindInterestRate(cntry, mat, IRCurve::LIBOR);
		double K = 1.0;
		K *= stockVal->GetTradePrice();
		int N = 10;
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << r << "\nT: " << mat << endl;
		double call_strike = K;

		// create underlying asset - note that volatility is a two-dimensional vector with equal entries
		Array<double,1> sgm1(2);
		sgm1 = 0.3;	
		cout << "S: " << stockVal->GetTradePrice() << "\nK: " << K << "\nr: " << \
			r << "\nT: " << mat << endl;
		cout << "T := " << T << endl;
		cout << " Sigma1 := " << sgm1 << endl;

		std::shared_ptr<DeterministicAssetVol>  vol = std::make_shared<ConstVol>(sgm1);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,vol);
		stock->SetDivYield(0.03);

		// closed-form vanilla option prices
		double CFcall = stock->option(mat,K,r);
		double CFput  = stock->option(mat,K,r,-1);
		double CFiput = stock->option(T(N/2),K,r,-1);
		cout << "Closed form call: " << CFcall << endl;
		cout << "Closed form put: " << CFput << endl;
		cout << "Closed form intermediate maturity put: " << CFiput << endl;

		// initialise vector of underlying assets
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		underlying.push_back(stock);
		// flat term structure for discounting
		FlatTermStructure ts(r,0.0,mat+10.0);
		// create MBinaries for European call option
		Array<int,2> index(2,1);
		index = 0, N;
		Array<double,1> alpha(1);
		alpha = 1.0;
		Array<int,2> xS(1,1);
		xS = 1;
		Array<double,2> xA(1,1);
		xA = 1.0;
		Array<double,1> xa(1);
		xa = K;
		MBinary V1(underlying,ts,T,index,alpha,xS,xA,xa);
		Array<double,1> alpha2(1);
		alpha2 = 0.0;
		MBinary V2(underlying,ts,T,index,alpha2,xS,xA,xa);
		cout << "MBinary price of call option: " << V1.price(1000000000)-K*V2.price(1000000000) << std::endl;
		// create second asset
		Array<double,1> sgm2(2);
		sgm2 = 0.1, 0.4;
		std::shared_ptr<DeterministicAssetVol>  vol2 = std::make_shared<ConstVol>(sgm2);
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock2 = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,vol2);
		stock2->SetDivYield(0.04);

		// option to exchange one asset for another
		// closed form price via formula implemented as member function of BlackScholesAsset - K is strike factor
		K = 1.0;
		double margrabe_price = stock->Margrabe(stock2,mat,K);
		cout << "Closed form Margrabe option: " << margrabe_price << endl;
		// add second asset to vector of underlying assets
		underlying.push_back(stock2);
		// create MBinaries for Margrabe option
		Array<int,2> mindex(2,2);
		mindex = 0, 1, 
			N, N;
		Array<double,1> malpha(2);
		malpha = 1.0, 0.0;
		Array<double,2> mA(1,2);
		mA = 1.0, -1.0;
		Array<double,1> ma(1);
		ma = K;
		MBinary M1(underlying,ts,T,mindex,malpha,xS,mA,ma);
		double M1price = M1.price(1000000000);
		Array<double,1> malpha2(2);
		malpha2 = 0.0, 1.0;
		MBinary M2(underlying,ts,T,mindex,malpha2,xS,mA,ma);
		cout << "MBinary price of Margrabe option: " << M1price-K*M2.price(1000000000) << std::endl;
		// use the Margrabe option class from MExotics.hpp
		exotics::Margrabe margrabe(stock,stock2,0.0,mat,1.0,K,ts);
		cout << "Price of Margrabe option via MExotics: " << margrabe.price() << std::endl;

		// European call option price (on asset with piecewise constant vol)

		Array<double,2> piecewise_vols(T.extent(firstDim)-1,2);
		for (int i=0;i<piecewise_vols.extent(firstDim);i++)
		{
			piecewise_vols(i,0) = sgm2(0) * (T(i+1)-T(0))/(T(T.extent(firstDim)-1)-T(0)) * 1.5;
			piecewise_vols(i,1) = sgm2(1) * (T(i+1)-T(0))/(T(T.extent(firstDim)-1)-T(0)) * 1.5;
		}

		cout << "T = " << T << endl;
		cout << "Vol = " << piecewise_vols << endl;

		std::shared_ptr<DeterministicAssetVol>  pvol = std::make_shared<PiecewiseConstVol>(T,piecewise_vols);

		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> passet = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,pvol);
		passet->SetDivYield(.03);

		double CFprice = passet->option(mat,call_strike,r);
		cout << " call option price via piecewise volatility := " << CFprice << endl;

		// Geometric average option price 
		/// construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> gasset = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal);
		gasset->SetDivYield(0);

		/// Fixed strike
		exotics::DiscreteGeometricMeanFixedStrike geo(gasset,T,call_strike,ts,1,stockVal->GetTradePrice());
		double CFgeo = geo.price();
		cout << "Closed form value of geometric average option: " << CFgeo << endl;
		// Floating strike
		exotics::DiscreteGeometricMeanFloatingStrike geofloat(gasset,T,call_strike/stockVal->GetTradePrice(),ts,1,stockVal->GetTradePrice());
		CFgeo = geofloat.price();
		cout << "Closed form value of geometric average option (floating strike): " << CFgeo << endl;
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
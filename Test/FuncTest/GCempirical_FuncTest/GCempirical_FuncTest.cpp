/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// GCempirical_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream> 
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp> 
#include <string>
#include "gtest/gtest.h"
#include "BlackScholesAsset.hpp"
#include "Payoff.hpp"
#include "Binomial.hpp"
#include "ConstVol.hpp"
#include "Global.hpp"

#include <fstream>
#include <boost/math/distributions/normal.hpp>
#include <random/normal.h>
#include "GramCharlierAssetAdapter.hpp"
#include "CSV2Array.hpp"

#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "Windows.h"
#include "Global.hpp"
#include "IStockValue.hpp"
#include "Maturity.hpp"
#include"IIRValue.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IStock.hpp"
#include "VolatilitySmile.hpp"
#include "EquityVolatilitySurface.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

/// define command line variables here. FIXME??
int highest_moment;
char  filename[25];

// Declare a new test fixture for BinomialTest, deriving from testing::Test.
class GCempirical : public testing::Test 
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
};


TEST_F(GCempirical, LoadLibraries) {

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

/*
TEST_F(GCempirical, GCempiricalFileDataTest) 
{
	using std::cout;
	using std::endl;
	using std::flush;

	int i;
	double Spot,ttm,r_d,r_f,Delta,volATM,volRR,volBF,Delta2,volRR2,volBF2;
	Delta2 = volRR2 = volBF2 = 0.0;
	try 
	{
		std::string symbol("AAPL");
		std::shared_ptr<IStockValue> stockValuePtr = PrimaryUtil::getStockValue(symbol);

		/// If we are here without any assert failures mean, we got apple stock
		/// object and current apple stock value.
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(stockValuePtr);

		std::ifstream is_inputs(filename);

		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::cout << "Inputs: " << inputs_matrix << std::endl;
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		if (inputs_map.count("Spot")) Spot = std::atof(inputs_map["Spot"].data());
		stockValuePtr->SetTradePrice(Spot);
		if (inputs_map.count("Delta")) Delta = std::atof(inputs_map["Delta"].data());
		if (inputs_map.count("ATM")) volATM = std::atof(inputs_map["ATM"].data())/100.0;
		if (inputs_map.count("RR")) volRR = std::atof(inputs_map["RR"].data())/100.0;
		if (inputs_map.count("BF")) volBF = std::atof(inputs_map["BF"].data())/100.0;
		if (inputs_map.count("ttm")) ttm = std::atof(inputs_map["ttm"].data());
		else 
		{
			ttm = 0.0;
			if (inputs_map.count("TTM")) 
			{
				if (inputs_map["TTM"]==std::string("1M")) ttm = 1.0/12.0;
				if (inputs_map["TTM"]==std::string("3M")) ttm = 0.25; 
			}
		}
		if (ttm==0.0) throw std::logic_error("Unknown time to maturity");
		if (inputs_map.count("r_d")) 
		{
			r_d = std::atof(inputs_map["r_d"].data());
			r_d = std::log(1+ttm*r_d); 
		}
		if (inputs_map.count("r_f")) 
		{
			r_f = std::atof(inputs_map["r_f"].data());
			r_f = std::log(1+ttm*r_f); 
		}
		double sgmcdelta = volBF + 0.5*volRR + volATM;
		double sgmpdelta = sgmcdelta - volRR;
		double domestic_discount = exp(-r_d*ttm);
		double foreign_discount  = exp(-r_f*ttm);
		double forwardFX = Spot * foreign_discount/domestic_discount;
		double sqrtttm = std::sqrt(ttm);
		boost::math::normal normal;
		double d = boost::math::quantile(normal,Delta);
		double cdeltastrike = Spot * std::exp(-sgmcdelta*sqrtttm*d+(r_d-r_f+0.5*sgmcdelta*sgmcdelta)*ttm);
		d = boost::math::quantile(normal,1.0-Delta);
		double pdeltastrike = Spot * std::exp(-sgmpdelta*sqrtttm*d+(r_d-r_f+0.5*sgmpdelta*sgmpdelta)*ttm);
		if (inputs_map.count("Delta2")) Delta2 = std::atof(inputs_map["Delta2"].data());
		if (inputs_map.count("RR2")) volRR2 = std::atof(inputs_map["RR2"].data())/100.0;
		if (inputs_map.count("BF2")) volBF2 = std::atof(inputs_map["BF2"].data())/100.0;
		double sgmcdelta2 = volBF2 + 0.5*volRR2 + volATM;
		double sgmpdelta2 = sgmcdelta2 - volRR2;
		double cdelta2strike,pdelta2strike;
		int nstrikes = 3;
		if (Delta2>0.0) 
		{
			nstrikes = 5;
			d = boost::math::quantile(normal,Delta2);
			cdelta2strike = Spot * std::exp(-sgmcdelta2*sqrtttm*d+(r_d-r_f+0.5*sgmcdelta2*sgmcdelta2)*ttm);
			d = boost::math::quantile(normal,1.0-Delta2);
			pdelta2strike = Spot * std::exp(-sgmpdelta2*sqrtttm*d+(r_d-r_f+0.5*sgmpdelta2*sgmpdelta2)*ttm); 
		}
		Array<double,1> coeff(highest_moment+1);
		coeff = 0.0;
		coeff(0) = 1.0;
		GramCharlier gc(coeff);
		GramCharlierAsset gcasset(gc,volATM*sqrtttm,Spot,ttm);

		cout << "Calibrate:" << endl;
		std::shared_ptr<Array<double,1> > strikes = std::make_shared<Array<double,1> >(nstrikes);
		std::shared_ptr<Array<double,1> > vols = std::make_shared<Array<double,1> >(nstrikes);
		strikes->operator()(blitz::Range(0,2)) = pdeltastrike,forwardFX,cdeltastrike;
		vols->operator()(blitz::Range(0,2))    = sgmpdelta,volATM,sgmcdelta;
		if (nstrikes==5) 
		{
			strikes->operator()(3) = pdelta2strike;
			strikes->operator()(4) = cdelta2strike;
			vols->operator()(3)    = sgmpdelta2;
			vols->operator()(4)    = sgmcdelta2; 
		}
		cout << "Strikes: " << *strikes << "\nVolatilities: " << *vols << endl;
		double match = gcasset.calibrate(strikes,vols,domestic_discount,foreign_discount,highest_moment);
		cout << "Calibration result:\nObjective function: " << match << endl;
		cout << "Sigma:, " << gcasset.standard_deviation() << "\nSkewness:, " << gcasset.skewness() << ", Excess kurtosis:, " << gcasset.excess_kurtosis() << endl;
		cout << "GC coefficients: " << gc.coefficients()  << endl;
		cout << "Strike,Implied Volatility,Black/Scholes Price,Fitted Gram/Charlier Price" << endl;
		for (i=0;i<nstrikes;i++) 
		{
			cout << strikes->operator()(i) << ',' << vols->operator()(i) << ',';
			std::unique_ptr<DeterministicAssetVol>  v(new ConstVol(vols->operator()(i)));
		    /// now construct the BlackScholesAdapter from the stock value.
		    std::shared_ptr<BlackScholesAssetAdapter> asset = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(v));
		    asset->SetDivYield(r_f);

			cout << asset->option(ttm,strikes->operator()(i),r_d) << ',' << gcasset.call(strikes->operator()(i),domestic_discount,foreign_discount) << endl; 
		}
		cout << "Generating data for a smile plot:\nStrike,Price,Implied Volatility" << endl;

		std::unique_ptr<DeterministicAssetVol>  cv(new ConstVol(volATM));
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> stock = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,std::move(cv));
		stock->SetDivYield(r_f);

		double K;
		double startK = 0.8*strikes->operator()(0);
		double endK   = 1.2 * strikes->operator()(2);
		double dK     = (endK-startK)/100.0;
		for (K=startK;K<=endK;K+=dK)
		{
			double price = gcasset.call(K,domestic_discount,foreign_discount);
			cout << K << ',' << price << ',' << stock->CalculateImpliedVolatility(price,ttm,K,r_d) << endl; 
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

*/

/*
TEST_F(GCempirical, GCempiricalRealDataTest)
{
	std::string symbol("AAPL");
	int maturity = (dd::date(2016, 01, 15) - dd::date(2014, 10, 13)).days();
	std::shared_ptr<IStockValue> stockVal;
	try
	{
		stockVal = PrimaryUtil::getStockValue(symbol);
		cout << "Retrieved stock data  " << stockVal->GetAsset()->GetSymbol() \
			<< " Trade price " << stockVal->GetTradePrice() \
			<< " Trade date " << stockVal->GetTradeDate() << endl;
	}
	catch(RegistryException& e)
	{
		LOG(ERROR) << "Registry exception occurred: Unable to get " << symbol << endl;
		ASSERT_TRUE(true);
	}
	catch(std::exception& e)
	{
		LOG(ERROR) << "exception occurred: Unable to get " << symbol << endl;
		ASSERT_TRUE(true);
	}

	/// we get the stock value..
	int highest_moment = 4;
	Array<double,1> coeff(highest_moment+1);
	coeff = 0.0;
	coeff(0) = 1.0;
	GramCharlier gc(coeff);
	std::shared_ptr<GramCharlierAssetAdapter> gcAdapter = GramCharlierAssetAdapter::Create(gc, stockVal, maturity);
	gcAdapter->calibrate(1, 1, highest_moment);
}

*/

/*
TEST_F(GCempirical, VolatilitySurfaceTest)
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the volatility surface for AAPL
	std::string symbol("AAPL");
	dd::date today = dd::day_clock::local_day();
	try
	{
		std::shared_ptr<EquityVolatilitySurface> surface = BuildEquityVolSurface(symbol, today);
		std::shared_ptr<DeterministicAssetVol> vol1 = surface->GetVolByStrike(100.0);
		cout << "Vol 1" << vol1->component_vol(0) << endl;
		std::shared_ptr<DeterministicAssetVol> vol2 = surface->GetVolByStrike(20.0);
		cout << "Vol 2" << vol2->component_vol(0) << endl;
		std::shared_ptr<DeterministicAssetVol> vol3 = surface->GetVolByStrike(108.5);
		cout << "Vol 3" << vol3->component_vol(0) << endl;
		std::shared_ptr<DeterministicAssetVol> vol4 = surface->GetVolByStrike(1000.0);
		cout << "Vol 4" << vol4->component_vol(0) << endl;
	}
	catch (RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(false);
	}
	catch (...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

*/
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");

	// change the current working directory
	/*
	std::cout << boost::filesystem::current_path() << std::endl; 
	_chdir("..\\CSVInputs"); 
	std::cout << boost::filesystem::current_path() << std::endl; 

	for (int i=0;i< argc;i++) cout << argv[i] << ' ';
	cout << endl;
	if (argc<2) throw std::logic_error("Missing input data CSV file name");
	highest_moment = 4;
	if (argc>2) highest_moment = std::atoi(argv[2]);
	strcpy(filename,argv[1]);
	*/
	return RUN_ALL_TESTS();
}
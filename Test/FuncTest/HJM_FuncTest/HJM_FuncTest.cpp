
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// HJM_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <memory>
#include <functional>

#include <boost/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/filesystem.hpp> 
#include "gtest/gtest.h"
#include "Global.hpp"

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

#include "TSBootstrap.hpp"
#include "QFRandom.hpp"
#include "CSV2Array.hpp"
#include "GaussianEconomy.hpp"
#include "GaussMarkovWorld.hpp"
#include "MCGeneric.hpp"
#include "LongstaffSchwartz.hpp"
#include "MCAmerican.hpp"
#include "Payoff.hpp"
#include "MExotics.hpp"
#include "ExponentialVol.hpp"
#include "TestUtil.hpp"


using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

//using namespace std::placeholders;

#undef max;
#undef min;

string csvfilepath;

// Declare a new test fixture for BinomialTest, deriving from testing::Test.
class HJMTest : public testing::Test 
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


TEST_F(HJMTest, LoadLibraries) 
{
	/// load DataSource_MySQL component explicity
	bool retValue = LoadSharedLibrary("DataSource_MySQL");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load DataSource_Yahoo component explicity
	retValue = LoadSharedLibrary("DataSource_Yahoo");
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
}

/*
TEST_F(HJMTest, LSMCHJM) 
{
	int i,j;
	try
	{
		size_t minpaths = 100;
		size_t maxpaths = 4000000;

		// Read data from files and create multicurrency term structure model
		std::ifstream is_inputs("inputlsmchjm.csv");
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open input parameter CSV file");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));

		GaussMarkovWorld world("worldbs.csv");
		double maturity = 10.0;
		size_t train = 100;
		int degree = 2;
		int IRdegree = 0;
		int crossdegree = 0;
		int steps = 60;
		double strike = 110.0;
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		if (inputs_map.count("Strike"))                   strike = std::atof(inputs_map["Strike"].data());
		if (inputs_map.count("Expiry"))                 maturity = std::atof(inputs_map["Expiry"].data());
		if (inputs_map.count("Training paths"))            train = std::atoi(inputs_map["Training paths"].data());
		if (inputs_map.count("Polynomial degree"))        degree = std::atoi(inputs_map["Polynomial degree"].data());
		if (inputs_map.count("IR polynomial degree"))   IRdegree = std::atoi(inputs_map["IR polynomial degree"].data());
		if (inputs_map.count("Cross polynomial degree")) crossdegree = std::atoi(inputs_map["Cross polynomial degree"].data());
		if (inputs_map.count("Steps"))                     steps = std::atoi(inputs_map["Steps"].data());
		cout << "Training paths," << train << "\nPolynomial degree," << degree << "\nIR polynomial degree," << IRdegree << "\nCross polynomial degree," << crossdegree << "\nSteps," << steps << endl;
		Array<double,1> timeline(steps+1);
		double dt = maturity/steps;
		firstIndex idx;
		timeline = idx * dt;
		const std::vector<std::shared_ptr<GaussianEconomy> >& ecv = world.get_economies();
		const GaussianEconomy& domestic_economy = *(ecv[0]);
		std::shared_ptr<BlackScholesAssetAdapter> Sp(domestic_economy.underlying[0]);
		const BlackScholesAsset& S = Sp->GetOrigin();
		world.set_timeline(timeline);
		Array<double,1> numeraire_values(timeline.extent(firstDim));
		cout << "European option price: " << domestic_economy.hjm->option(S,maturity,strike,-1) << endl;

		boost::mt19937 boost_random_engine; // this should be the single instance of a random number generator in the whole system
		// Run the actual simulation
		int numeraire_index = 0;
		cout << "Numeraire index: " << numeraire_index << endl;
		Array<double,1> T(timeline);
		world.set_reporting(0,0);
		world.set_reporting(0,-1,world.time_horizon());
		boost::normal_distribution<double> normal;
		
		std::shared_ptr<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> > > normalRNG = \
			std::make_shared<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> > >(boost_random_engine, normal);
		std::shared_ptr<RandomWrapper<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> >, double> > normalRNGwrap = \
			std::make_shared<RandomWrapper<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> >, double>>(normalRNG);
		RandomArray<RandomWrapper<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> >,double>,double> 
			random_container(normalRNGwrap,world.factors(),world.number_of_steps()); 
		
		MCTrainingPaths<GaussMarkovWorld,RandomArray<RandomWrapper<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> >,double>,double> >
			training_paths(world,T,train,random_container,*(domestic_economy.initialTS),numeraire_index);
		std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> > basisfunctions;
		Array<int,1> p(2);
		p(1) = 0.0;
		for (i=0;i<=degree;i++)
		{
			p(0) = i;
			add_polynomial_basis_function(basisfunctions,p); 
		}
		if (IRdegree) 
		{
			p(0) = 0.0;
			for (i=1;i<=IRdegree;i++) 
			{
				p(1) = i;
				add_polynomial_basis_function(basisfunctions,p);
			}
		}
		if (crossdegree) 
		{
			for (i=1;i<=crossdegree;i++)
			{
				p(1) = p(0) = i;
				add_polynomial_basis_function(basisfunctions,p); 
			}
		}
		// BlackScholesAsset inputs are not used by Monte Carlo payoff from Mopt
		exotics::ProductOption Mopt(domestic_economy.underlying[0],domestic_economy.underlying[0],\
			T(0),world.time_horizon(),strike,*(domestic_economy.initialTS),-1);
		std::function<double (const Array<double,1>&,const Array<double,2>&)> payoff = \
			std::bind(&exotics::ProductOption::early_exercise_payoff,&Mopt,std::placeholders::_1,std::placeholders::_2);
		Array<double,1> tmp(training_paths.state_variables()(1,Range::all(),1));
		cout << "Training path for bond:\n" << tmp << endl;
		RegressionExerciseBoundary boundary(T,training_paths.state_variables(),training_paths.numeraires(),payoff,basisfunctions);
		LSExerciseStrategy<RegressionExerciseBoundary> exercise_strategy(boundary);
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(exercise_strategy,world,*(domestic_economy.initialTS),numeraire_index);
		std::function<double (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mapping,&mc_mapping,std::placeholders::_1);
		MCGeneric<Array<double,2>,
			double,
			RandomArray<RandomWrapper<boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> >,double>,double> > 
			mc(func,random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normdist;
		double d = boost::math::quantile(normdist,0.95);
		cout << "Simulations,MC value,95% CI lower bound,95% CI upper bound" << endl;
		while (mcgatherer.number_of_simulations()<maxpaths)
		{
			mc.simulate(mcgatherer,n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean()-d*mcgatherer.stddev() << ',' << mcgatherer.mean()+d*mcgatherer.stddev();
			cout << endl;
			n = mcgatherer.number_of_simulations(); 
			std::cerr << n << endl; 
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

TEST_F(HJMTest, MBinaryGaussianTest) 
{
	using std::cout;
	using std::endl;
	using std::flush;
	using std::cerr;

	int i;
	double mat,strike,diff;
	bool passed = true;
	double eps = 1e-9;
	int numeraire_index = 0;
	try 
	{
		//  option.csv worldbsf.csv 100 1000000000 0
		size_t minpaths = 100;
		size_t maxpaths = 1000000000;
		// Read data from files and create multicurrency term structure model
		std::ifstream is_inputs("option.csv");
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open option parameter CSV file");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		std::string str("Option expiry");
		if (inputs_map.count(str)) mat = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing option expiry"); 
		str = "Strike";
		if (inputs_map.count(str)) strike = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing strike"); 
		double strikeFX    = strike;
		double strikeQS    = strike;
		double strikeFBS   = strike;
		double strikeZCB   = strike;
		double strikeFZCB  = strike;
		double lvl         = strike;
		double strikeIS    = strike;
		double strikeIFX   = strike;
		double strikeIQS   = strike;
		double strikeIFBS  = strike;
		double strikeIZCB  = strike;
		double strikeIFZCB = strike;
		double Ilvl        = strike;
		GaussMarkovWorld world("worldbsf.csv");
		std::cerr << "Getting economy..." << endl;
		const std::vector<std::shared_ptr<GaussianEconomy> >& ecv = world.get_economies();
		const GaussianEconomy& ec = *(ecv[0]);
		std::cerr << "Number of assets: " << ec.underlying.size() << endl;
		std::cerr << "Getting asset..." << endl;
		std::shared_ptr<BlackScholesAssetAdapter> Sp(ec.underlying[0]);
		std::cerr << "...got pointer..." << endl;
		const BlackScholesAsset& S = Sp->GetOrigin();
		std::cerr << "...got asset." << endl;
		strike *= S.initial_value() / (*world.get_economies()[0]->initialTS)(mat);
		std::cerr << "Strike: " << strike << endl;
		double CFcall = world.get_economies()[0]->hjm->option(S,mat,strike);
		cout << "Closed form value: " << CFcall << endl;
		// Underlying is first asset in domestic economy
		Array<double,1> T(2);
		T = 0.0, mat;
		world.set_timeline(T);
		world.set_reporting(0,0);
		std::shared_ptr<AssetBinary> A_payoff(new AssetBinary(world,strike,0.0,mat,0));
		std::shared_ptr<BondBinary>  B_payoff(new BondBinary(world,strike,0.0,mat,0));
		MBinary A(world,*A_payoff);
		MBinary B(world,*B_payoff);
		double Avalue = A.price();
		double Bvalue = B.price();
		double MBCFcall = Avalue - strike*Bvalue;
		cout << "Closed form value using MBinary: " << MBCFcall << "   Diff: " << (diff = CFcall-MBCFcall) << endl;
		if (std::abs(diff)>eps) passed = false;
		double imat = mat/2.0;
		strikeIS *= S.initial_value() / (*world.get_economies()[0]->initialTS)(imat);
		std::cerr << "Strike: " << strikeIS << endl;
		strikeFX  *= world.get_forward_exchange_rate(1,mat);
		strikeIFX *= world.get_forward_exchange_rate(1,imat);
		//strikeFX = 0.0;
		double CFcallFX = world.get_economies()[0]->hjm->FXoption(world.get_initial_exchange_rates()(0),
			mat,
			strikeFX,
			*(world.get_economies()[1]->hjm),
			*(world.get_FXvolatilities()[0]));
		cout << "Closed form value for FX call: " << CFcallFX << endl;
		double CFcallIFX = world.get_economies()[0]->hjm->FXoption(world.get_initial_exchange_rates()(0),
			imat,
			strikeIFX,
			*(world.get_economies()[1]->hjm),
			*(world.get_FXvolatilities()[0]));
		cout << "Closed form value for intermediate maturity FX call: " << CFcallIFX << endl;
		// Underlying is foreign currency 1
		int reportable_FX_index = world.set_reporting(1,-2,mat);
		// Price option using MBinary
		std::shared_ptr<AssetBinary> A_payoffFX(new AssetBinary(world,strikeFX,0.0,mat,1));
		std::shared_ptr<BondBinary>  B_payoffFX(new BondBinary(world,strikeFX,0.0,mat,1));
		MBinary AFX(world,*A_payoffFX);
		MBinary BFX(world,*B_payoffFX);
		double AFXvalue = AFX.price();
		double BFXvalue = BFX.price();
		double MBCFcallFX = AFXvalue - strikeFX*BFXvalue;
		cout << "FX option closed form value using MBinary: " << MBCFcallFX << "   Diff: " << (diff = CFcallFX-MBCFcallFX) << endl;
		if (std::abs(diff)>eps) passed = false;
		int iZCBindex   = world.set_reporting(0,-1,mat);
		int iFZCBindex  = world.set_reporting(1,-1,mat);
		std::shared_ptr<MBinaryPayoff>  A_IFXpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,3,1));
		std::shared_ptr<MBinaryPayoff>  B_IFXpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,3,1));
		A_IFXpayoff->timeline = 0.0, imat;
		A_IFXpayoff->index    = reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1;      
		A_IFXpayoff->alpha    = 1.0, 1.0, -1.0;
		A_IFXpayoff->S        = 1.0;
		A_IFXpayoff->A        = 1.0, 1.0, -1.0;
		A_IFXpayoff->a        = strikeIFX;
		B_IFXpayoff->timeline = 0.0, imat;
		B_IFXpayoff->index    = reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1;      
		B_IFXpayoff->alpha    = 0.0;
		B_IFXpayoff->S        = 1.0;
		B_IFXpayoff->A        = 1.0, 1.0, -1.0;
		B_IFXpayoff->a        = strikeIFX;
		MBinary AIFX(world,*A_IFXpayoff);
		MBinary BIFX(world,*B_IFXpayoff);
		double AIFXvalue = AIFX.price();
		double BIFXvalue = BIFX.price();
		double MBCFcallIFX = AIFXvalue - strikeIFX*BIFXvalue;
		cout << "FX intermediate maturity option closed form value using MBinary: " << MBCFcallIFX << "   Diff: " << (diff = CFcallIFX-MBCFcallIFX) << endl;
		if (std::abs(diff)>eps) passed = false;
		// First asset in foreign economy
		int reportable_asset_index = world.set_reporting(1,0);
		//Array<int,1> reportable_asset_index(2);
		//reportable_asset_index = 1, 2; // Underlying is first asset in foreign economy, multiplied by exchange rate
		const GaussianEconomy& ecF = *(ecv[1]);
		std::cerr << "Number of assets: " << ecF.underlying.size() << endl;
		std::cerr << "Getting asset..." << endl;
		std::shared_ptr<BlackScholesAssetAdapter> SpF(ecF.underlying[0]);
		std::cerr << "...got pointer..." << endl;
		const BlackScholesAsset& SF = SpF->GetOrigin();
		strikeFBS  *= SF.initial_value() / (*world.get_economies()[1]->initialTS)(mat);
		strikeIFBS *= SF.initial_value() / (*world.get_economies()[1]->initialTS)(imat);
		//strikeFBS = 0.0;
		double CFcallFBS = world.get_economies()[1]->hjm->option(SF,mat,strikeFBS);
		cout << "Closed form value for foreign asset call option: " << CFcallFBS << endl;
		double fxspot = (world.get_initial_exchange_rates())(0);
		CFcallFBS *= fxspot;
		cout << "Closed form value converted to domestic currency: " << CFcallFBS << endl;
		double CFcallIFBS = world.get_economies()[1]->hjm->option(SF,imat,strikeIFBS);
		cout << "Closed form value for intermediate maturity foreign asset call option: " << CFcallIFBS << endl;
		CFcallIFBS *= fxspot;
		cout << "Closed form value converted to domestic currency: " << CFcallIFBS << endl;
		//strikeFBS *= fxspot;
		//strikeFBS *= world.get_forward_exchange_rate(1,mat);
		//Array<double,1> alpha(2);
		//alpha = 1.0, 0.0;
		//AssetProductBinary A_payoffFBS(world,strikeFBS,0.0,mat,reportable_asset_index);
		//AssetProductBinary B_payoffFBS(world,strikeFBS,0.0,mat,reportable_asset_index,alpha);
		std::shared_ptr<ForeignOption> A_payoffFBS(new ForeignOption(world,strikeFBS,0.0,mat,reportable_asset_index,reportable_FX_index,true));
		std::shared_ptr<ForeignOption> B_payoffFBS(new ForeignOption(world,strikeFBS,0.0,mat,reportable_asset_index,reportable_FX_index,false));
		MBinary AFBS(world,*A_payoffFBS);
		MBinary BFBS(world,*B_payoffFBS);
		double AFBSvalue = AFBS.price();
		double BFBSvalue = BFBS.price();
		double MBCFcallFBS = AFBSvalue - strikeFBS*BFBSvalue;
		cout << "Foreign asset option closed form value using MBinary: " << MBCFcallFBS << "   Diff: " << (diff = CFcallFBS-MBCFcallFBS) << endl;
		if (std::abs(diff)>eps) passed = false;
		double IFBSdiv = SF.dividend_discount(imat,mat);
		std::shared_ptr<MBinaryPayoff>  A_IFBSpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,4,1));
		std::shared_ptr<MBinaryPayoff>  B_IFBSpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,4,1));
		A_IFBSpayoff->timeline = 0.0, imat;
		A_IFBSpayoff->index    = reportable_asset_index, reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1, 1;      
		A_IFBSpayoff->alpha    = 1.0, 1.0, 1.0, 0.0;
		A_IFBSpayoff->S        = 1.0;
		A_IFBSpayoff->A        = 1.0, 0.0, 0.0, 1.0;
		A_IFBSpayoff->a        = IFBSdiv*strikeIFBS;
		B_IFBSpayoff->timeline = 0.0, imat;
		B_IFBSpayoff->index    = reportable_asset_index, reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1, 1;       
		B_IFBSpayoff->alpha    = 0.0, 1.0, 1.0, -1.0;
		B_IFBSpayoff->S        = 1.0;
		B_IFBSpayoff->A        = 1.0, 0.0, 0.0, 1.0;
		B_IFBSpayoff->a        = IFBSdiv*strikeIFBS;
		MBinary AIFBS(world,*A_IFBSpayoff);
		MBinary BIFBS(world,*B_IFBSpayoff);
		double AIFBSvalue = AIFBS.price();
		double BIFBSvalue = BIFBS.price();
		double MBCFcallIFBS = (AIFBSvalue - IFBSdiv*strikeIFBS*BIFBSvalue)/IFBSdiv;
		cout << "Foreign asset intermediate maturity option closed form value using MBinary: " << MBCFcallIFBS << "   Diff: " << (diff = CFcallIFBS-MBCFcallIFBS) << endl;
		if (std::abs(diff)>eps) passed = false;
		// Call option on domestic zero coupon bond
		double bondmat = 2.5*mat;
		strikeZCB  *= (*world.get_economies()[0]->initialTS)(bondmat) / (*world.get_economies()[0]->initialTS)(mat);
		strikeIZCB *= (*world.get_economies()[0]->initialTS)(bondmat) / (*world.get_economies()[0]->initialTS)(imat);
		//strikeZCB = 0.0;
		double CFZCBcall = world.get_economies()[0]->hjm->ZCBoption(mat,bondmat,strikeZCB);
		cout << "Closed form call on domestic zero coupon bond: " << CFZCBcall << endl;
		double CFIZCBcall = world.get_economies()[0]->hjm->ZCBoption(imat,bondmat,strikeIZCB);
		cout << "Closed form intermediate maturity call on domestic zero coupon bond: " << CFIZCBcall << endl;
		int zcbidx = world.set_reporting(0,-1,bondmat);
		// Price option using MBinary
		std::shared_ptr<AssetBinary> A_ZCBpayoff(new AssetBinary(world,strikeZCB,0.0,mat,zcbidx));
		std::shared_ptr<BondBinary>  B_ZCBpayoff(new BondBinary(world,strikeZCB,0.0,mat,zcbidx));
		MBinary AZCB(world,*A_ZCBpayoff);
		MBinary BZCB(world,*B_ZCBpayoff);
		double AZCBvalue = AZCB.price();
		double BZCBvalue = BZCB.price();
		double MBCFZCBcall = AZCBvalue - strikeZCB*BZCBvalue;
		cout << "Closed form value using MBinary: " << MBCFZCBcall << "   Diff: " << (diff = CFZCBcall-MBCFZCBcall) << endl;
		if (std::abs(diff)>eps) passed = false;
		std::shared_ptr<AssetBinary> A_IZCBpayoff(new AssetBinary(world,strikeIZCB,0.0,imat,zcbidx));
		std::shared_ptr<BondBinary>  B_IZCBpayoff(new BondBinary(world,strikeIZCB,0.0,imat,zcbidx));
		MBinary AIZCB(world,*A_IZCBpayoff);
		MBinary BIZCB(world,*B_IZCBpayoff);
		double AIZCBvalue = AIZCB.price();
		double BIZCBvalue = BIZCB.price();
		double MBCFIZCBcall = AIZCBvalue - strikeIZCB*BIZCBvalue;
		cout << "Closed form intermediate maturity value using MBinary: " << MBCFIZCBcall << "   Diff: " << (diff = CFIZCBcall-MBCFIZCBcall) << endl;
		if (std::abs(diff)>eps) passed = false;
		// Call option on foreign zero coupon bond
		strikeFZCB  *= (*world.get_economies()[1]->initialTS)(bondmat) / (*world.get_economies()[1]->initialTS)(mat);
		strikeIFZCB *= (*world.get_economies()[1]->initialTS)(bondmat) / (*world.get_economies()[1]->initialTS)(imat);
		//strikeFZCB = 0.0;
		double CFFZCBcall = world.get_economies()[1]->hjm->ZCBoption(mat,bondmat,strikeFZCB);
		cout << "Closed form call on foreign zero coupon bond: " << CFFZCBcall << endl;
		CFFZCBcall *= fxspot;
		cout << "Price in domestic currency: " << CFFZCBcall << endl;
		double CFIFZCBcall = world.get_economies()[1]->hjm->ZCBoption(imat,bondmat,strikeIFZCB);
		cout << "Closed form intermediate maturity call on foreign zero coupon bond: " << CFIFZCBcall << endl;
		CFIFZCBcall *= fxspot;
		cout << "Price in domestic currency: " << CFIFZCBcall << endl;
		int fzcbidx = world.set_reporting(1,-1,bondmat);
		// Price option using MBinary
		std::shared_ptr<ForeignOption> A_FZCBpayoff(new ForeignOption(world,strikeFZCB,0.0,mat,fzcbidx,reportable_FX_index,true));
		std::shared_ptr<ForeignOption> B_FZCBpayoff(new ForeignOption(world,strikeFZCB,0.0,mat,fzcbidx,reportable_FX_index,false));
		MBinary AFZCB(world,*A_FZCBpayoff);
		MBinary BFZCB(world,*B_FZCBpayoff);
		double AFZCBvalue = AFZCB.price();
		double BFZCBvalue = BFZCB.price();
		double MBCFcallFZCB = AFZCBvalue - strikeFZCB*BFZCBvalue;
		cout << "Foreign zero coupon bond option closed form value using MBinary: " << MBCFcallFZCB << "   Diff: " << (diff = CFFZCBcall-MBCFcallFZCB) << endl;
		if (std::abs(diff)>eps) passed = false;
		std::shared_ptr<MBinaryPayoff>  A_IFZCBpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,4,1));
		std::shared_ptr<MBinaryPayoff>  B_IFZCBpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,4,1));
		A_IFZCBpayoff->timeline = 0.0, imat;
		A_IFZCBpayoff->index    = fzcbidx, reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1, 1;      
		A_IFZCBpayoff->alpha    = 1.0, 1.0, 1.0, -1.0;
		A_IFZCBpayoff->S        = 1.0;
		A_IFZCBpayoff->A        = 1.0, 0.0, 0.0, 0.0;
		A_IFZCBpayoff->a        = strikeIFZCB;
		B_IFZCBpayoff->timeline = 0.0, imat;
		B_IFZCBpayoff->index    = fzcbidx, reportable_FX_index, iZCBindex, iFZCBindex,
			1, 1, 1, 1;       
		B_IFZCBpayoff->alpha    = 0.0, 1.0, 1.0, -1.0;
		B_IFZCBpayoff->S        = 1.0;
		B_IFZCBpayoff->A        = 1.0, 0.0, 0.0, 0.0;
		B_IFZCBpayoff->a        = strikeIFZCB;
		MBinary AIFZCB(world,*A_IFZCBpayoff);
		MBinary BIFZCB(world,*B_IFZCBpayoff);
		double AIFZCBvalue = AIFZCB.price();
		double BIFZCBvalue = BIFZCB.price();
		double MBCFcallIFZCB = AIFZCBvalue - strikeIFZCB*BIFZCBvalue;
		cout << "Foreign zero coupon bond intermediate maturity option closed form value using MBinary: " << MBCFcallIFZCB << "   Diff: " << (diff = CFIFZCBcall-MBCFcallIFZCB) << endl;
		if (std::abs(diff)>eps) passed = false;
		// Quanto caplet
		double delta = 0.75;
		lvl *= (*world.get_economies()[1]->initialTS).simple_rate(mat,delta);
		Ilvl *= (*world.get_economies()[1]->initialTS).simple_rate(imat,delta);
		double CFquanto = world.get_economies()[0]->hjm->QuantoCaplet(fxspot,mat,delta,lvl,*world.get_economies()[1]->hjm,*((world.get_FXvolatilities())[0]));
		cout << "Closed form value for quanto caplet: " << CFquanto << endl;
		double CFquanto_old = world.get_economies()[0]->hjm->QuantoCaplet_old(fxspot,mat,delta,lvl,*world.get_economies()[1]->hjm,*((world.get_FXvolatilities())[0]));
		cout << "Closed form value for quanto caplet: " << CFquanto_old << endl;
		double CFIquanto = world.get_economies()[0]->hjm->QuantoCaplet(fxspot,imat,delta,Ilvl,*world.get_economies()[1]->hjm,*((world.get_FXvolatilities())[0]));
		cout << "Closed form value for intermediate maturity quanto caplet: " << CFIquanto << endl;
		// Price option using MBinary
		int Qidx  = world.set_reporting(0,-1,mat+delta);
		int QFidx = world.set_reporting(1,-1,mat+delta);
		double strikeQ = 1.0+lvl*delta;
		std::shared_ptr<MBinaryPayoff>  A_Qpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,2,1));
		std::shared_ptr<MBinaryPayoff>  B_Qpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,2,1));
		A_Qpayoff->timeline = 0.0, mat;
		A_Qpayoff->index    = Qidx, QFidx,
			1, 1;      
		A_Qpayoff->alpha    = 1.0, -1.0;
		A_Qpayoff->S        = 1.0;
		A_Qpayoff->A        = 0.0, -1.0;
		A_Qpayoff->a        = strikeQ;
		B_Qpayoff->timeline = 0.0, mat;
		B_Qpayoff->index    = Qidx, QFidx,
			1, 1;      
		B_Qpayoff->alpha    = 1.0, 0.0;
		B_Qpayoff->S        = 1.0;
		B_Qpayoff->A        = 0.0, -1.0;
		B_Qpayoff->a        = strikeQ;
		MBinary Aquanto(world,*A_Qpayoff);
		MBinary Bquanto(world,*B_Qpayoff);
		double AQvalue = Aquanto.price();
		double BQvalue = Bquanto.price();
		double MBCFquanto = AQvalue - strikeQ*BQvalue;
		MBCFquanto *= fxspot;
		cout << "Closed form value using MBinary: " << MBCFquanto << "   Diff: " << (diff = CFquanto-MBCFquanto) << endl;
		diff /= CFquanto;
		if (std::abs(diff)>eps) passed = false;
		int IQidx  = world.set_reporting(0,-1,imat+delta);
		int IQFidx = world.set_reporting(1,-1,imat+delta);
		double strikeIQ = 1.0+Ilvl*delta;
		std::shared_ptr<MBinaryPayoff>  A_IQpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,2,1));
		std::shared_ptr<MBinaryPayoff>  B_IQpayoff(new MBinaryPayoff(*world.get_economies()[0]->initialTS,2,2,1));
		A_IQpayoff->timeline = 0.0, imat;
		A_IQpayoff->index    = IQidx, IQFidx,
			1, 1;      
		A_IQpayoff->alpha    = 1.0, -1.0;
		A_IQpayoff->S        = 1.0;
		A_IQpayoff->A        = 0.0, -1.0;
		A_IQpayoff->a        = strikeIQ;
		B_IQpayoff->timeline = 0.0, imat;
		B_IQpayoff->index    = IQidx, IQFidx,
			1, 1;      
		B_IQpayoff->alpha    = 1.0, 0.0;
		B_IQpayoff->S        = 1.0;
		B_IQpayoff->A        = 0.0, -1.0;
		B_IQpayoff->a        = strikeIQ;
		MBinary AIquanto(world,*A_IQpayoff);
		MBinary BIquanto(world,*B_IQpayoff);
		double AIQvalue = AIquanto.price();
		double BIQvalue = BIquanto.price();
		double MBCFIquanto = AIQvalue - strikeIQ*BIQvalue;
		MBCFIquanto *= fxspot;
		cout << "Closed form intermediate maturity value using MBinary: " << MBCFIquanto << "   Diff: " << (diff = CFIquanto-MBCFIquanto) << endl;
		diff /= CFIquanto;
		if (std::abs(diff)>eps) passed = false;
		double CFcallIS = world.get_economies()[0]->hjm->option(S,imat,strikeIS);
		cout << "Closed form value of intermediate maturity option: " << CFcallIS << endl;
		// Price intermediate maturity option using MBinary
		// MC works for this one. 
		// int iZCBindex  = world.set_reporting(0,-1,mat);
		double IBSdiv = S.dividend_discount(imat,mat);
		Array<int,1> product_index(2);
		product_index = 0, iZCBindex;
		std::shared_ptr<AssetProductBinary> A_Ipayoff(new AssetProductBinary(world,strikeIS*IBSdiv,0.0,imat,product_index));
		std::shared_ptr<BondProductBinary>  B_Ipayoff(new BondProductBinary(world,strikeIS*IBSdiv,0.0,imat,product_index));
		MBinary AIS(world,*A_Ipayoff);
		MBinary BIS(world,*B_Ipayoff);
		double AISvalue = AIS.price();
		double BISvalue = BIS.price();
		double MBCFcallIS = (AISvalue - IBSdiv*strikeIS*BISvalue)/IBSdiv;
		cout << "Closed form value using MBinary: " << MBCFcallIS << "   Diff: " << (diff = CFcallIS-MBCFcallIS) << endl;
		if (std::abs(diff)>eps) passed = false;
		// permutate
		product_index = iZCBindex, 0;
		std::shared_ptr<AssetProductBinary> A_IPpayoff(new AssetProductBinary(world,IBSdiv*strikeIS,0.0,imat,product_index));
		std::shared_ptr<BondProductBinary>  B_IPpayoff(new BondProductBinary(world,IBSdiv*strikeIS,0.0,imat,product_index));
		MBinary AIPS(world,*A_IPpayoff);
		MBinary BIPS(world,*B_IPpayoff);
		AISvalue = AIPS.price();
		BISvalue = BIPS.price();
		double MBCFcallIPS = (AISvalue - IBSdiv*strikeIS*BISvalue)/IBSdiv;
		cout << "Closed form value using MBinary: " << MBCFcallIPS << "   Diff: " << (diff = CFcallIS-MBCFcallIPS) << endl;
		if (std::abs(diff)>eps) passed = false;
		if (passed) cout << "************ PASSED ************" << endl;
		else        cout << "************ FAILED ************" << endl;
		if (passed) cerr << "************ PASSED ************" << endl;
		else        cerr << "************ FAILED ************" << endl;

		// Price asset quanto option using MBinary
		int QSidx = reportable_asset_index;
		strikeQS *= SF.initial_value() / (*world.get_economies()[1]->initialTS)(mat);
		std::shared_ptr<AssetBinary> A_QSpayoff(new AssetBinary(world,strikeQS,0.0,mat,QSidx));
		std::shared_ptr<BondBinary>  B_QSpayoff(new BondBinary(world,strikeQS,0.0,mat,QSidx));
		MBinary ASquanto(world,*A_QSpayoff);
		MBinary BSquanto(world,*B_QSpayoff);
		double AQSvalue = ASquanto.price();
		double BQSvalue = BSquanto.price();
		double MBCFSquanto = AQSvalue - strikeQS*BQSvalue;
		MBCFSquanto *= fxspot;
		cout << "Closed form value for asset quanto option using MBinary: " << MBCFSquanto << endl;
		// Price quanto zero coupon bond option using MBinary
		std::shared_ptr<AssetBinary> A_QZCBpayoff(new AssetBinary(world,strikeFZCB,0.0,mat,fzcbidx));
		std::shared_ptr<BondBinary>  B_QZCBpayoff(new BondBinary(world,strikeFZCB,0.0,mat,fzcbidx));
		MBinary AZCBquanto(world,*A_QZCBpayoff);
		MBinary BZCBquanto(world,*B_QZCBpayoff);
		double AQZCBvalue = AZCBquanto.price();
		double BQZCBvalue = BZCBquanto.price();
		double MBCFZCBquanto = AQZCBvalue - strikeFZCB*BQZCBvalue;
		MBCFZCBquanto *= fxspot;
		cout << "Closed form value for quanto zero coupon bond option using MBinary: " << MBCFZCBquanto << endl;
		// Call option on the domestic geometric average
		int asian_n = 11;
		Array<double,1> asianT(asian_n);
		double dt = mat/(asian_n-1.0);
		firstIndex idx;
		asianT = idx*dt;
		double asianK = std::sqrt(strike*S.initial_value());
		exotics::DiscreteGeometricMeanFixedStrike asian_option(world,asianT,asianK,0,0,1.0);
		double MBCFasian = asian_option.price();
		cout << "Closed form value for call option on the domestic geometric average using MBinary: " << MBCFasian << endl;
		// Call option on the foreign geometric average
		double asianFK = std::sqrt(strikeFBS*SF.initial_value());
		exotics::DiscreteGeometricMeanFixedStrike asianF_option(world,asianT,asianFK,reportable_asset_index,0,1.0);
		double MBCFasianF = asianF_option.price();
		cout << "Closed form value for call option on the foreign geometric average using MBinary: " << MBCFasianF << endl;

		// Monte Carlo
		int number_of_options = 17;
		i = 0;
		Array<double,1> CFvalues(number_of_options);
		Array<double,1> MBCFvalues(number_of_options);
		Array<std::string,1> labels(number_of_options);
		ranlib::NormalUnit<double> normalRNG;
		MCPayoffList mclist;
		mclist.push_back(asian_option.get_payoff());
		CFvalues(i) = MBCFasian;
		MBCFvalues(i) = MBCFasian;
		labels(i) = "Domestic DiscreteGeometricMeanFixedStrike";
		i++;
		std::shared_ptr<BondBinary> ZCBpayoff(new BondBinary(Sp,0.0,0.0,mat,*world.get_economies()[0]->initialTS));
		mclist.push_back(ZCBpayoff);
		MBCFvalues(i) = CFvalues(i) = (*world.get_economies()[0]->initialTS)(mat);
		labels(i) = "ZCB";
		i++;
		std::shared_ptr<MCPayoffList> calloption(new MCPayoffList);
		calloption->push_back(A_payoff);
		calloption->push_back(B_payoff,-strike);
		mclist.push_back(calloption);
		CFvalues(i) = CFcall;
		MBCFvalues(i) = MBCFcall;
		labels(i) = "Call on domestic asset";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIS(new MCPayoffList);
		calloptionIS->push_back(A_Ipayoff);
		calloptionIS->push_back(B_Ipayoff,-strikeIS*IBSdiv);
		mclist.push_back(calloptionIS,1.0/IBSdiv);
		CFvalues(i) = CFcallIS;
		MBCFvalues(i) = MBCFcallIS;
		labels(i) = "Call on domestic asset - intermediate maturity";
		i++;
		std::shared_ptr<MCPayoffList> calloptionFX(new MCPayoffList);
		calloptionFX->push_back(A_payoffFX);
		calloptionFX->push_back(B_payoffFX,-strikeFX);
		mclist.push_back(calloptionFX);
		CFvalues(i) = CFcallFX;
		MBCFvalues(i) = MBCFcallFX;
		labels(i) = "FX call";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIFX(new MCPayoffList);
		calloptionIFX->push_back(A_IFXpayoff);
		calloptionIFX->push_back(B_IFXpayoff,-strikeIFX);
		mclist.push_back(calloptionIFX);
		CFvalues(i) = CFcallIFX;
		MBCFvalues(i) = MBCFcallIFX;
		labels(i) = "FX intermediate maturity call";
		i++;
		std::shared_ptr<MCPayoffList> calloptionFBS(new MCPayoffList);
		calloptionFBS->push_back(A_payoffFBS);
		calloptionFBS->push_back(B_payoffFBS,-strikeFBS);
		mclist.push_back(calloptionFBS);
		CFvalues(i) = CFcallFBS;
		MBCFvalues(i) = MBCFcallFBS;
		labels(i) = "Call on foreign asset";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIFBS(new MCPayoffList);
		calloptionIFBS->push_back(A_IFBSpayoff);
		calloptionIFBS->push_back(B_IFBSpayoff,-strikeIFBS*IFBSdiv);
		mclist.push_back(calloptionIFBS,1.0/IFBSdiv);
		CFvalues(i) = CFcallIFBS;
		MBCFvalues(i) = MBCFcallIFBS;
		labels(i) = "Foreign asset intermediate maturity call";
		i++;
		std::shared_ptr<MCPayoffList> calloptionZCB(new MCPayoffList);
		calloptionZCB->push_back(A_ZCBpayoff);
		calloptionZCB->push_back(B_ZCBpayoff,-strikeZCB);
		mclist.push_back(calloptionZCB);
		CFvalues(i) = CFZCBcall;
		MBCFvalues(i) = MBCFZCBcall;
		labels(i) = "Call on domestic ZCB";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIZCB(new MCPayoffList);
		calloptionIZCB->push_back(A_IZCBpayoff);
		calloptionIZCB->push_back(B_IZCBpayoff,-strikeIZCB);
		mclist.push_back(calloptionIZCB);
		CFvalues(i) = CFIZCBcall;
		MBCFvalues(i) = MBCFIZCBcall;
		labels(i) = "Intermediate maturity call on domestic ZCB";
		i++;
		std::shared_ptr<MCPayoffList> calloptionFZCB(new MCPayoffList);
		calloptionFZCB->push_back(A_FZCBpayoff);
		calloptionFZCB->push_back(B_FZCBpayoff,-strikeFZCB);
		mclist.push_back(calloptionFZCB);
		CFvalues(i) = CFFZCBcall;
		MBCFvalues(i) = MBCFcallFZCB;
		labels(i) = "Call on foreign ZCB";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIFZCB(new MCPayoffList);
		calloptionIFZCB->push_back(A_IFZCBpayoff);
		calloptionIFZCB->push_back(B_IFZCBpayoff,-strikeIFZCB);
		mclist.push_back(calloptionIFZCB);
		CFvalues(i) = CFIFZCBcall;
		MBCFvalues(i) = MBCFcallIFZCB;
		labels(i) = "Intermediate maturity call on foreign ZCB";
		i++;
		std::shared_ptr<MCPayoffList> calloptionQ(new MCPayoffList);
		calloptionQ->push_back(A_Qpayoff);
		calloptionQ->push_back(B_Qpayoff,-strikeQ);
		mclist.push_back(calloptionQ,fxspot);
		CFvalues(i) = CFquanto;
		MBCFvalues(i) = MBCFquanto;
		labels(i) = "Quanto caplet";
		i++;
		std::shared_ptr<MCPayoffList> calloptionIQ(new MCPayoffList);
		calloptionIQ->push_back(A_IQpayoff);
		calloptionIQ->push_back(B_IQpayoff,-strikeIQ);
		mclist.push_back(calloptionIQ,fxspot);
		CFvalues(i) = CFIquanto;
		MBCFvalues(i) = MBCFIquanto;
		labels(i) = "Intermediate maturity quanto caplet";
		i++;
		std::shared_ptr<MCPayoffList> calloptionQS(new MCPayoffList);
		calloptionQS->push_back(A_QSpayoff);
		calloptionQS->push_back(B_QSpayoff,-strikeQS);
		mclist.push_back(calloptionQS,fxspot);
		CFvalues(i) = MBCFSquanto;
		MBCFvalues(i) = MBCFSquanto;
		labels(i) = "Foreign asset quanto";
		i++;
		std::shared_ptr<MCPayoffList> calloptionQZCB(new MCPayoffList);
		calloptionQZCB->push_back(A_QZCBpayoff);
		calloptionQZCB->push_back(B_QZCBpayoff,-strikeFZCB);
		mclist.push_back(calloptionQZCB,fxspot);
		CFvalues(i) = MBCFZCBquanto;
		MBCFvalues(i) = MBCFZCBquanto;
		labels(i) = "Foreign ZCB quanto";
		i++;
		mclist.push_back(asianF_option.get_payoff());
		CFvalues(i) = MBCFasianF;
		MBCFvalues(i) = MBCFasianF;
		labels(i) = "Foreign DiscreteGeometricMeanFixedStrike";
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(mclist,world,*(world.get_economies()[0]->initialTS),numeraire_index);
		std::function<Array<double,1> (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mappingArray,&mc_mapping,std::placeholders::_1);
		//std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		std::cerr << "Number of factors: " << world.factors() << endl;
		std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		RandomArray<ranlib::NormalUnit<double>,double> random_container(normalRNG,world.factors(),world.number_of_steps()); 
		MCGeneric<Array<double,2>,Array<double,1>,RandomArray<ranlib::NormalUnit<double>,double> > mc(func,random_container);
		//MCGatherer<double> mcgatherer;
		MCGatherer<Array<double,1> > mcgatherer(number_of_options);
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal,0.95);
		for (i=0;i<number_of_options;i++) cout << "," << labels(i) << "," << CFvalues(i) << ",,,";
		cout << endl << "Paths";
		for (i=0;i<number_of_options;i++) cout << ",MC value,95% CI lower bound,95% CI upper bound,Std error,Std error against MBinary";
		cout << endl;
		while (mcgatherer.number_of_simulations()<maxpaths) 
		{
			mc.simulate(mcgatherer,n);
			Array<double,1> mean(mcgatherer.mean());
			Array<double,1> stddev(mcgatherer.stddev());
			cout << mcgatherer.number_of_simulations();
			bool MCpassed = true;
			for (i=0;i<number_of_options;i++) 
			{
				double stderror = (mean(i)-CFvalues(i))/stddev(i);
				double stderrorMB = (mean(i)-MBCFvalues(i))/stddev(i);
				cout << ',' << mean(i) << ',' << mean(i)-d*stddev(i) << ',' << mean(i)+d*stddev(i);
				cout << ',' << std::abs(stderror) << ',' << std::abs(stderrorMB);
				if (std::max(std::abs(stderror),std::abs(stderrorMB))>2.0) MCpassed = false; 
			}
			if (MCpassed) cout << ",PASSED";
			else          cout << ",FAILED";
			cout << endl; 
			n = mcgatherer.number_of_simulations(); 
			std::cerr << n << ' ' << std::flush;
			if (MCpassed) std::cerr << "  PASSED" << endl;
			else          std::cerr << "  FAILED" << endl; 
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

TEST_F(HJMTest, MCBondOpt) 
{
	/// mcbondopt 10 0.05 0.03 1.5 1 3 100 1000000000 0.1

	int i;
	try
	{
		double bondmat = 10.0;
		double r = 0.05;
		double sgm = 0.03;
		double mat = 1.5;
		double K = 1.0;
		int N = 10;
		size_t minpaths = 100;
		size_t maxpaths = 1000000000;
		double mean_reversion = 0.1;
		cout << bondmat << ' ' << r << ' ' << sgm << ' ' << mat << ' ' << K << ' ' << N << ' ' << minpaths << ' ' << maxpaths;
		cout << ' ' << mean_reversion << endl;
		int numeraire_index = 0;
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		std::shared_ptr<DeterministicAssetVol> hjmvol(new ExponentialVol(sgm,mean_reversion));
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		std::shared_ptr<TermStructure> ts(new FlatTermStructure(r,0.0,bondmat+10.0));
		double strike = K * (*ts)(bondmat)/(*ts)(mat);
		std::shared_ptr<GaussianEconomy> domestic_economy(new GaussianEconomy(underlying,hjmvol,ts));
		std::vector<std::shared_ptr<GaussianEconomy> > economies;
		economies.push_back(domestic_economy);
		double CFcall = domestic_economy->hjm->ZCBoption(mat,bondmat,strike);
		cout << "Closed form call: " << CFcall << endl;
		std::vector<std::shared_ptr<DeterministicAssetVol> > xvols;
		Array<double,1> initial_exchange_rates(0);
		GaussMarkovWorld world(economies,xvols,initial_exchange_rates);
		world.set_timeline(T);
		world.set_reporting(0,-1,bondmat);
		ranlib::NormalUnit<double> normalRNG;
		RandomArray<ranlib::NormalUnit<double>,double> random_container(normalRNG,world.factors(),world.number_of_steps()); 
		MCEuropeanCall callpayoff(T(0),mat,0,strike);
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(callpayoff,world,*ts,numeraire_index);
		std::function<double (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mapping,&mc_mapping,std::placeholders::_1);
		MCGeneric<Array<double,2>,double,RandomArray<ranlib::NormalUnit<double>,double> > mc(func,random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal,0.95);
		cout << "Number of time steps: " << world.number_of_steps() << endl;
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,Std error" << endl;
		while (mcgatherer.number_of_simulations()<maxpaths)
		{
			mc.simulate(mcgatherer,n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean()-d*mcgatherer.stddev() << ',' << mcgatherer.mean()+d*mcgatherer.stddev();
			cout << ',' << (mcgatherer.mean()-CFcall)/mcgatherer.stddev() << endl;
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

TEST_F(HJMTest, MCBSForeignOpt) 
{
	int i;
	double mat,strike;
	// option.csv worldbsf.csv 100 100000000
	try 
	{
		size_t minpaths = 100;
		size_t maxpaths = 100000000;
		int N = 10;
		// Read data from files and create multicurrency term structure model
		std::ifstream is_inputs("option.csv");
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open option parameter CSV file");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		std::string str("Option expiry");
		if (inputs_map.count(str)) mat = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing option expiry"); 
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		str = "Strike";
		if (inputs_map.count(str)) strike = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing strike"); 
		GaussMarkovWorld world("worldbsf.csv");
		std::cerr << "Getting economy..." << endl;
		const std::vector<std::shared_ptr<GaussianEconomy> >& ecv = world.get_economies();
		const GaussianEconomy& ec = *(ecv[1]);
		std::cerr << "Number of assets: " << ec.underlying.size() << endl;
		std::cerr << "Getting asset..." << endl;
		std::shared_ptr<BlackScholesAssetAdapter> Sp(ec.underlying[0]);
		std::cerr << "...got pointer..." << endl;
		const BlackScholesAsset& S = Sp->GetOrigin();
		std::cerr << "...got asset." << endl;
		strike *= S.initial_value() / (*world.get_economies()[1]->initialTS)(mat);
		std::cerr << "Strike: " << strike << endl;
		double CFcall = world.get_economies()[1]->hjm->option(S,mat,strike);
		cout << "Closed form value: " << CFcall << endl;
		double fxspot = (world.get_initial_exchange_rates())(0);
		CFcall *= fxspot;
		cout << "Closed form value converted to domestic currency: " << CFcall << endl;

		/// construct a adapter for the domestic fx BlackScholes class.
		std::shared_ptr<IExchangeRateValue> fxObj = TestUtil::getExchangeRateValue("dom", "fx1");
		fxObj->SetTradePrice(fxspot);

		std::shared_ptr<DeterministicAssetVol> fxVol = *(world.get_FXvolatilities().begin());
		std::shared_ptr<BlackScholesAssetAdapter> fxAdapter = \
			std::make_shared<BlackScholesAssetAdapter>(fxObj, fxVol);		

		world.set_timeline(T);
		std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		// Underlying is first asset in foreign economy
		world.set_reporting(1,0);
		// Report exchange rate
		world.set_reporting(1,-2,mat);
		int numeraire_index = 0;
		ranlib::NormalUnit<double> normalRNG;
		// Payoff is converted to domestic currency - thus need MBinary
		std::shared_ptr<ForeignOption> callpayoffasset(new ForeignOption(Sp,fxAdapter,strike,0.0,mat,*world.get_economies()[1]->initialTS));
		std::shared_ptr<ForeignOption> callpayoffstrike(new ForeignOption(Sp,fxAdapter,strike,0.0,mat,*world.get_economies()[1]->initialTS,false));
		MCPayoffList mclist;
		mclist.push_back(callpayoffasset);
		mclist.push_back(callpayoffstrike,-strike);
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(mclist,world,*(world.get_economies()[0]->initialTS),numeraire_index);
		std::function<double (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mapping,&mc_mapping,std::placeholders::_1);
		std::cerr << "Number of factors: " << world.factors() << endl;
		RandomArray<ranlib::NormalUnit<double>,double> random_container(normalRNG,world.factors(),world.number_of_steps()); 
		MCGeneric<Array<double,2>,double,RandomArray<ranlib::NormalUnit<double>,double> > mc(func,random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal,0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,Std error" << endl;
		while (mcgatherer.number_of_simulations()<maxpaths)
		{
			mc.simulate(mcgatherer,n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean()-d*mcgatherer.stddev() << ',' << mcgatherer.mean()+d*mcgatherer.stddev();
			cout << ',' << (mcgatherer.mean()-CFcall)/mcgatherer.stddev() << endl;
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

TEST_F(HJMTest, MCBSOpt) 
{
	/// option.csv worldbsf.csv 100 100000000
	int i;
	double mat,strike;
	int numeraire_index = 0;  
	try 
	{
		size_t minpaths = 100;
		size_t maxpaths = 100;
		int N = 10;
		// Read data from files and create multicurrency term structure model
		std::ifstream is_inputs("option.csv");
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open option parameter CSV file");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		std::string str("Option expiry");
		if (inputs_map.count(str)) mat = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing option expiry"); 
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		str = "Strike";
		if (inputs_map.count(str)) strike = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing strike"); 
		GaussMarkovWorld world("worldbsf.csv");
		std::cerr << "Getting economy..." << endl;
		const std::vector<std::shared_ptr<GaussianEconomy> >& ecv = world.get_economies();
		const GaussianEconomy& ec = *(ecv[0]);
		std::cerr << "Number of assets: " << ec.underlying.size() << endl;
		std::cerr << "Getting asset..." << endl;
		std::shared_ptr<BlackScholesAssetAdapter> Sp(ec.underlying[0]);
		std::cerr << "...got pointer..." << endl;
		const BlackScholesAsset& S = Sp->GetOrigin();
		std::cerr << "...got asset." << endl;
		strike *= S.initial_value() / (*world.get_economies()[0]->initialTS)(mat);
		std::cerr << "Strike: " << strike << endl;
		double CFcall = world.get_economies()[0]->hjm->option(S,mat,strike);
		cout << "Closed form value: " << CFcall << endl;
		world.set_timeline(T);
		std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		// Underlying is first asset in domestic economy
		world.set_reporting(0,0);
		world.set_reporting(0,1);
		ranlib::NormalUnit<double> normalRNG;
		MCEuropeanCall callpayoff(T(0),mat,0,strike);
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(callpayoff,world,*(world.get_economies()[0]->initialTS),numeraire_index);
		std::function<double (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mapping,&mc_mapping,std::placeholders::_1);
		std::cerr << "Number of factors: " << world.factors() << endl;
		RandomArray<ranlib::NormalUnit<double>,double> random_container(normalRNG,world.factors(),world.number_of_steps()); 
		MCGeneric<Array<double,2>,double,RandomArray<ranlib::NormalUnit<double>,double> > mc(func,random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal,0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,Std error" << endl;
		while (mcgatherer.number_of_simulations()<maxpaths) 
		{
			mc.simulate(mcgatherer,n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean()-d*mcgatherer.stddev() << ',' << mcgatherer.mean()+d*mcgatherer.stddev();
			cout << ',' << (mcgatherer.mean()-CFcall)/mcgatherer.stddev() << endl;
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

TEST_F(HJMTest, MCFXOpt) 
{
	/// option.csv worldbsf.csv 100 1000000
	int i;
	double mat,strike;
	try 
	{
		size_t minpaths = 100;
		size_t maxpaths = 100;
		int N = 10;
		// Read data from files and create multicurrency term structure model
		std::ifstream is_inputs("option.csv");
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open option parameter CSV file");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		std::string str("Option expiry");
		if (inputs_map.count(str)) mat = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing option expiry"); 
		Array<double,1> T(N+1);
		firstIndex idx;
		double dt = mat/N;
		T = idx*dt;
		str = "Strike";
		if (inputs_map.count(str)) strike = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing strike"); 
		GaussMarkovWorld world("worldbsf.csv");
		world.set_timeline(T);
		std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		strike *= world.get_forward_exchange_rate(1,mat);
		double CFcall = world.get_economies()[0]->hjm->FXoption(world.get_initial_exchange_rates()(0),
			mat,
			strike,
			*(world.get_economies()[1]->hjm),
			*(world.get_FXvolatilities()[0]));
		cout << "Closed form value: " << CFcall << endl;
		// Underlying is foreign currency 1, i.e. a zero coupon bond in currency 1 maturing at the same time as the option
		world.set_reporting(1,-2,mat);
		int numeraire_index = 0;
		ranlib::NormalUnit<double> normalRNG;
		MCEuropeanCall callpayoff(T(0),mat,0,strike);
		MCMapping<GaussMarkovWorld,Array<double,2> > mc_mapping(callpayoff,world,*(world.get_economies()[0]->initialTS),numeraire_index);
		std::function<double (Array<double,2>)> func = std::bind(&MCMapping<GaussMarkovWorld,Array<double,2> >::mapping,&mc_mapping,std::placeholders::_1);
		RandomArray<ranlib::NormalUnit<double>,double> random_container(normalRNG,world.factors(),world.number_of_steps()); 
		std::cerr << "Number of steps: " << world.number_of_steps() << endl;
		std::cerr << "Number of factors: " << world.factors() << endl;
		MCGeneric<Array<double,2>,double,RandomArray<ranlib::NormalUnit<double>,double> > mc(func,random_container);
		MCGatherer<double> mcgatherer;
		size_t n = minpaths;
		boost::math::normal normal;
		double d = boost::math::quantile(normal,0.95);
		cout << "Paths,MC value,95% CI lower bound,95% CI upper bound,Std error" << endl;
		while (mcgatherer.number_of_simulations()<maxpaths) 
		{
			mc.simulate(mcgatherer,n);
			cout << mcgatherer.number_of_simulations() << ',' << mcgatherer.mean() << ',' << mcgatherer.mean()-d*mcgatherer.stddev() << ',' << mcgatherer.mean()+d*mcgatherer.stddev();
			cout << ',' << (mcgatherer.mean()-CFcall)/mcgatherer.stddev() << endl;
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

*/

int main(int argc, char **argv)
{
	// change the current working directory
	std::cout << boost::filesystem::current_path() << std::endl; 
	_chdir("..\\CSVInputs"); 
	std::cout << boost::filesystem::current_path() << std::endl; 

	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}



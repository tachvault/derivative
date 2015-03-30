/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

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

#include "IIR.hpp"
#include "IIRValue.hpp"
#include "IIBOR.hpp"
#include "IIBORValue.hpp"
#include "IZeroCouponBondValue.hpp"
#include "IFixedRateBondValue.hpp"
#include "IIRDataSrc.hpp"
#include "IRCurve.hpp"
#include "PrimaryAssetUtil.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

// Declare a new test fixture for EntityManager, deriving from testing::Test.
class DomainTest : public testing::Test 
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

TEST_F(DomainTest, LoadLibraries) {

	/// load DataSource_MySQL component explicity
	bool retValue = LoadSharedLibrary("DataSource_MySQL");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load DataSource_REST component explicity
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

	/// load MySQLDataAccess  component explicity
	retValue = LoadSharedLibrary("XigniteDataAccess");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);
}

TEST_F(DomainTest, IRCurveLIBORTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest Libor rates for USA
	std::string cntry("USA");
	dd::date today = dd::day_clock::local_day();
	std::shared_ptr<IRCurve> irCurve;

	try
	{
		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::DataSourceType::LIBOR, cntry, today);
		const TermStructure& term = *(irCurve->GetTermStructure());

		auto length = term.length();
		auto _last = term.t(length -1);
		double dur = _last/20; /// get 20 points date
		double tindex = term.t(0);		
		while ( tindex <= _last)
		{
			double Bt = term(tindex);
			double r = PrimaryUtil::getDFToSimpleRate(Bt, tindex, 1);
			double cr = PrimaryUtil::getDFToCompoundRate(term(tindex), tindex);
			cout << tindex << "," << term(tindex) << "," << r * 100 << "%" << cr * 100 << "%" << endl;
			tindex = tindex + dur;
		}
		double Bt = term(_last);
		double r = PrimaryUtil::getDFToSimpleRate(Bt, _last, 1);
		double cr = PrimaryUtil::getDFToCompoundRate(term(_last), _last);
		cout << _last << "," << term(_last) << "," << r*100 << "%" << cr*100 << "%" << endl;
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(false);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

TEST_F(DomainTest, IRCurveIRTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest Libor rates for USA
	std::string cntry("USA");
	dd::date today = dd::day_clock::local_day();
	std::shared_ptr<IRCurve> irCurve;

	try
	{
		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::DataSourceType::YIELD, cntry, today);
		const TermStructure& term = *(irCurve->GetTermStructure());

		auto length = term.length();
		auto _last = term.t(length -1);
		double dur = _last/20; /// get 20 points date
		double tindex = term.t(0);		
		while ( tindex <= _last)
		{
			double Bt = term(tindex);
			double r = PrimaryUtil::getDFToSimpleRate(Bt, tindex, 1);
			cout << tindex << "," << term(tindex) << "," << r*100 << "%" << endl;
			tindex = tindex + dur;
		}
		double Bt = term(_last);
		double r = PrimaryUtil::getDFToSimpleRate(Bt, _last, 1);
		double cr = PrimaryUtil::getDFToCompoundRate(term(tindex), tindex);
		cout << _last << "," << term(_last) << "," << r*100 << "%" << endl;
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(false);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

TEST_F(DomainTest, IRCurveBondTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest Libor rates for USA
	std::string cntry("USA");
	dd::date day = dd::date(2014, 2, 18);
	std::shared_ptr<IRCurve> irCurve;
	
	try
	{
		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::DataSourceType::BOND, cntry, day);
		const TermStructure& term = *(irCurve->GetTermStructure());

		auto length = term.length();
		auto _last = term.t(length -1);
		double dur = _last/50; /// get 20 points date
		double tindex = term.t(0);		
		while ( tindex <= _last)
		{
			double Bt = term(tindex);
			double r = 2*(pow((1/Bt),1/(2*tindex))-1);
			cout << tindex << "," << term(tindex) << "," << r*100 << "%" << endl;
			tindex = tindex + dur;
		}
		double Bt = term(_last);
	    double r = 2*(pow((1/Bt),1/(2*_last))-1);
		cout << _last << "," << term(_last) << "," << r*100 << "%" << endl;
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(false);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}


int main(int argc, char **argv)
{
	FLAGS_log_dir = "C://Temp/glog";
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}
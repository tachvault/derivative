/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "EntityManager.hpp"
#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "gtest/gtest.h"
#include "Windows.h"
#include "Global.hpp"

#include "TSLinear.hpp"
#include "DeterministicCashflow.hpp"

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

	/// load PrimaryAsset  component explicity
	retValue = LoadSharedLibrary("PrimaryAsset");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load MySQLDataAccess  component explicity
	retValue = LoadSharedLibrary("MySQLDataAccess");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);	

}


TEST_F(DomainTest, TermStructureTest) 
{
	try
	{
		/* Construct cash flows.
		Coupon rate, Maturity, Price 
		4.125,    0.5,    101.05,    
		4.500,    1.0,    102.58,    
		4.750,    2.0,    105.47,    
		5.000,    3.0,    108.30,    
		3.875,    5.0,    105.21,    
		4.000,    7.0,    104.34,    
		3.500,    10.0,    97.94,    
		*/

		std::vector<shared_ptr<DeterministicCashflow> > cashFlows;

		//Coupon rate, Maturity, Price 
		// 4.125,    0.5,    101.05   
		Array<double, 1> _timeline_1(2);
		Array<double, 1> _cashflow_1(2);
		_timeline_1 = 0.0, 0.5;
		_cashflow_1 = 0.0, 102.0625;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_1 = \
			std::shared_ptr<DeterministicCashflow>(new DeterministicCashflow(_timeline_1,_cashflow_1, 101.05));
		cashFlows.push_back(m_cashFlow_1);

		/// Coupon rate, Maturity, Price 
		/// 4.500,    1.0,    102.58
		Array<double, 1> _timeline_2(3);
		Array<double, 1> _cashflow_2(3);
		_timeline_2 = 0.0, 0.5, 1.0;
		_cashflow_2 = 0.0, 2.25, 102.25;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_2 = \
			std::make_shared<DeterministicCashflow>(_timeline_2,_cashflow_2, 102.58);
		cashFlows.push_back(m_cashFlow_2);

		/// Coupon rate, Maturity, Price 
		/// 4.750,    2.0,    105.47
		Array<double, 1> _timeline_3(5);
		Array<double, 1> _cashflow_3(5);
		_timeline_3 = 0.0, 0.5, 1.0, 1.5, 2.0;
		_cashflow_3 = 0.0, 2.375, 2.375, 2.375, 102.375;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_3 = \
			std::make_shared<DeterministicCashflow>(_timeline_3,_cashflow_3, 105.47);
		cashFlows.push_back(m_cashFlow_3);

		/// Coupon rate, Maturity, Price
		/// 5.000,    3.0,    108.30
		Array<double, 1> _timeline_4(7);
		Array<double, 1> _cashflow_4(7);
		_timeline_4 = 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0;
		_cashflow_4 = 0.0, 2.5, 2.5, 2.5, 2.5, 2.5, 102.5;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_4 = \
			std::make_shared<DeterministicCashflow>(_timeline_4,_cashflow_4, 108.30);
		cashFlows.push_back(m_cashFlow_4);

		/// Coupon rate, Maturity, Price
		/// 3.875,    5.0,    105.21
		Array<double, 1> _timeline_5(11);
		Array<double, 1> _cashflow_5(11);
		_timeline_5 = 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0;
		_cashflow_5 = 0.0, 1.9375, 1.9375, 1.9375, 1.9375, 1.9375, 1.9375, 1.9375, 1.9375, 1.9375, 101.9375;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_5 = \
			std::make_shared<DeterministicCashflow>(_timeline_5,_cashflow_5, 105.21);
		cashFlows.push_back(m_cashFlow_5);

		/// Coupon rate, Maturity, Price
		/// 4.000,    7.0,    104.34
		Array<double, 1> _timeline_6(15);
		Array<double, 1> _cashflow_6(15);
		_timeline_6 = 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0;
		_cashflow_6 = 0.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 102.0;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_6 = \
			std::make_shared<DeterministicCashflow>(_timeline_6,_cashflow_6, 104.34);
		cashFlows.push_back(m_cashFlow_6);

		/// Coupon rate, Maturity, Price
		/// 3.500,    10.0,    97.94
		Array<double, 1> _timeline_7(21);
		Array<double, 1> _cashflow_7(21);
		_timeline_7 = 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0;
		_cashflow_7 = 0.0, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 1.75, 101.75;
		std::shared_ptr<DeterministicCashflow> m_cashFlow_7 = \
			std::make_shared<DeterministicCashflow>(_timeline_7,_cashflow_7, 97.94);
		cashFlows.push_back(m_cashFlow_7);
	
		Array<double, 1> _timeline(21);
		Array<double, 1> _cashflow(21);
		_timeline = 0.0;
		_cashflow = 0.0;
		TSLinear _termStructure(_timeline, _cashflow);

		/// Construct term structure from the given cashFlow
		_termStructure.bootstrap(cashFlows);

		/// output term structure of interest rate in  7 day interval
		double dur = 0.5;
		double tindex = _termStructure.t(0);	

		/// 2*((1/B(t,T))^(1/(2*(tindex))-1)
		cout << "Period, Discount factor, spot rate" << endl;
		while ( tindex <= 10)
		{
			double Bt = _termStructure(tindex);
			double r = 2*(pow((1/Bt),1/(2*tindex))-1);
			cout << tindex << "," << _termStructure(tindex) << "," << r*100 << "%" << endl;
			tindex = tindex + dur;
		}
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
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}
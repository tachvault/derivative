/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/


#include "gtest/gtest.h"

#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"

#include "EntityManager.hpp"
#include "IDataSource.hpp"
#include "CurrencyHolder.hpp"
#include "ExchangeHolder.hpp"
#include "ExchangeExt.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

// Declare a new test fixture for FinanialUtility, deriving from testing::Test.
class FinancialUtilityTest : public testing::Test 
{
protected:  

	/// virtual void SetUp() will be called before each test is run.  
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

// When you have a test fixture, you define a test using TEST_F
// instead of TEST.

// Tests the default c'tor.
TEST_F(FinancialUtilityTest, LoadData) 
{
	// You can access data in the test fixture here.

	/// Get the EntityManager instance
	/// Let the client catch and process
	/// RegistryException if thrown
	//EntityManager& entMgr = EntityManager::getInstance();
}

/// Test CurrencyHolder
TEST_F(FinancialUtilityTest, CurrencyTest) 
{
	std::string currStr("USD");
	std::string errCurrStr("US");
	CurrencyHolder& currHolder = CurrencyHolder::getInstance();
	Currency curr = currHolder.GetCurrency(currStr);

	ASSERT_TRUE(curr.GetCode().compare(currStr) == 0);

	try
	{
		CurrencyHolder& currHolder = CurrencyHolder::getInstance();
		Currency curr = currHolder.GetCurrency(errCurrStr);
		ASSERT_TRUE(false);
	}
	catch (std::invalid_argument& e)
	{
		LOG(ERROR) << "Currency  not found  for " << errCurrStr << endl;
		LOG(ERROR) << e.what() << endl;
		ASSERT_TRUE(true);
	}
}

/// Test ExchangeHolder
TEST_F(FinancialUtilityTest, ExchangeTest) 
{
	std::string exName("NYSE");
	std::string errExName("NYS");
	ExchangeHolder& exHolder = ExchangeHolder::getInstance();
	Exchange exchange = exHolder.GetExchange(exName);

	ASSERT_TRUE(exchange.GetExchangeName().compare(exName) == 0);

	try
	{
		ExchangeHolder& exHolder = ExchangeHolder::getInstance();
		Exchange exchange = exHolder.GetExchange(errExName);
		ASSERT_TRUE(false);
	}
	catch (std::invalid_argument& e)
	{
		LOG(ERROR) << "exchange name not found  for " << exName << endl;
		LOG(ERROR) << e.what() << endl;
		ASSERT_TRUE(true);
	}
}

/// Test ExchangeExt
TEST_F(FinancialUtilityTest, ExchangeExtTest) 
{
	std::string exchange("XTSE");
	std::string extension("TO");
	ushort src = YAHOO;
	ExchangeExt& symMap = ExchangeExt::getInstance();
	try
	{
		std::string ext = symMap.GetExchangeExt(src, exchange);
		ASSERT_TRUE(ext.compare(extension) == 0);
	}
	catch (std::invalid_argument& e)
	{
		LOG(ERROR) << "alias not found for src/symbol" << endl;
		ASSERT_TRUE(false);
	}	

	std::string exchange2("XNAS");
	std::string extension2("");
	try
	{
		std::string ext = symMap.GetExchangeExt(src, exchange2);
		ASSERT_TRUE(ext.compare(extension2) == 0);
	}
	catch (std::invalid_argument& e)
	{
		LOG(ERROR) << "alias not found for src/symbol" << endl;
		ASSERT_TRUE(false);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}
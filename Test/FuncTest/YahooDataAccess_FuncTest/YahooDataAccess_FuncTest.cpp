/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
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

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "HistoricalStockInfo.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "HistoricalExchangeRateInfo.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

// Declare a new test fixture for EntityManager, deriving from testing::Test.
class EntityManagerTest : public testing::Test 
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

	
	/// stock object ptrs
	std::shared_ptr<IStock> stockPtr;
	std::shared_ptr<IStockValue> stockValuePtr;

	/// exchange rate ptrs
	std::shared_ptr<IExchangeRate> exchangeRatePtr;
	std::shared_ptr<IExchangeRateValue> exchangeRateValuePtr;

	/// Declares the variables your tests want to use.
};

TEST_F(EntityManagerTest, LoadLibraries) {
	
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
}

/// Get the Stock value
TEST_F(EntityManagerTest, StockFindTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get the name for S&P 500 index (stored in MySQL database)
	std::string symbol("AAPL");
	Name stockName = IStock::ConstructName(symbol);
	bool foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(stockName, names);
	
	/// for IStock::TYPE there should be only Stock::TYPED
	/// as concrete type exists in the registry
	ASSERT_EQ(names.size(), 1);

	try
	{
		/// This call should find the named object in
		/// registry. If not found then it should fetch
		/// the stock data from database, construct stock
		/// and register with entity manager.
		stockPtr = dynamic_pointer_cast<IStock>(EntityMgrUtil::findObject(stockName));
		std::cout << stockPtr->GetName() << "," << stockPtr->GetDomesticCurrency().GetCode() << "," << stockPtr->GetDescription() \
			<< "," << stockPtr->GetExchange().GetCountry().GetCode() << endl;
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


/// Get the Stock value
TEST_F(EntityManagerTest, StockValueFindTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get the name for BCE (stored in MySQL database)
	std::string symbol("BCE");
	Name stockName = IStockValue::ConstructName(symbol);
	bool foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(stockName, names);
	
	/// for IStockValue::TYPE there should be only StockValue::TYPED
	/// as concrete type exists in the registry
	ASSERT_EQ(names.size(), 1);

	try
	{
		/// This call should find the named object in
		/// registry. If not found then it should fetch
		/// the stock value from yahoo, construct stock value
		/// and register with entity manager.
		stockValuePtr = dynamic_pointer_cast<IStockValue>(EntityMgrUtil::findObject(stockName, YAHOO));
		cout << "Retrieved stock data  " << stockValuePtr->GetAsset()->GetSymbol() \
			<< " Trade price " << stockValuePtr->GetTradePrice() \
			<< " Trade date " << stockValuePtr->GetTradeDate() << endl;
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

	/// get one more time

	try
	{
		/// refresh stockValue object
		bool status = EntityMgrUtil::refreshObject(dynamic_pointer_cast<IObject>(stockValuePtr), YAHOO);
		ASSERT_TRUE(status);
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


/// Get the historic Stock value
TEST_F(EntityManagerTest, HistoricStockValueFindTest) {

	/// Get the name for S&P 500 index (stored in MySQL database)
	std::string symbol("AAPL");
	/// start date
	dd::date start = dd::from_undelimited_string(std::string("20140101"));
	dd::date end = dd::from_undelimited_string(std::string("20140405"));

	try
	{
		BuildHistoricalStockInfo(YAHOO, symbol, start, end);
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

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// test if the created object can be found
	try
	{
		Name name = HistoricalStockInfo::ConstructName(symbol, start, end);			
		std::shared_ptr<IObject> obj = entMgr.findObject(name);
		ASSERT_TRUE(obj != nullptr);
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

/// Get the Stock value
TEST_F(EntityManagerTest, ExchangeRateValueFindTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the domestic currency
	std::string domestic("CAD");
	/// get the foreign currency
	std::string foreign("USD");
	Name exchangeRateName = IExchangeRateValue::ConstructName(domestic, foreign);
	bool foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(exchangeRateName, names);
	
	/// for IExchangeRateValue::TYPE there should be only ExchangeRateValue::TYPED
	/// as concrete type exists in the registry
	ASSERT_EQ(names.size(), 1);

	try
	{
		/// This call should find the named object in
		/// registry. If not found then it should fetch
		/// the exchangerate value from yahoo, construct exchange rate value
		/// and register with entity manager.
		exchangeRateValuePtr = dynamic_pointer_cast<IExchangeRateValue>(EntityMgrUtil::findObject(exchangeRateName, YAHOO));
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

	/// get one more time

	try
	{
		/// refresh stockValue object
		bool status = EntityMgrUtil::refreshObject(dynamic_pointer_cast<IObject>(exchangeRateValuePtr), YAHOO);
		ASSERT_TRUE(status);
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
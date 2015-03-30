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

#include "IStock.hpp"
#include "IIR.hpp"
#include "IIRValue.hpp"
#include "IIBOR.hpp"
#include "IIBORValue.hpp"
#include "IZeroCouponBondValue.hpp"
#include "IFixedRateBondValue.hpp"
#include "IIRDataSrc.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "IDailyFuturesOptionValue.hpp"
#include "IFutures.hpp"
#include "IFuturesValue.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

// Declare a new test fixture for EntityManager, deriving from testing::Test.
class MySQLDataAccessTest : public testing::Test 
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

TEST_F(MySQLDataAccessTest, LoadLibraries) {

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

/// Test finding stock objects.
TEST_F(MySQLDataAccessTest, FindStockTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get the name for S&P 500 index (stored in MySQL database)
	std::string symbol("AAPL");
	std::shared_ptr<IObject> Stock_SP;

	Name nm_1 = IStock::ConstructName(symbol);
	bool foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(nm_1, names);

	ASSERT_EQ(names.size(), 1);

	try
	{
		/// This call should find the named object in
		/// registry. If not found then it should fetch
		/// the stock data from database, construct stock
		/// and register with entity manager.
		Stock_SP = EntityMgrUtil::findObject(nm_1);
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

	/// if we are here, it means that the given stock object is in memory
	/// and registered with EntityManager
	/// find an object that is in the registry
	std::shared_ptr<IObject> temp_1;
	try
	{
		temp_1 = EntityMgrUtil::findObject(nm_1);
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

/// Test finding futures objects.
TEST_F(MySQLDataAccessTest, FindFuturesTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	std::string symbol("NL");
	std::shared_ptr<IObject> fur;

	Name nm_1 = IStock::ConstructName(symbol);
	bool foundFlag = true;

	/// Now find the registered concerete Names for the
	/// given alias type
	std::vector<Name> names;
	bool retValue = entMgr.findAlias(nm_1, names);

	ASSERT_EQ(names.size(), 1);

	try
	{
		fur = EntityMgrUtil::findObject(nm_1);
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

	/// if we are here, it means that the given stock object is in memory
	/// and registered with EntityManager
	/// find an object that is in the registry
	std::shared_ptr<IObject> temp_1;
	try
	{
		temp_1 = EntityMgrUtil::findObject(nm_1);
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


/// Test finding  Interest rate objects.
TEST_F(MySQLDataAccessTest, FindFuturesValueTest)
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	std::string symbol("JD");
	dd::date tradeDate = dd::date(2015, 3, 13);
	dd::date deliveryDate = dd::date(2015, 4, 17);
	Name nm = IFuturesValue::ConstructName(symbol, tradeDate, deliveryDate);

	bool foundFlag = true;
	std::shared_ptr<IFuturesValue> frValue;
	try
	{
		frValue = dynamic_pointer_cast<IFuturesValue>(EntityMgrUtil::findObject(nm));
		cout << "Trade Price " << frValue->GetSettledPrice() << endl;
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


/// Test finding  Interest rate objects.
TEST_F(MySQLDataAccessTest, FindInterestRateTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get the name for US interest rate with 6 months maturity
	/// with today's issue date
	std::string cntry("USA");
	auto maturity = Maturity::MaturityType::M6;
	dd::date today = dd::day_clock::local_day();
	Name nm = IIRValue::ConstructName(cntry, maturity, today);

	bool foundFlag = true;
	std::shared_ptr<IIRValue> ir;
	try
	{
		ir = dynamic_pointer_cast<IIRValue>(EntityMgrUtil::findObject(nm));
		cout << ir->GetLastRate() << ir->GetReportedDate() << ir->GetRate()->GetCountry().GetCode() << ir->GetRate()->GetMaturityType() << endl;
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

/// Test finding  Libor rate objects.
TEST_F(MySQLDataAccessTest, FindLiborRateTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get the name for US interest rate with one month maturity
	/// with today's issue date
	std::string curr("USD");
	auto maturity = Maturity::MaturityType::M1;
	dd::date today = dd::day_clock::local_day();
	Name nm = IIBORValue::ConstructName(curr, maturity, today);
	bool foundFlag = true;
	std::shared_ptr<IIBORValue> ir;
	try
	{
		ir = dynamic_pointer_cast<IIBORValue>(EntityMgrUtil::findObject(nm));
		try
		{
			//dynamic_pointer_cast<IIRDataSrc>(ir)->generateCashFlow(dd::day_clock::local_day());
		}
		catch(std::logic_error& e)
		{
			LOG(ERROR) << " Cannot use " << nm << " old rate " << endl;
		}
		cout << ir->GetLastRate() << ir->GetReportedDate() << ir->GetRate()->GetCurrency().GetCode() \
			<< ir->GetRate()->GetMaturityType() << endl;
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

TEST_F(MySQLDataAccessTest, FindLiborRatesTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest Libor rates for USD
	std::string curr("USD");
	dd::date today = dd::day_clock::local_day();
	Maturity::MaturityType mat = Maturity::MaturityType::O;
	Name nm = IIBORValue::ConstructName(curr, mat, today);

	try
	{
		std::vector<std::shared_ptr<IObject> > rates;
		EntityMgrUtil::findObjects(nm, rates);
		for(std::shared_ptr<IObject> obj: rates)
		{
			std::shared_ptr<IIBORValue> rate = dynamic_pointer_cast<IIBORValue>(obj);
			try
			{
				//dynamic_pointer_cast<IIRDataSrc>(rate)->generateCashFlow(dd::day_clock::local_day());
			}
			catch(std::logic_error& e)
			{
				LOG(ERROR) << " Cannot use " << rate->GetName() << " old rate " << endl;
			}
			cout << " Retrieved data:" \
				<< " Rate maturity " << rate->GetRate()->GetMaturityType() \
				<< " Traded date " << rate->GetReportedDate() \
				<< " rate " << rate->GetLastRate() << endl;
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

/// Test finding  Interest rate objects.
TEST_F(MySQLDataAccessTest, FindZeroCouponBondValueTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest 11 days US treasury bills
	std::string symbol("USABillD58");
	//'2014-02-18 00:00:00'
	dd::date day = dd::date(2014, 2, 18);
	Name nm = IZeroCouponBondValue::ConstructName(symbol, day);

	bool foundFlag = true;
	std::shared_ptr<IZeroCouponBondValue> bondVal;
	try
	{
		bondVal = dynamic_pointer_cast<IZeroCouponBondValue>(EntityMgrUtil::findObject(nm));
		try
		{
			dynamic_pointer_cast<IIRDataSrc>(bondVal)->generateCashFlow();
		}
		catch(std::logic_error& e)
		{
			LOG(ERROR) << " Cannot use " << bondVal->GetName() << " old rate " << endl;
		}
		cout << " Retrieved data:" \
			<< " Bond Description " << bondVal->GetAsset()->GetSymbol() \
			<< " Traded date " << bondVal->GetTradeDate() \
			<< " Maturity Date " << bondVal->GetMaturityDate() \
			<< " Bond price " << bondVal->GetTradePrice() << endl;
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

TEST_F(MySQLDataAccessTest, FindZeroCouponBondValuesTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get  all the zero coupon bonds with given trade date
	std::string symbol("USA");
	dd::date day = dd::date(2014, 2, 18);
	Name nm = IZeroCouponBondValue::ConstructName(symbol, day);

	try
	{
		std::vector<std::shared_ptr<IObject> > bonds;
		EntityMgrUtil::findObjects(nm, bonds);
		for(std::shared_ptr<IObject> obj: bonds)
		{
			std::shared_ptr<IZeroCouponBondValue> bond = dynamic_pointer_cast<IZeroCouponBondValue>(obj);
			try
			{
				dynamic_pointer_cast<IIRDataSrc>(bond)->generateCashFlow();
			}
			catch(std::logic_error& e)
			{
				LOG(ERROR) << " Cannot use " << bond->GetName() << " old rate " << endl;
			}
			cout << " Retrieved data:" \
				<< " Bond Description " << bond->GetAsset()->GetSymbol() \
				<< " Traded date " << bond->GetTradeDate() \
				<< " Maturity Date " << bond->GetMaturityDate() \
				<< " Bond price " << bond->GetTradePrice() << endl;
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

/// Test finding  Interest rate objects.
TEST_F(MySQLDataAccessTest, FindFixedRateBondValueTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get the latest 30 year US treasury bond
	std::string symbol("USANoteY10");
	dd::date day = dd::date(2014, 2, 18);
	Name nm = IFixedRateBondValue::ConstructName(symbol, day);

	bool foundFlag = true;
	std::shared_ptr<IFixedRateBondValue> bondVal;
	try
	{
		bondVal = dynamic_pointer_cast<IFixedRateBondValue>(EntityMgrUtil::findObject(nm));
		try
		{
			dynamic_pointer_cast<IIRDataSrc>(bondVal)->generateCashFlow();
		}
		catch(std::logic_error& e)
		{
			LOG(ERROR) << " Cannot use " << bondVal->GetName() << " old rate " << endl;
		}
		cout << " Retrieved data:" \
			<< " Bond Description " << bondVal->GetAsset()->GetSymbol() \
			<< " Traded date " << bondVal->GetTradeDate() \
			<< " Maturity Date " << bondVal->GetMaturityDate() \
			<< " Bond price " << bondVal->GetTradePrice() << endl;
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

TEST_F(MySQLDataAccessTest, FindFixedRateBondValuesTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// get  all rge
	std::string symbol("USA");
	dd::date day = dd::date(2014, 2, 18);
	Name nm = IFixedRateBondValue::ConstructName(symbol, day);

	try
	{
		std::vector<std::shared_ptr<IObject> > bonds;
		EntityMgrUtil::findObjects(nm, bonds);
		for(std::shared_ptr<IObject> obj: bonds)
		{
			std::shared_ptr<IFixedRateBondValue> bond = dynamic_pointer_cast<IFixedRateBondValue>(obj);
			try
			{
				dynamic_pointer_cast<IIRDataSrc>(bond)->generateCashFlow();
			}
			catch(std::logic_error& e)
			{
				LOG(ERROR) << " Cannot use " << bond->GetName() << " old rate " << endl;
			}
			cout << " Retrieved data:" \
				<< " Bond Description " << bond->GetAsset()->GetSymbol() \
				<< " Traded date " << bond->GetTradeDate() \
				<< " Maturity Date " << bond->GetMaturityDate() \
				<< " Bond price " << bond->GetTradePrice() << endl;
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


TEST_F(MySQLDataAccessTest, FindEquityOptionValuesTest) 
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get option data for apple with trade date as today,
	/// option maturity as Oct 10th
	std::string symbol("AAPL");
	dd::date today;
    dd::date matDate;
	IDailyEquityOptionValue::OptionType opt = IDailyEquityOptionValue::OptionType::OPTION_TYPE_UNKNOWN;
	double strike = 0;
	
	Name nm = IDailyEquityOptionValue::ConstructName(symbol, today, opt, matDate, strike);

	try
	{
		std::vector<std::shared_ptr<IObject> > options;
		EntityMgrUtil::findObjects(nm, options);
		for(std::shared_ptr<IObject> obj: options)
		{
			std::shared_ptr<IDailyEquityOptionValue> option = dynamic_pointer_cast<IDailyEquityOptionValue>(obj);
			cout << " Retrieved data:" \
				<< " Symbol " << option->GetAsset()->GetSymbol() \
				<< " Traded date " << option->GetTradeDate() \
				<< " Maturity Date " << option->GetMaturityDate() \
				<< " strike price " << option->GetStrikePrice() \
				<< " trade price " << option->GetTradePrice() << endl;
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


TEST_F(MySQLDataAccessTest, FindFuturesOptionValuesTest)
{
	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Get option data for Natural Gas with trade date as today
	std::string symbol("NG");
	dd::date tdate;
	dd::date matDate = dd::date(2015, 4, 15);
	IDailyEquityOptionValue::OptionType opt = IDailyEquityOptionValue::OptionType::VANILLA_CALL;
	double strike = 0;

	Name nm = IDailyFuturesOptionValue::ConstructName(symbol, tdate, opt, matDate, strike);

	try
	{
		std::vector<std::shared_ptr<IObject> > options;
		EntityMgrUtil::findObjects(nm, options);
		for (std::shared_ptr<IObject> obj : options)
		{
			std::shared_ptr<IDailyFuturesOptionValue> option = dynamic_pointer_cast<IDailyFuturesOptionValue>(obj);
			cout << " Retrieved data:" \
				<< " Symbol " << option->GetAsset()->GetSymbol() \
				<< " Trade date " << option->GetTradeDate() \
				<< " Delivery date " << option->GetDeliveryDate() \
				<< " Maturity date " << option->GetMaturityDate() \
				<< " strike price " << option->GetStrikePrice() \
				<< " trade price " << option->GetTradePrice() \
				<< " settled price " << option->GetSettledPrice() << endl;
		}
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

/// Unbind an object.
TEST_F(MySQLDataAccessTest, UnbindTest) {


	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// find an object that is in the registry
	/// Get the name for S&P 500 index (stored in MySQL database)
	std::string symbol("INDEXSP:.INX");
	std::shared_ptr<IObject> Stock_SP;

	long long stockId  = std::hash<std::string>()(symbol);

	/// find an object that is in the registry
	Name nm_1(IStock::TYPEID, static_cast<size_t>(stockId));

	/// get concrete objects registered for this Name
	std::vector<std::shared_ptr<IObject> > objs;
	try
	{
		entMgr.findAlias(Name(IDataSource::TYPEID, MYSQL), objs);
		EXPECT_EQ(objs.size(), 1);
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

	/// Now unbind *objs.begin()
	try
	{
		entMgr.unbind(*objs.begin());
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

	/// Now try to find the object
	try
	{
		entMgr.findAlias(Name(IDataSource::TYPEID, MYSQL), objs);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;

		/// the stock is registered and unbound
		/// the findAlias should throw Reghistration exception.
		ASSERT_TRUE(true);
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
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "EntityManager.hpp"
#include "Name.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h" 
#include "IStock.hpp"
#include "MockStock.hpp"
#include "MockStockValue.hpp"

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

	/// Declares the variables your tests want to use.
};

// Tests the default c'tor.
TEST_F(EntityManagerTest, EntityManagerSingleton) {
	// You can access data in the test fixture here.

	/// Get the EntityManager instance
	/// Let the client catch and process
	/// RegistryException if thrown
	EntityManager& entMgr = EntityManager::getInstance();
}

/// Test Adding new object.
TEST_F(EntityManagerTest, RegisterTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Register a Named object not in registry
	Name nm_1(MockStock::TYPEID,1001);
	std::shared_ptr<IStock> stock_1 = std::make_shared<MockStock>(nm_1);
	Name test = stock_1->GetName();

	try
	{
		entMgr.registerObject(nm_1, stock_1);
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

	/// Register another Named object not in registry
	Name nm_2(MockStock::TYPEID, 1002);
	std::shared_ptr<IStock> stock_2 = std::make_shared<MockStock>(nm_1);
	try
	{
		entMgr.registerObject(nm_2, stock_2);
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

	/// Register Named object that is in registry
	stock_2 = std::make_shared<MockStock>(nm_2);

	try
	{
		entMgr.registerObject(nm_2, stock_2);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

/// Test register alias.
TEST_F(EntityManagerTest, RegisterAliasTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// Register Stock and IStock
	entMgr.registerAlias(MockStock::TYPEID, IStock::TYPEID);

	/// Now find the registered concerete objects for the
	/// given alias type
	std::vector<std::shared_ptr<IObject> > objs;
	Name nm(IStock::TYPEID,1001);
	bool retValue = entMgr.findAlias(nm, objs);
	ASSERT_EQ(objs.size(), 1);

	/// Get the object and see if the type ID
	/// object ID are the correct Name(Stock::TYPEID, 1001)
	std::shared_ptr<IObject> temp_1 = *(objs.begin());
	int grpId = temp_1->GetName().GetGrpId();
	std::size_t objId = temp_1->GetName().GetObjId();
	int stockType = MockStock::TYPEID;
    ASSERT_EQ(grpId, stockType);
	ASSERT_EQ(objId, 1001); 
}

/// Test finding  objects.
TEST_F(EntityManagerTest, FindTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// find an object that is in the registry
	Name nm_1(MockStock::TYPEID, 1001);
	try
	{
		const std::shared_ptr<IObject> temp_1 = entMgr.findObject(nm_1);
		EXPECT_NE(temp_1, nullptr);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}	
	
	/// find an object that is not in the registry
	Name nm_3(MockStock::TYPEID, 1003);
	try
	{
		const std::shared_ptr<IObject> temp_3 = entMgr.findObject(nm_3);
		EXPECT_EQ(temp_3, nullptr);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(true);
	}	
	
	/// find an object type that is not in the registry
	Name nm_4(4, 1001);
	try
	{
		const std::shared_ptr<IObject> temp_4 = entMgr.findObject(nm_4);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

// Test 'contains'  member function of entity manager.
TEST_F(EntityManagerTest, ContainsTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// find an object that is in the registry
	Name nm_1(MockStock::TYPEID, 1001);
	std::shared_ptr<IObject> temp_1;
	try
	{
		temp_1 = entMgr.findObject(nm_1);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}	
	
	/// Check of temp_1 in the registry
	try
	{
		bool status = entMgr.contains(temp_1);
		EXPECT_EQ(status, true);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}	
	
	/// check the registry if an object that was not in returns true
	Name nm_3(MockStock::TYPEID, 1003);
	std::shared_ptr<IObject> stock_3 = std::make_shared<MockStock>(nm_3);
	try
	{
		bool status = entMgr.contains(stock_3);
		EXPECT_EQ(status, false);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(false);
	}
}

/// Unbind an object.
TEST_F(EntityManagerTest, UnbindTest) {

	/// Test for EntityManager singleton
	EntityManager& entMgr = EntityManager::getInstance();

	/// find an object that is in the registry
	Name nm_1(MockStock::TYPEID, 1001);
	std::shared_ptr<IObject> temp_1;
	try
	{
		temp_1 = entMgr.findObject(nm_1);
		EXPECT_NE(temp_1, nullptr);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(true);
	}	

	/// Now unbind temp_1
	try
	{
		entMgr.unbind(temp_1);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(true);
	}

	/// Now try to find the object
	try
	{
		const std::shared_ptr<IObject> temp_3 = entMgr.findObject(nm_1);
		EXPECT_EQ(temp_3, nullptr);
	}
	catch(RegistryException& e)
	{
		LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
		ASSERT_TRUE(true);
	}
	catch(...)
	{
		LOG(WARNING) << " Unknown Exception thrown " << endl;
		ASSERT_TRUE(true);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleMock(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}
/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

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
#include "Global.hpp"
#include "PrimaryAssetUtil.hpp"
#include "EquityVanillaOptMessage.hpp"
#include "FuturesVanillaOptMessage.hpp"
#include "IMessageSink.hpp"
#include "MsgProcessorManager.hpp"
#include "EquityGARCH.hpp"
#include "ExchangeRateGARCH.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;
using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;
using namespace std::placeholders;

#undef max;
#undef min;

// Declare a new test fixture for FacadesTest, deriving from testing::Test.
class FacadesTest : public testing::Test 
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
};

std::shared_ptr<IMessageSink> getMsgSink(msgType msgId)
{
	/// Get the MsgProcessorManager instance
	MsgProcessorManager& msgMgr = MsgProcessorManager::getInstance();

	/// Get the MsgSink group ID for the given message type
	grpType grpId = msgMgr.findProcessor(msgId);
	
	try
	{
		EntityManager& entMgr = EntityManager::getInstance();
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(grpId));
        std::shared_ptr<IMake> obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(Name(grpId, 1));
		std::shared_ptr<IMessageSink> msgProcessor = dynamic_pointer_cast<IMessageSink>(obj);

		/// now return the created message processor
		return msgProcessor;
	}
	catch (RegistryException& e)
	{
		LOG(WARNING) << " EntityGroup for the given Message Processor type not found for " << grpId << endl;
		LOG(WARNING) << e.what() << endl;
		throw e;
	}
}

TEST_F(FacadesTest, LoadLibraries) {

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

	/// load Messages  component explicity
	retValue = LoadSharedLibrary("Messages");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);

	/// load Facades  component explicity
	retValue = LoadSharedLibrary("Facades");
	/// Cannot continue if not load correctly.
	ASSERT_TRUE(retValue == true);
}


TEST_F(FacadesTest, FacadesHistoricVolTest)
{
	/// get the  historic vol.
	dd::date today = dd::day_clock::local_day();
	std::shared_ptr<EquityGARCH> garch = BuildEquityGARCH("AAPL", today);
	std::shared_ptr<DeterministicAssetVol> vol;
	try
	{
		// first try Vol surface
		vol = garch->GetVolatility();
	}
	catch (std::domain_error& e)
	{
		cout << "Error " << e.what() << endl;
	}

	/// get the  historic vol.
	std::shared_ptr<GARCH> garchfx = BuildExchangeRateGARCH("CAD", "USD", today);
	std::shared_ptr<DeterministicAssetVol> volfx;
	try
	{
		// first try Vol surface
		volfx = garchfx->GetVolatility();
	}
	catch (std::domain_error& e)
	{
		cout << "Error " << e.what() << endl;
	}
};

TEST_F(FacadesTest, FacadesEquityCallTest) 
{
	std::shared_ptr<EquityVanillaOptMessage> msg = std::make_shared<EquityVanillaOptMessage>();
	EquityVanillaOptMessage::Request req;
	req.option = EquityVanillaOptMessage::CALL;
	req.rateType = EquityVanillaOptMessage::LIBOR;
	req.style = EquityVanillaOptMessage::EUROPEAN;
	req.maturity = dd::date(2015, 5, 10);
	req.strike = 130.0;
	req.underlying = string("AAPL");

	/// closed form solution
	req.method = EquityVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	EquityVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution of call option " << res.optPrice << endl;

	/// lattice form solution
	req.method = EquityVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of call option " << res.optPrice << endl;

	/// closed form solution
	req.method = EquityVanillaOptMessage::MONTE_CARLO;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	//sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Monte-Carlo form solution  of call option " << res.optPrice << endl;
	cout << "\n---------------------------------------------------------\n";
};

TEST_F(FacadesTest, FacadesEquityPutEurTest)
{
	std::shared_ptr<EquityVanillaOptMessage> msg = std::make_shared<EquityVanillaOptMessage>();
	EquityVanillaOptMessage::Request req;
	req.option = EquityVanillaOptMessage::PUT;
	req.rateType = EquityVanillaOptMessage::LIBOR;
	req.style = EquityVanillaOptMessage::EUROPEAN;
	req.maturity = dd::date(2015, 5, 10);
	req.strike = 130.0;
	req.underlying = string("AAPL");

	/// closed form solution
	req.method = EquityVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	EquityVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution  of put option " << res.optPrice << endl;

	/// lattice form solution
	req.method = EquityVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of put option " << res.optPrice << endl;

	/// closed form solution
	req.method = EquityVanillaOptMessage::MONTE_CARLO;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	//sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Monte-Carlo form solution  of put option  " << res.optPrice << endl;
	cout << "\n---------------------------------------------------------\n";
};

TEST_F(FacadesTest, FacadesEquityPutAmerTest)
{
	std::shared_ptr<EquityVanillaOptMessage> msg = std::make_shared<EquityVanillaOptMessage>();
	EquityVanillaOptMessage::Request req;
	req.option = EquityVanillaOptMessage::PUT;
	req.rateType = EquityVanillaOptMessage::LIBOR;
	req.style = EquityVanillaOptMessage::AMERICAN;
	req.maturity = dd::date(2015, 5, 10);
	req.strike = 130.0;
	req.underlying = string("AAPL");

	/// closed form solution
	req.method = EquityVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	EquityVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution  of early exercise put option  " << res.optPrice << endl;

	/// lattice form solution
	req.method = EquityVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of early exercise put option " << res.optPrice << endl;

	/// closed form solution
	req.method = EquityVanillaOptMessage::MONTE_CARLO;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Monte-Carlo form solution  of early exercise put option " << res.optPrice << endl;
	cout << "\n---------------------------------------------------------\n";
};

TEST_F(FacadesTest, FacadesFuturesCallTest)
{
	std::shared_ptr<FuturesVanillaOptMessage> msg = std::make_shared<FuturesVanillaOptMessage>();
	FuturesVanillaOptMessage::FuturesRequest req;
	req.option = FuturesVanillaOptMessage::CALL;
	req.rateType = FuturesVanillaOptMessage::LIBOR;
	req.style = FuturesVanillaOptMessage::EUROPEAN;
	req.maturity = dd::date(2015, 10, 15);
	req.deliveryDate = dd::date(2015, 10, 15);
	req.strike = 3.4;
	req.underlying = string("NG");

	/// closed form solution
	req.method = FuturesVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	FuturesVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution of call option " << res.optPrice << endl;

	/// lattice form solution
	req.method = FuturesVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of call option " << res.optPrice << endl;

	/// closed form solution
	sink->Passivate();
	cout << "\n---------------------------------------------------------\n";
};

TEST_F(FacadesTest, FacadesFuturesPutEurTest)
{
	std::shared_ptr<FuturesVanillaOptMessage> msg = std::make_shared<FuturesVanillaOptMessage>();
	FuturesVanillaOptMessage::FuturesRequest req;
	req.option = FuturesVanillaOptMessage::PUT;
	req.rateType = FuturesVanillaOptMessage::LIBOR;
	req.style = FuturesVanillaOptMessage::EUROPEAN;
	req.maturity = dd::date(2015, 10, 15);
	req.deliveryDate = dd::date(2015, 10, 15);
	req.strike = 3.4;
	req.underlying = string("NG");

	/// closed form solution
	req.method = FuturesVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	FuturesVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution  of put option " << res.optPrice << endl;

	/// lattice form solution
	req.method = FuturesVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of put option " << res.optPrice << endl;
	sink->Passivate();
	cout << "Monte-Carlo form solution  of put option  " << res.optPrice << endl;
	cout << "\n---------------------------------------------------------\n";
};

TEST_F(FacadesTest, FacadesFuturesPutAmerTest)
{
	std::shared_ptr<FuturesVanillaOptMessage> msg = std::make_shared<FuturesVanillaOptMessage>();
	FuturesVanillaOptMessage::FuturesRequest req;
	req.option = FuturesVanillaOptMessage::PUT;
	req.rateType = FuturesVanillaOptMessage::LIBOR;
	req.style = FuturesVanillaOptMessage::AMERICAN;
	req.maturity = dd::date(2015, 10, 15);
	req.deliveryDate = dd::date(2015, 10, 15);
	req.strike = 3.4;
	req.underlying = string("NG");

	/// closed form solution
	req.method = FuturesVanillaOptMessage::CLOSED;
	msg->SetRequest(req);
	std::shared_ptr<IMessageSink> sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	FuturesVanillaOptMessage::Response res = msg->GetResponse();
	sink->Passivate();
	cout << "\n*********************************************************\n";
	cout << "Closed form solution  of early exercise put option  " << res.optPrice << endl;

	/// lattice form solution
	req.method = FuturesVanillaOptMessage::LATTICE;
	msg->SetRequest(req);
	sink = getMsgSink(msg->GetMsgId());
	sink->Dispatch(dynamic_pointer_cast<IMessage>(msg));
	res = msg->GetResponse();
	sink->Passivate();
	cout << "Lattice form solution  of early exercise put option " << res.optPrice << endl;
	sink->Passivate();
	cout << "Monte-Carlo form solution  of early exercise put option " << res.optPrice << endl;
	cout << "\n---------------------------------------------------------\n";
};

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	google::InitGoogleLogging("Derivative");
	return RUN_ALL_TESTS();
}

/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/


#include <memory>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <string.h>

#include "Name.hpp"
#include "StockValueYahooDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "IStockValue.hpp"
#include "IStock.hpp"
#include "EntityMgrUtil.hpp"
#include "PrimaryAssetUtil.hpp"
#include "RESTConnectionUtil.hpp"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(StockValueYahooDAO);
	DAO_REGISTER(IStockValue, YAHOO, StockValueYahooDAO);

	const int StockValueYahooDAO::MaxCount = 100;
	std::shared_ptr<IMake> StockValueYahooDAO::Make(const Name &nm)
	{
		/// Construct StockValueYahooDAO from given name and register with EntityManager
		std::shared_ptr<StockValueYahooDAO> dao = make_shared<StockValueYahooDAO>(nm);
		dao = dynamic_pointer_cast<StockValueYahooDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " StockValueYahooDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IMake> StockValueYahooDAO::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This function is not applicable to StockValueYahooDAO");
	}

	const std::shared_ptr<IObject> StockValueYahooDAO::find(const Name& nm)
	{
		LOG(INFO) << "StockValueYahooDAO::find(..) is called for " << nm << endl;
		/// If we are here means, StockValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from Yahoo.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_stockVal = PrimaryUtil::ConstructEntity<IStockValue>(nm);

		/// Now associate StockVal with stock.
		/// find stock with EntityUtil.
		/// find an object that is in the registry
		Name stockName(IStock::TYPEID, nm.GetObjId());
		try
		{
			/// This call should find the named object in
			/// registry. If not found then it should fetch
			/// the stock data from database, construct stock
			/// and register with entity manager.
			m_stockVal->SetStock(dynamic_pointer_cast<IStock>(EntityMgrUtil::findObject(stockName)));
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch (...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}

		/// Once we have the m_stockVal skeleton, it is time to populate the
		/// fields from the stockVal fetched from Yahoo.

		/// build the URI to include all the query parameters
		/// The built query should be the following if the symbol is GOOG
		///http://finance.yahoo.com/d/quotes.csv?s=AAPL+GOOG+MSFT&f=abopl1c1p2ydd1t1c1p2exrj2v

		web::http::uri_builder builder(U("http://finance.yahoo.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("d/quotes.csv"));

		std::string symbolStr = RESTConnectionUtil::GetTickerSymbol(YAHOO, m_stockVal->GetStock());
		utility::string_t symbol = utility::conversions::to_string_t(symbolStr);
		builder.append_query(L"s=" + symbol);
		builder.append_query(U("f=abopl1c1p2ydghjkd1t1c1p2exrj2v"));

		http_client client(U("http://finance.yahoo.com/d/"));
		client.request(methods::GET, builder.to_string()).then([&](http_response response)
		{
			Concurrency::streams::container_buffer<std::string> instringbuffer;
			response.body().read_to_end(instringbuffer).wait();
			std::string& data = instringbuffer.collection();
			istringstream istrData(data);
			char line[1024];
			if (!istrData.eof()) istrData.getline(line, 512, '\n');
			istringstream istr(line);
			istr >> m_stockVal;
		}).wait();

		/// now return m_stockVal
		m_stockVal = dynamic_pointer_cast<IStockValue>(EntityMgrUtil::registerObject(m_stockVal->GetName(), m_stockVal));
		return m_stockVal;
	}

	bool StockValueYahooDAO::refresh(shared_ptr<IObject>& obj)
	{
		LOG(INFO) << "StockValueYahooDAO::refresh(..) is called for " << obj->GetName() << endl;

		/// cast to IStockValue
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(obj);

		/// Once we have the m_stockVal skeleton, it is time to populate the
		/// fields from the stockVal fetched from Yahoo.

		/// build the URI to include all the query parameters
		/// The built query should be the following if the symbol is GOOG
		///http://finance.yahoo.com/d/quotes.csv?s=AAPL+GOOG+MSFT&f=abopl1c1p2ydd1t1c1p2exrj2v

		web::http::uri_builder builder(U("http://finance.yahoo.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("d/quotes.csv"));

		/// Get Stock's symbol value
		std::string symbolStr;
		IStockValue::GetKeys(stockVal->GetName(), symbolStr);

		utility::string_t symbol = utility::conversions::to_string_t(symbolStr);
		builder.append_query(L"s=" + symbol);

		builder.append_query(U("f=abopl1c1p2ydghjkd1t1c1p2exrj2v"));

		http_client client(U("http://finance.yahoo.com/d/"));
		client.request(methods::GET, builder.to_string()).then([&](http_response response)
		{
			Concurrency::streams::container_buffer<std::string> instringbuffer;
			response.body().read_to_end(instringbuffer).wait();
			std::string& data = instringbuffer.collection();
			istringstream istrData(data);
			char line[512];
			if (!istrData.eof()) istrData.getline(line, 512, '\n');
			istringstream istr(line);
			istr >> stockVal;
		}).wait();

		return true;
	}

} /* namespace derivative */
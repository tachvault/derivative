/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/


#include <memory>
#include "Name.hpp"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <string.h>

#include "ExchangeRateValueYahooDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "IExchangeRateValue.hpp"
#include "IExchangeRate.hpp"
#include "EntityMgrUtil.hpp"
#include "IExchangeRateValue.hpp"
#include "PrimaryAssetUtil.hpp"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(ExchangeRateValueYahooDAO);
	DAO_REGISTER(IExchangeRateValue, YAHOO, ExchangeRateValueYahooDAO);
	
	const int ExchangeRateValueYahooDAO::MaxCount = 100;
	std::shared_ptr<IMake> ExchangeRateValueYahooDAO::Make(const Name &nm)
	{
		/// Construct ExchangeRateValueYahooDAO from given name and register with EntityManager
		std::shared_ptr<ExchangeRateValueYahooDAO> dao = make_shared<ExchangeRateValueYahooDAO>(nm);
		dao = dynamic_pointer_cast<ExchangeRateValueYahooDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " ExchangeRateValueYahooDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IMake> ExchangeRateValueYahooDAO::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This function is not applicable to ExchangeRateValueYahooDAO");
	}
	
	const std::shared_ptr<IObject> ExchangeRateValueYahooDAO::find(const Name& nm)
	{
		LOG(INFO) << "ExchangeRateValueYahooDAO::find(..) is called for " << nm << endl;
		/// If we are here means, ExchangeRateValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from Yahoo.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_exchangeRateVal = PrimaryUtil::ConstructEntity<IExchangeRateValue>(nm);

		/// Now associate ExchangeRateVal with exchangeRate.
		/// find exchangeRate with EntityUtil.
		/// find an object that is in the registry
		Name exchangeRateName(IExchangeRate::TYPEID, nm.GetObjId());
		try
		{
			/// This call should find the named object in
			/// registry. If not found then it should fetch
			/// the exchangeRate data from database, construct exchangeRate
			/// and register with entity manager.
			m_exchangeRateVal->SetExchangeRate(dynamic_pointer_cast<IExchangeRate>(EntityMgrUtil::findObject(exchangeRateName)));
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch(...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}

		/// Once we have the m_exchangeRateVal skeleton, it is time to populate the
		/// fields from the exchangeRateVal fetched from Yahoo.

		/// build the URI to include all the query parameters
		/// The built query should be the following:
		/// "(foreign currency)(domestic currency)=X"
		/// http://finance.yahoo.com/d/quotes.csv?s=CADUSD=X&f=sl1abd1t1

		web::http::uri_builder builder(U("http://finance.yahoo.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("d/quotes.csv"));

		/// Get ExchangeRate's symbol value
		std::string domestic;
		std::string foreign;
		IExchangeRateValue::GetKeys(m_exchangeRateVal->GetName(), domestic, foreign);

		utility::string_t symbol = utility::conversions::to_string_t(foreign + domestic + "=X");        
		builder.append_query(L"s=" + symbol);
        
		builder.append_query(U("f=d1t1l1ab"));

		http_client client(U("http://finance.yahoo.com/d/"));
		client.request(methods::GET, builder.to_string()).then([=](http_response response)
		{
			Concurrency::streams::container_buffer<std::string> instringbuffer;
			response.body().read_to_end(instringbuffer).wait();
			std::string& data = instringbuffer.collection();

			/// remove \r
			data.erase (std::remove(data.begin(), data.end(), '\r'), data.end());

			istringstream istrData(data);
			char line[512];
			if (!istrData.eof()) istrData.getline(line, 512, '\n');
			istringstream istr(line);
			istr >> m_exchangeRateVal;	
		}).wait();

		/// now return m_exchangeRateVal
		m_exchangeRateVal = dynamic_pointer_cast<IExchangeRateValue>(EntityMgrUtil::registerObject(m_exchangeRateVal->GetName(), m_exchangeRateVal));
		return m_exchangeRateVal;
	}	

	bool ExchangeRateValueYahooDAO::refresh(shared_ptr<IObject>& obj)
	{
		LOG(INFO) << "ExchangeRateValueYahooDAO::refresh(..) is called for " << obj->GetName() << endl;

		/// cast to IExchangeRateValue
		std::shared_ptr<IExchangeRateValue> exchangeRateVal = dynamic_pointer_cast<IExchangeRateValue>(obj);

		/// Once we have the m_exchangeRateVal skeleton, it is time to populate the
		/// fields from the exchangeRateVal fetched from Yahoo.

		/// build the URI to include all the query parameters
		/// The built query should be the following if the symbol is GOOG
		///http://finance.yahoo.com/d/quotes.csv?s=AAPL+GOOG+MSFT&f=abopl1c1p2ydd1t1c1p2exrj2v

		web::http::uri_builder builder(U("http://finance.yahoo.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("d/quotes.csv"));

		/// get primary keys
		std::string domestic;
		std::string foreign;
		IExchangeRateValue::GetKeys(exchangeRateVal->GetName(), domestic, foreign);
		
		utility::string_t symbol = utility::conversions::to_string_t(foreign + domestic + "=X");        
		builder.append_query(L"s=" + symbol);
        
		builder.append_query(U("f=d1t1l1ab"));

		http_client client(U("http://finance.yahoo.com"));
		client.request(methods::GET, builder.to_string()).then([&](http_response response)
		{
			Concurrency::streams::container_buffer<std::string> instringbuffer;
			response.body().read_to_end(instringbuffer).wait();
			std::string& data = instringbuffer.collection();

			/// remove \r
			data.erase (std::remove(data.begin(), data.end(), '\r'), data.end());

			istringstream istrData(data);
			char line[512];
			if (!istrData.eof()) istrData.getline(line, 512, '\n');
			istringstream istr(line);
			istr >> exchangeRateVal;	
		}).wait();

		return true;
	}	

} /* namespace derivative */
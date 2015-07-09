/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "RESTJSONRequestInterceptor.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"
#include "DException.hpp"
#include "IMake.hpp"
#include "GroupRegister.hpp"
#include "Global.hpp"
#include "EntityMgrUtil.hpp"
#include "WebserviceUtil.hpp"
#include "ESBManager.hpp"

namespace derivative
{
	GROUP_REGISTER(RESTJSONRequestInterceptor);
	ALIAS_REGISTER(RESTJSONRequestInterceptor, IRequestInterceptor);
	ALIAS_REGISTER(RESTJSONRequestInterceptor, IRESTJSONRequestInterceptor);

	RESTJSONRequestInterceptor::RESTJSONRequestInterceptor(const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << " Exemplar id constructed " << endl;
	}

	RESTJSONRequestInterceptor::RESTJSONRequestInterceptor(const Name& nm)
		: m_name(nm)
	{
		LOG(INFO) << " MySQLDataSource constructed with name " << nm << endl;
	}

	RESTJSONRequestInterceptor::~RESTJSONRequestInterceptor()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	std::shared_ptr<IMake> RESTJSONRequestInterceptor::Make(const Name &nm)
	{
		/// Construct data source from given name and register with EntityManager
		std::shared_ptr<RESTJSONRequestInterceptor> interceptor = make_shared<RESTJSONRequestInterceptor>(nm);
		EntityManager& entMgr = EntityManager::getInstance();
		entMgr.registerObject(nm, interceptor);

		/// return constructed object if no exception is thrown
		return interceptor;
	}

	std::shared_ptr<IMake> RESTJSONRequestInterceptor::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This method is not supported");
	}

	void RESTJSONRequestInterceptor::StartInterceptor()
	{
		/// get address, port and path from name
		std::string address;
		unsigned int port = 0;
		std::string path;
		IRESTJSONRequestInterceptor::GetKeys(m_name, address, port, path);

		/// build uri_builder
		address.append(to_string(port));
		uri_builder uri(utility::conversions::to_string_t(address));
		uri.append_path(utility::conversions::to_string_t(path));

		/// now open the URI and listen
		m_listener = http_listener(uri.to_uri().to_string());
		m_listener.support(methods::GET, std::bind(&RESTJSONRequestInterceptor::handle_get, this, std::placeholders::_1));
		m_listener.support(methods::PUT, std::bind(&RESTJSONRequestInterceptor::handle_put, this, std::placeholders::_1));
		m_listener.support(methods::POST, std::bind(&RESTJSONRequestInterceptor::handle_post, this, std::placeholders::_1));
		m_listener.support(methods::DEL, std::bind(&RESTJSONRequestInterceptor::handle_delete, this, std::placeholders::_1));

		//LOG(INFO) << "Listening for requests at: " << address << std::endl;
		try
		{
			pplx::task<void>  task = m_listener.open();
			cout << " Listener opened " << endl;
			task.wait();
			cout << " Waiting for Listener " << endl;
		}
		catch (exception& e)
		{
			cout << " throwing exception " << e.what() << endl;
		}
		return;
	}

	void RESTJSONRequestInterceptor::Shutdown()
	{
		m_listener.close().wait();
		return;
	}

	void RESTJSONRequestInterceptor::handle_get(http_request message)
	{
		try
		{
			LOG(INFO) << "Received message: " << utility::conversions::to_utf8string(message.relative_uri().to_string()) << endl;

			/// get the path components. The Message type will be derived from path
			auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

			/// query strings provide the message data.
			auto query_strings = http::uri::split_query(http::uri::decode(message.relative_uri().query()));

			/// authorize the request first
			if (query_strings.find(U("_token")) != query_strings.end())
			{
				ESBManager& esb = ESBManager::getInstance();
				if (!esb.Authorize(conversions::to_utf8string(query_strings.at(U("_token")))))
				{
					throw std::invalid_argument("Invalid access token");
				}
			}
			else
			{
				throw std::invalid_argument("Request is not authorized");
			}

			/// JSON message for output
			if (!paths[0].empty() && paths[0].compare(U("EquityOption")) == 0)
			{
				auto msg = WebServiceUtil::HandleEquityOption(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else if (paths[0].compare(U("FuturesOption")) == 0)
			{
				auto msg = WebServiceUtil::HandleFuturesOption(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else if (paths[0].compare(U("FXOption")) == 0)
			{
				auto msg = WebServiceUtil::HandleFXOption(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else if (paths[0].compare(U("InterestrateOption")) == 0)
			{
				throw std::invalid_argument("Interest rate options are not exposed yet");
			}
			else if (paths[0].compare(U("EquitySpread")) == 0)
			{
				auto msg = WebServiceUtil::HandleEquityOptionSpread(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else if (paths[0].compare(U("FuturesSpread")) == 0)
			{
				auto msg = WebServiceUtil::HandleFuturesOptionSpread(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else if (paths[0].compare(U("FXSpread")) == 0)
			{
				auto msg = WebServiceUtil::HandleFXOptionSpread(paths, query_strings);
				message.reply(status_codes::OK, msg);
			}
			else
			{
				throw std::invalid_argument("Only equity/futures/interest rate asset classes are supported");
			}
		}
		catch (std::exception& e)
		{
			json::value out;
			out[L"outcome"] = json::value::number(IMessage::RequestError);
			out[L"Message"] = json::value::string(utility::conversions::to_string_t(e.what()));
			message.reply(status_codes::InternalError, out);
		}
		return;
	};

	void RESTJSONRequestInterceptor::handle_post(http_request message)
	{
		json::value out;
		out[L"outcome"] = json::value::number(IMessage::RequestError);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t("POST method is not supported for this request"));
		message.reply(status_codes::InternalError, out);
	};

	void RESTJSONRequestInterceptor::handle_delete(http_request message)
	{
		json::value out;
		out[L"outcome"] = json::value::number(IMessage::RequestError);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t("DELETE method is not supported for this request"));
		message.reply(status_codes::InternalError, out);
	};

	void RESTJSONRequestInterceptor::handle_put(http_request message)
	{
		json::value out;
		out[L"outcome"] = json::value::number(IMessage::RequestError);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t("PUT method is not supported for this request"));
		message.reply(status_codes::InternalError, out);
	};
} /* namespace derivative */
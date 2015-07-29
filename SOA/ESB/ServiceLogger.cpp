/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include <future>
#include "ServiceLogger.hpp"
#include "DException.hpp"
#include "UserInfoMySQLDAO.hpp"

namespace derivative
{
	ServiceLogger::ServiceLogger()
	{
		LOG(INFO) << "ServiceLogger constructor is called " << endl;
		auto future = std::async(std::launch::async, &ServiceLogger::WriteRequest, this);
	}

	ServiceLogger::~ServiceLogger()
	{
		LOG(INFO) << "ServiceLogger destructor is called " << endl;
	}

	void ServiceLogger::LogMessage(const std::string& token, const std::string& datetime, const std::string& url)
	{
		std::shared_ptr<request> req = std::make_shared<request>(token, datetime, url);
		m_reqQueue.push(req);
	}

	void ServiceLogger::WriteRequest()
	{
		while (true)
		{
			/// try pop a message
			std::shared_ptr<request>& req = m_reqQueue.wait_and_pop();
			UserInfoMySQLDAO dao;
			dao.insertReq(req->token, req->datetime, req->url);
		}
	}
} /* namespace derivative */
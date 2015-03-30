/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include "ServiceLogger.hpp"
#include "DException.hpp"

namespace derivative
{
	ServiceLogger::ServiceLogger()
	{
		LOG(INFO) << "ServiceLogger constructor is called " << endl;
	}

	ServiceLogger::~ServiceLogger()
	{
		LOG(INFO) << "ServiceLogger destructor is called " << endl;
	}

	void ServiceLogger::LogMessage(const std::shared_ptr<IMessage>& msg)
	{
		
	}
} /* namespace derivative */
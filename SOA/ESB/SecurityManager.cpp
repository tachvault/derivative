/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include "SecurityManager.hpp"
#include "DException.hpp"
#include "UserInfoHolder.hpp"

namespace derivative
{
	SecurityManager::SecurityManager()
	{
		/// part of the security check is checking the request token
		/// against the set of tokens stored in the database as authorised.
		m_userInfo = std::unique_ptr<UserInfoHolder>(new UserInfoHolder);
		m_userInfo->LoadData();

		LOG(INFO) << "SecurityManager constructor is called " << endl;
	}

	SecurityManager::~SecurityManager()
	{
		LOG(INFO) << "SecurityManager destructor is called " << endl;
	}

	bool SecurityManager::AuthorizeRequest(const std::string& token)
	{
		/// check if the msg token in the set of tokens authorised
		return m_userInfo->IsValid(token);
	}
} /* namespace derivative */
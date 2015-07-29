/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <vector>
#include <algorithm>

#include "IDataSource.hpp"
#include "UserInfoHolder.hpp"
#include "UserInfoMySQLDAO.hpp"

namespace derivative
{

	UserInfoHolder::UserInfoHolder()
	{
		LoadData();
	}

	UserInfoHolder::~UserInfoHolder()
	{}

	void UserInfoHolder::LoadData()
	{
		LOG(INFO) << "Loading tokens into database \n";
		UserInfoMySQLDAO dao;
		dao.GetTokens(m_tokens);
	}

	bool  UserInfoHolder::IsValid(const std::string& token)
	{
		std::unique_lock<SpinLock> lock(m_lock);
		auto  result = std::find(m_tokens.begin(), m_tokens.end(), token);
		if (result != m_tokens.end())
		{
			return true;
		}
		else
		{
			/// unlock lock so that other threads can access m_tokens.
			lock.unlock();
			UserInfoMySQLDAO dao;
			if (dao.GetToken(token))
			{
				lock.lock();
				m_tokens.push_back(token);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}
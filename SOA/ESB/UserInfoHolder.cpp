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
	{}

	UserInfoHolder::~UserInfoHolder()
	{}
	
	void UserInfoHolder::LoadData()
	{
		UserInfoMySQLDAO dao;
		dao.GetTokens(m_tokens);
	}

	bool  UserInfoHolder::IsValid(const std::string& token)
	{
		auto  result = std::find(m_tokens.begin(), m_tokens.end(), token);

        /// Print the result.
        if (result != m_tokens.end()) 
		{
			return true;
		}

		return false;
	}	
}
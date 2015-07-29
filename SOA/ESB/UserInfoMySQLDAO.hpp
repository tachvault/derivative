/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_USERINFOMYSQLDAO_H_
#define _DERIVATIVE_USERINFOMYSQLDAO_H_

#include <memory>
#include <mutex>

#include "MySqlUserInfoConnection.hpp"
#include "Global.hpp"

namespace derivative
{
	// Data Access Object for UserInfo entity. 
	class UserInfoMySQLDAO 			 
	{
	public:

		enum {TYPEID = CLASS_USERINFOMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator UserInfoMySQLDAO object
		UserInfoMySQLDAO()
		{
		}

		/// Get all the UserInfo objects
		void GetTokens(std::vector<string> &tokens);

		bool GetToken(const std::string& token);

		void insertReq(const std::string& token, const std::string& datetime, const std::string& url);

	private:
		
		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		std::unique_ptr<sql::Connection> m_logCon;

		std::mutex m_lock;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_USERINFOMYSQLDAO_H_ */

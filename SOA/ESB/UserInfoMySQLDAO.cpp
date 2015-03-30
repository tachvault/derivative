/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

#include "UserInfoMySQLDAO.hpp"

namespace derivative
{
	void UserInfoMySQLDAO::GetTokens(std::vector<string> &tokens)
	{
		try
		{
			if (!m_con)
			{
				MySQLUserInfoConnection::InitConnection(m_driver, m_con);
			}
		}
		catch(sql::SQLException &e) 
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}	

		try 
		{
			std::auto_ptr<sql::Statement> stmt(m_con->createStatement());	
			stmt->execute("CALL get_all_tokens()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					std::string token = res->getString("token").asStdString();
					tokens.push_back(token);

					LOG(INFO) << " Retrived token " << token << endl;
				}	
			} 
			while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	}

} /* namespace derivative */
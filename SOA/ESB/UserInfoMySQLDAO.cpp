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
		catch (sql::SQLException &e)
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
			} while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	}

	bool UserInfoMySQLDAO::GetToken(const std::string& token)
	{
		try
		{
			if (!m_con)
			{
				MySQLUserInfoConnection::InitConnection(m_driver, m_con);
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}

		try
		{
			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("select count(*) AS cnt from security where token = \"") + token + "\"";
			res.reset(stmt->executeQuery(query.c_str()));
			while (res->next())
			{
				auto count = res->getInt("cnt");
				if (count > 0) return true;
				else return false;
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	}

	void UserInfoMySQLDAO::insertReq(const std::string& token, const std::string& datetime, const std::string& url)
	{
		try
		{
			MySQLUserInfoConnection::InitConnection(m_driver, m_con);
			
			auto pstmt = m_con->prepareStatement("CALL insert_requests(?, ?, ?)");
			pstmt->setString(1, token);
			pstmt->setString(2, datetime);
			pstmt->setString(3, url);
			pstmt->execute();
			pstmt->close();
			delete pstmt;
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << "Error loading data " << url << endl;
			LOG(ERROR) << "# ERR: " << e.what();
		}
	}

} /* namespace derivative */
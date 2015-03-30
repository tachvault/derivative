/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "CurrencyMySQLDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"

namespace derivative
{
	void CurrencyMySQLDAO::GetEntities(std::deque<Currency> &data)
	{	
		/// Make a connection to the MySQL if not connected
		/// log the SQLException error if thrown
		/// and retrow the exception so that the client
		/// can catch and take required action
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
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
			stmt->execute("CALL get_all_currency()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					std::string name = res->getString("name").asStdString();
					std::string isoCode = res->getString("isoCode").asStdString();
					unsigned int isoNumCode = res->getInt("isoNumCode");
					std::string symbol = res->getString("symbol").asStdString();
					unsigned int fractionUnit = res->getInt("fractionUnit");
					std::string format = res->getString("format").asStdString();
					std::string fractionSymbol = res->getString("fractionSymbol").asStdString();

					Currency curr(name, isoCode, isoNumCode, symbol, fractionSymbol, fractionUnit);
					data.push_back(curr);
					LOG(INFO) << " New Currency object created with Currency(" << name << ", " << isoCode << ", " \
						<< isoNumCode << ", " << symbol << endl;
				}
			} 
			while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " Error loading exchange data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	}

} /* namespace derivative */
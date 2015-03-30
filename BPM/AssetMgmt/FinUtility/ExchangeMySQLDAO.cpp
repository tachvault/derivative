/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityMgrUtil.hpp"

#include "ExchangeMySQLDAO.hpp"

#include "Country.hpp"
#include "Exchange.hpp"
#include "CountryHolder.hpp"

namespace derivative
{
	void ExchangeMySQLDAO::GetEntities(std::deque<Exchange> &data)
	{
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
			stmt->execute("CALL get_all_exchange()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					std::string name = res->getString("name").asStdString();
					std::string currStr = res->getString("country").asStdString();
					int timeOffSet = res->getInt("timeOffSet");

					/// get country for the given code
					CountryHolder& currHolder = CountryHolder::getInstance();
					const Country& curr = currHolder.GetCountry(currStr);

					/// create Exchange object
					Exchange exchange(name, curr, timeOffSet);
					data.push_back(std::move(exchange));

					LOG(INFO) << " New Exchange object created with Exchange(" << name << ", " << currStr << ", " \
						<< timeOffSet << endl;
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
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityMgrUtil.hpp"

#include "CountryMySQLDAO.hpp"

#include "Currency.hpp"
#include "Country.hpp"
#include "CurrencyHolder.hpp"

namespace derivative
{
	void CountryMySQLDAO::GetEntities(std::deque<Country> &data)
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
			stmt->execute("CALL get_all_country()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					std::string code = res->getString("isoCode").asStdString();
					std::string name = res->getString("name").asStdString();
					std::string currStr = res->getString("currencyCode").asStdString();

					/// get currency for the given code
					CurrencyHolder& currHolder = CurrencyHolder::getInstance();
					const Currency& curr = currHolder.GetCurrency(currStr);

					/// create Country object
					Country country(code, name, curr);
					data.push_back(std::move(country));

					LOG(INFO) << " New Country object created with Country(" << code << ", " << name << ", " \
						<< currStr << endl;
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
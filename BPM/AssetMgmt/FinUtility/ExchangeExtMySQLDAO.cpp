/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "ExchangeExtMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "IDataSource.hpp"

namespace derivative
{
	void ExchangeExtMySQLDAO::GetExchangeExt(ExchangeExt::ExchangeExtType& symMap)
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
			stmt->execute("CALL get_all_ExchangeExt()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					ushort datasrc = static_cast<ushort>(res->getInt("datasrc"));
					std::string exchange = res->getString("exchange").asStdString();
					std::string extension = res->getString("extension").asStdString();

					LOG(INFO) << " ExchangeExt object constructed with " << exchange \
						<< " " << datasrc \
						<< " " << extension << endl;

					/// insert the exchange extension into map.
					std::pair<ushort, std::string> exchangeExPair;
					exchangeExPair = make_pair(datasrc, exchange);
					symMap.insert(std::make_pair(exchangeExPair, extension));
				}		
			} 
			while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No data found in source for symbol map" << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};		
	}

} /* namespace derivative */
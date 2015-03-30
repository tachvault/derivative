/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "SymbolMapMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "IDataSource.hpp"

namespace derivative
{
	void SymbolMapMySQLDAO::GetSymbolMap(SymbolMap::SymbolMapType& symMap)
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
			stmt->execute("CALL get_all_symbolMap()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					ushort datasrc = static_cast<ushort>(res->getInt("datasrc"));
					std::string symbol = res->getString("symbol").asStdString();
					std::string alias = res->getString("symbolalias").asStdString();

					LOG(INFO) << " SymbolMap object constructed with " << symbol \
						<< " " << datasrc \
						<< " " << alias << endl;

					/// insert the symbol into map.
					std::pair<ushort, std::string> dataSrcSymbolPair;
					dataSrcSymbolPair = make_pair ( datasrc, symbol);
					symMap.insert(std::make_pair(dataSrcSymbolPair, alias));		
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
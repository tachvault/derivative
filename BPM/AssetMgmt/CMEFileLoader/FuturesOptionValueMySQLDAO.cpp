/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <future>

#include "MySqlConnection.hpp"
#include "FuturesOptionValueMySQLDAO.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void FuturesOptionValueMySQLDAO::Connect()
	{
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	}

	void FuturesOptionValueMySQLDAO::Upload()
	{
		vector<string> vec;

		std::ifstream infile(m_furFile.c_str());
		string line;
		/// skip the first header line
		if (infile.is_open()) getline(infile, line);
		if (infile.is_open())
		{
			while (getline(infile, line))
			{
				Insert(line);
				//std::future<void> f = std::async(std::launch::async, &FuturesOptionValueMySQLDAO::InsertOption, this, line);
			}
		}
		infile.close();
	};

	void FuturesOptionValueMySQLDAO::Delete()
	{
		sql::PreparedStatement *pstmt;
		pstmt = m_con->prepareStatement("CALL delete_DailyOptionValue()");
		pstmt->execute();
		delete pstmt;
	}

	void FuturesOptionValueMySQLDAO::Insert(const std::string& line)
	{
		/// input: PRODUCT SYMBOL(0),CONTRACT MONTH(1),CONTRACT YEAR(2),CONTRACT DAY(3),PUT/CALL(4),STRIKE(5),CONTRACT(6),
		/// PRODUCT DESCRIPTION(7),OPEN(8),HIGH(9),HIGH AB INDICATOR(10),LOW(11),LOW AB INDICATOR(12),LAST(13),
		/// LAST AB INDICATOR(14),SETTLE(15),PT CHG(16),EST. VOL(17),PRIOR SETTLE(18),PRIOR VOL(19),
		/// PRIOR INT(20),TRADEDATE(21)

		/// output: future_id(0), tradedate(21), contract_date(2,1,3), maturity_date(2,1,3), opt_type(4),strike(5), open(8),high(9),
		/// low(11),last_price(13),settle(15), volume(17),open_int(20)

		std::vector<std::string> vec;
		splitLine(line, vec);

		string symbol = vec[0];
		auto fur_id = std::hash<std::string>()(symbol);
		dd::date contract_date;
		if (vec[3].empty())
		{
			contract_date = dd::date(stoi(vec[2]), stoi(vec[1]), 15);
		}
		else
		{
			contract_date = dd::date(stoi(vec[2]), stoi(vec[1]), stoi(vec[3]));
		}
		int optType = vec[4].compare("C") == 0 ? 1 : 2;
		auto strike = vec[5].empty() ? 0.0 : std::atof(vec[5].c_str());
		auto open = vec[8].empty() ? 0.0 : std::atof(vec[8].c_str());
		auto high = vec[9].empty() ? 0.0 : std::atof(vec[9].c_str());
		auto low = vec[11].empty() ? 0.0 : std::atof(vec[11].c_str());
		auto last = vec[13].empty() ? 0.0 : std::atof(vec[13].c_str());
		auto settle = vec[15].empty() ? 0.0 : std::atof(vec[15].c_str());
		int vol = vec[17].empty() ? 0.0 : std::atoi(vec[17].c_str());
		int openInt = vec[20].empty() ? 0.0 : std::atoi(vec[20].c_str());

		auto trade_date = dd::from_us_string(vec[21]);
		/// now load data
		try
		{
			auto pstmt = m_con->prepareStatement("CALL insert_dailyfuturesoptionvalue(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			pstmt->setUInt64(1, fur_id);
			pstmt->setString(2, dd::to_iso_extended_string(trade_date));
			pstmt->setString(3, dd::to_iso_extended_string(contract_date));
			pstmt->setString(4, dd::to_iso_extended_string(contract_date));
			pstmt->setInt(5, optType);
			pstmt->setDouble(6, strike);
			pstmt->setDouble(7, open);
			pstmt->setDouble(8, high);
			pstmt->setDouble(9, low);
			pstmt->setDouble(10, last);
			pstmt->setDouble(11, settle);
			pstmt->setInt(12, vol);
			pstmt->setInt(13, openInt);
			pstmt->execute();
			pstmt->close();
			delete pstmt;
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << "Error loading data " << line << endl;
			LOG(ERROR) << "# ERR: " << e.what();
		}
	};

} /* namespace derivative */
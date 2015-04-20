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
#include "FuturesValueMySQLDAO.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void FuturesValueMySQLDAO::Connect()
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

	void FuturesValueMySQLDAO::Upload()
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
			}
		}
		infile.close();
	};

	void FuturesValueMySQLDAO::Delete()
	{
		sql::PreparedStatement *pstmt;
		pstmt = m_con->prepareStatement("CALL delete_DailyOptionValue()");
		pstmt->execute();
		delete pstmt;
	}

	void FuturesValueMySQLDAO::Insert(const std::string& line)
	{
		/// input: PRODUCT SYMBOL(0),CONTRACT MONTH(1),CONTRACT YEAR(2),CONTRACT DAY(3),CONTRACT,(4)PRODUCT DESCRIPTION(5),
		/// OPEN(6),HIGH(7),HIGH AB INDICATOR(8),LOW(9),LOW AB INDICATOR(10),LAST(11),LAST AB INDICATOR(12),SETTLE(13),PT CHG(14),
		/// EST. VOL(15),PRIOR SETTLE(16),PRIOR VOL(17),PRIOR INT(18),TRADEDATE(19)

		/// output: sym_tdate_id(0), contract_date(2,1), open(6), high(7), low(9), last(11), settle(13), 
		/// volume(15), open_int(18), tradedate(19)

		std::vector<std::string> vec;
		splitLine(line, vec);

		string symbol = vec[0];
		dd::date tdate = dd::from_us_string(vec[19]);
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
		auto open = vec[6].empty() ? 0.0 : std::atof(vec[6].c_str());
		auto high = vec[7].empty() ? 0.0 : std::atof(vec[7].c_str());
		auto low = vec[9].empty() ? 0.0 : std::atof(vec[9].c_str());
		auto last = vec[11].empty() ? 0.0 : std::atof(vec[11].c_str());
		auto settle = vec[13].empty() ? 0.0 : std::atof(vec[13].c_str());
		auto est_vol = vec[15].empty() ? 0.0 : std::atof(vec[15].c_str());
		auto prior_int = vec[18].empty() ? 0.0 : std::atof(vec[18].c_str());
		auto trade_date = dd::from_us_string(vec[19]);

	//	cout << fur_id << "," << dd::to_iso_extended_string(contract_date) << "," \
			<< open << "," << high << "," << low << "," << last << "," << settle << "," \
			<< est_vol << "," << prior_int << "," << dd::to_iso_extended_string(trade_date) << endl;

		/// now load data
		try
		{
			auto pstmt = m_con->prepareStatement("CALL insert_FuturesValue(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			pstmt->setUInt64(1, fur_id);
			pstmt->setString(2, dd::to_iso_extended_string(contract_date));
			pstmt->setDouble(3, open);
			pstmt->setDouble(4, high);
			pstmt->setDouble(5, low);
			pstmt->setDouble(6, last);
			pstmt->setDouble(7, settle);
			pstmt->setDouble(8, est_vol);
			pstmt->setInt(9, prior_int);
			pstmt->setString(10, dd::to_iso_extended_string(trade_date));
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
/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <future>
#include <tchar.h>

#include "Poco/Foundation.h"
#include "Poco/Net/Net.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Net/DialogSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"
#include "Poco/URI.h"
#include "Poco/StreamCopier.h"

#include "MySqlConnection.hpp"
#include "FuturesValueMySQLDAO.hpp"
#include "QFUtil.hpp"

using Poco::Net::FTPStreamFactory;
using Poco::Net::FTPPasswordProvider;
using Poco::Net::DialogSocket;
using Poco::Net::SocketAddress;
using Poco::Net::FTPException;
using Poco::URI;
using Poco::StreamCopier;

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
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << std::endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	}

	void FuturesValueMySQLDAO::Upload()
	{
		URI uri;
		// ftp://ftp.cmegroup.com/pub/settle/comex_future.csv
		uri.setScheme("ftp");
		uri.setHost("ftp.cmegroup.com");
		//uri.setPort(20);
		uri.setPath("/pub/settle/comex_future.csv;type=a");
		FTPStreamFactory sf;
		std::unique_ptr<std::istream> pStr(sf.open(uri));
		char line[256];
		pStr->getline(line, 256);
		while (pStr->getline(line, 256))
		{
			Insert(line);
		}

		// ftp://ftp.cmegroup.com/pub/settle/nymex_future.csv
		uri.setScheme("ftp");
		uri.setHost("ftp.cmegroup.com");
		uri.setPath("/pub/settle/nymex_future.csv;type=a");
		pStr.reset(sf.open(uri));
		pStr->getline(line, 256);
		while (pStr->getline(line, 256))
		{
			Insert(line);
		}
	};

	void FuturesValueMySQLDAO::Delete()
	{
		sql::PreparedStatement *pstmt;
		pstmt = m_con->prepareStatement("CALL delete_DailyOptionValue()");
		pstmt->execute();
		delete pstmt;
	}

	void FuturesValueMySQLDAO::Insert(std::string line)
	{
		/// input: PRODUCT SYMBOL(0),CONTRACT MONTH(1),CONTRACT YEAR(2),CONTRACT DAY(3),CONTRACT,(4)PRODUCT DESCRIPTION(5),
		/// OPEN(6),HIGH(7),HIGH AB INDICATOR(8),LOW(9),LOW AB INDICATOR(10),LAST(11),LAST AB INDICATOR(12),SETTLE(13),PT CHG(14),
		/// EST. VOL(15),PRIOR SETTLE(16),PRIOR VOL(17),PRIOR INT(18),TRADEDATE(19)

		/// output: sym_tdate_id(0), contract_date(2,1), open(6), high(7), low(9), last(11), settle(13), 
		/// volume(15), open_int(18), tradedate(19)

		/// remove \r
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
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
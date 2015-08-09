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
#include "OptionFileMySQLDAO.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void OptionFileMySQLDAO::Connect()
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

	void OptionFileMySQLDAO::UploadOptions()
	{
		vector<string> vec;
		auto fileName = m_optFile.GetFileName();

		/// unzip the downloaded file
		char* zip;
		zip = getenv("ZIP");
		if (zip == NULL)
		{
			throw std::runtime_error("No zip utility");
		}

		/// compose unzip command
		
		auto dirindex = fileName.find_last_of("\\");
		auto dirName = fileName.substr(0, dirindex);
		char command[120];
		sprintf(command, "\"%s\" x -o%s %s", zip, dirName.c_str(), fileName.c_str());
		std::system(command);

		/// once the file is uncomressed then read and upload
		/// option data
		auto lastindex = fileName.find_last_of(".");
		auto rawName = fileName.substr(0, lastindex);

		std::ifstream infile(rawName.c_str());
		string line;
		/// skip the first header line
		if (infile.is_open()) getline(infile, line);
		if (infile.is_open())
		{
			while (getline(infile, line))
			{
				InsertOption(line);
				//std::future<void> f = std::async(std::launch::async, &OptionFileMySQLDAO::InsertOption, this, line);
			}
		}
		infile.close();
	};

	void OptionFileMySQLDAO::DeleteOption()
	{
		sql::PreparedStatement *pstmt;
		pstmt = m_con->prepareStatement("CALL delete_DailyOptionValue()");
		pstmt->execute();
		delete pstmt;
	}

	void OptionFileMySQLDAO::InsertOption(const std::string& line)
	{
		/// Input: Date(0)	ExpiryDate(1)	Symbol(2)	UnderlyingSymbol(3)	OSISymbol(4)	CallPut(5)	Strike(6) \
								/// Volume(7)	OpenInterest(8)	Bid(9)	Ask(10)	Last(11)
		/// 2015-01-23,2017-01-20,AAPLA20173115000,AAPL,AAPL170120C00115000,Call,115,148,11140,17.9,18.25,18.1
		
		LOG(INFO) << line << endl;
		std::vector<std::string> vec;
		splitLine(line, vec);

		auto volume = vec[7].empty() ? 0 : std::atoi(vec[7].c_str());
		if (volume == 0)
		{
			return;
		}

		string optSymbol = vec[2];  //AAPL7D17154900000
		string underlying = vec[3];
		string OSISymbol = vec[4];  //AAPL150102C00070000
		string optionStr = vec[5]; // Call
		auto stock_id = std::hash<std::string>()(vec[3]);
		auto opt = (optionStr.compare("Call") == 0) ? 1 : 2;
		auto tdate = dd::to_iso_extended_string(dd::from_string(vec[0]));
		auto mdate = dd::to_iso_extended_string(dd::from_string(vec[1]));
		auto strike = vec[6].empty() ? 0.0 : std::atof(vec[6].c_str());
		double tradePrice = vec[11].empty() ? 0.0 : std::atof(vec[11].c_str());
		auto askingPrice = vec[10].empty() ? 0.0 : std::atof(vec[10].c_str());
		auto bidPrice = vec[9].empty() ? 0.0 : std::atof(vec[9].c_str());
		auto openInterest = vec[8].empty() ? 0 : std::atoi(vec[8].c_str());
		/// now load data
		std::unique_ptr<sql::PreparedStatement> pstmt;
		try
		{
			pstmt.reset(m_con->prepareStatement("CALL insert_DailyOptionValue(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
			pstmt->setUInt64(1, stock_id);
			pstmt->setInt(2, opt);
			pstmt->setString(3, tdate);
			pstmt->setString(4, mdate);
			pstmt->setDouble(5, strike);
			pstmt->setDouble(6, tradePrice);
			pstmt->setDouble(7, askingPrice);
			pstmt->setDouble(8, bidPrice);
			pstmt->setInt(9, volume);
			pstmt->setInt(10, openInterest);
			pstmt->execute();
			pstmt->close();
		}
		catch (sql::SQLException &e)
		{
			pstmt->close();
			LOG(ERROR) << "Error loading data " << line << endl;
			LOG(ERROR) << "# ERR: " << e.what();
		}
	};

} /* namespace derivative */
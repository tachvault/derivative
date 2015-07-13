/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

// MonteCarlo_unittest.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <set>
#include <cpprest/asyncrt_utils.h>
#include <boost/filesystem.hpp> 

#include "Global.hpp"
#include "OptionFile.hpp"
#include "OptionXigniteDAO.hpp"
#include "OptionFileMySQLDAO.hpp"

using namespace derivative;

int main(int argc, char *args[])
{
	// start logging
	std::string log_dir;
	if (const char* env_p = std::getenv("LOG_DIR"))
	{
		log_dir = std::string(env_p);
		FLAGS_log_dir = log_dir.c_str();
		google::InitGoogleLogging("Derivative");
	}
	else
	{
		exit(1);
	}

	/// Get today's date
	dd::date date = dd::day_clock::local_day();
	if (argc == 2)
	{
		date = dd::from_string(args[1]);
	}
	string outputFileName = log_dir + std::string("\\") + dd::to_iso_string(date) + ".zip";
	
	/// define set of exchanges supported
	std::set<string_t> exchanges = { U("OPRA") };

	/// get only call options since call options are sufficient for implied volatility
	OptionFile::OptionType opt = OptionFile::OptionType::VANILLA_CALL;

	/// for each exchange supported
	for (auto& ex : exchanges)
	{
		/// construct optFile
		OptionFile optFile(ex, opt, date);
		optFile.SetFileName(outputFileName);

		/// construct XigniteOptionDAO to download file
		OptionXigniteDAO optXigniteDAO(optFile);

		/// call to get the file URL
		try
		{
			optXigniteDAO.RetriveOptFileName();
		}
		catch (std::exception& e)
		{
			LOG(ERROR) << "No Option file found for " << outputFileName << endl;
			exit(1);
		}

		/// now download file
		optXigniteDAO.RetriveOptFile();

		/// Get The MySQLDAO to insert downloaded data into Option table
		OptionFileMySQLDAO MySQLDAO(optFile);
		MySQLDAO.Connect();

		/// delete existing data
		//MySQLDAO.DeleteOption();

		/// Now load data;
		MySQLDAO.UploadOptions();
	}
}
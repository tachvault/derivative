/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <set>
#include <boost/filesystem.hpp> 

#include "Global.hpp"
#include "FuturesValueMySQLDAO.hpp"
#include "FuturesOptionValueMySQLDAO.hpp"

using namespace derivative;

int main(int argc, char *args[])
{

	if (argc < 2)
	{
		printf("Usage: CMEFileLoader.exe <future_file> <future option file>\n");
		return -1;
	}

	std::string futureFile = args[1];
	std::string futureOptionFile = args[2];

	/*
    /// insert data into futures table
	FuturesValueMySQLDAO futuresDAO(futureFile);
	futuresDAO.Connect();
	futuresDAO.Upload();
	*/

	/// insert data into futures option table
	FuturesOptionValueMySQLDAO futuresOptionDAO(futureOptionFile);
	futuresOptionDAO.Connect();
	futuresOptionDAO.Upload();
}
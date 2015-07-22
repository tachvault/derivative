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
	/// insert data into futures table
	FuturesValueMySQLDAO futuresDAO;
	LOG(INFO) << " Connecting to CME futures data" << std::endl;
	futuresDAO.Connect();
	LOG(INFO) << " Connected to CME " << std::endl;
	futuresDAO.Upload();
	LOG(INFO) << " Futures data loaded " << std::endl;
	
	/// insert data into futures option table
	FuturesOptionValueMySQLDAO futuresOptionDAO;
	LOG(INFO) << " Connecting to CME futures options data " << std::endl;
	futuresOptionDAO.Connect();
	LOG(INFO) << " Connected to CME " << std::endl;
	futuresOptionDAO.Upload();
	LOG(INFO) << " Futures option data loaded " << std::endl;
}
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
	futuresDAO.Connect();
	futuresDAO.Upload();
	
	/// insert data into futures option table
	FuturesOptionValueMySQLDAO futuresOptionDAO;
	futuresOptionDAO.Connect();
	futuresOptionDAO.Upload();
}
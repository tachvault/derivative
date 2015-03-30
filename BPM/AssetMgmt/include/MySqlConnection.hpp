/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MYSQLCONNECTION_H_
#define _DERIVATIVE_MYSQLCONNECTION_H_

#include <stdlib.h>
#include <iostream>

/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

namespace derivative
{
	/// MySQLConnection implements static methods for 
	/// establishing connection, closing connection,
	/// flushing data etc.
	class  MySQLConnection
	{
	public:

		static void InitConnection(sql::Driver *driver, std::unique_ptr<sql::Connection>& con)
		{
			/* Create a connection */
			driver = get_driver_instance();
			/// mysql-instance1.cmtulbltpl82.us-east-1.rds.amazonaws.com:3306
			con.reset(driver->connect("tcp://localhost:3306", "root", "root"));
			/* Connect to the MySQL derivative database */
			con->setSchema("derivative");
		}
	};

}

/// namespace derivative

#endif 
///_DERIVATIVE_MYSQLCONNECTION_H_

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESMYSQLDAO_H_
#define _DERIVATIVE_FUTURESVALUEMYSQLDAO_H_

#include <memory>
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	class FuturesValueMySQLDAO					 
	{
	public:

		FuturesValueMySQLDAO(const std::string& file)
			:m_furFile(file)
		{}
		
		/// connect to MySQL
		void Connect();

		/// load data into MySQL
		void Upload();

		/// Delete all existing data
		void Delete();

	private:

		void Insert(const std::string& line);
		
		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		std::string m_furFile;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESVALUEMYSQLDAO_H_ */
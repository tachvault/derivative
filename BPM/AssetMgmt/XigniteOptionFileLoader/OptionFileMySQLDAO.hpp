/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_OPTIONFILEMYSQLDAO_H_
#define _DERIVATIVE_OPTIONFILEMYSQLDAO_H_

#include <memory>
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"
#include "DException.hpp"
#include "OptionFile.hpp"

namespace derivative
{
	class OptionFileMySQLDAO					 
	{
	public:

		enum {TYPEID = CLASS_OPTIONFILEMYSQLDAO_TYPE};

		OptionFileMySQLDAO(OptionFile& opt)
			:m_optFile(opt)
		{}
		
		/// connect to MySQL
		void Connect();

		/// load data into MySQL
		void UploadOptions();

		/// Delete all existing data
		void DeleteOption();

	private:

		void InsertOption(const std::string& line);
		
		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;

		OptionFile m_optFile;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_OPTIONFILEMYSQLDAO_H_ */

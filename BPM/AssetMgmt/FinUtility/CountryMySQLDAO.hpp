/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_COUNTRYMYSQLDAO_H_
#define _DERIVATIVE_COUNTRYMYSQLDAO_H_

#include <memory>
#include "Country.hpp"
#include "MySqlConnection.hpp"
#include "Global.hpp"

namespace derivative
{
	// Data Access Object for Country entity. 
	class CountryMySQLDAO 			 
	{
	public:

		enum {TYPEID = CLASS_COUNTRYMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator CountryMySQLDAO object
		CountryMySQLDAO()
		{
		}

		/// Get all the Country objects
		void GetEntities(std::deque<Country> &data);

	private:
		
		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_COUNTRYMYSQLDAO_H_ */

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_CURRENCYMYSQLDAO_H_
#define _DERIVATIVE_CURRENCYMYSQLDAO_H_

#include <memory>
#include "Currency.hpp"
#include "MySqlConnection.hpp"
#include "Name.hpp"
#include "Global.hpp"

namespace derivative
{
	// Data Access Object for Currency entity. 
	class CurrencyMySQLDAO				 
	{
	public:

		enum {TYPEID = CLASS_CURRENCYMYSQLDAO_TYPE};

		/// constructor.
		CurrencyMySQLDAO()
		{
		}
		
		/// Get all the Currencies
		void GetEntities(std::deque<Currency> &data);

	private:

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_CURRENCYMYSQLDAO_H_ */

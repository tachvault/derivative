/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGEMYSQLDAO_H_
#define _DERIVATIVE_EXCHANGEMYSQLDAO_H_

#include <memory>
#include "Exchange.hpp"
#include "MySqlConnection.hpp"
#include "Global.hpp"

namespace derivative
{
	// Data Access Object for Exchange entity. 
	class ExchangeMySQLDAO 			 
	{
	public:

		enum {TYPEID = CLASS_EXCHANGEMYSQLDAO_TYPE};

		/// constructors.
		/// Constructor with Exemplar for the Creator ExchangeMySQLDAO object
		ExchangeMySQLDAO()
		{
		}

		/// Get all the Exchange objects
		void GetEntities(std::deque<Exchange> &data);

	private:
		
		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGEMYSQLDAO_H_ */

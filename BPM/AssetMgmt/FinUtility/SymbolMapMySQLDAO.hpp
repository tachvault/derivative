/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SYMBOLMAPMYSQLDAO_H_
#define _DERIVATIVE_SYMBOLMAPMYSQLDAO_H_

#include <memory>
#include "MySqlConnection.hpp"
#include "Global.hpp"
#include "ExchangeExt.hpp"


#if defined FINUTILITY_EXPORTS
#define FIN_UTIL_DLL_API __declspec(dllexport)
#else
#define FIN_UTIL_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	// Data Access Object for ExchangeExt entity.
	class FIN_UTIL_DLL_API ExchangeExtMySQLDAO				 
	{
	public:

		enum {TYPEID = CLASS_SYMBOLMAPMYSQLDAO_TYPE};

		/// constructors.
		ExchangeExtMySQLDAO()
		{}

		/// return the alias symbol used by source given the source and symbol used by
		/// derivative (this software)
		void GetExchangeExt(ExchangeExt::ExchangeExtType& symMap);

	private:

		sql::Driver *m_driver;

		std::unique_ptr<sql::Connection> m_con;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_SYMBOLMAPMYSQLDAO_H_ */

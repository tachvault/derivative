/*
*          Copyright Andrey Semashev 2007 - 2013.
* Distributed under the Boost Software License, Version 1.0.
*    (See accompanying file LICENSE_1_0.txt or copy at
*          http://www.boost.org/LICENSE_1_0.txt)
*
*  Customized Nathan Muruganantha. 2013.
*/

#ifndef _DERIVATIVE_DEXCEPTION_H_
#define _DERIVATIVE_DEXCEPTION_H_

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include <string>
#include <iostream>
#include <functional> 
#include <algorithm>
#include <mutex>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

using namespace logging::trivial;

namespace derivative
{
	/// Singleton wrapper class instanced first time called.	
	/// Uses Boost::Logger to perform actual logging.
	/// Currently designed to use a fixed filename in the
	/// process directory
	class  LogHandler
	{
	public:

		static shared_ptr<LogHandler>& instance()
		{
			try
			{
				/// Use std::call_once to make sure that the LogHandler constructor
				/// is run only once
				// private default constructor and return
				std::call_once(m_flag, [](){ m_instance = make_shared<LogHandler>(); });
			}
			/// catch all types of exception 
			catch(std::runtime_error &e)
			{
				throw e;
			}

			/// if no exception is thrown means that the constructor ran
			/// once without error. Therefore we can safely return the 
			return m_instance;      
		}

		/// return actual boost severity_logger
		src::severity_logger< severity_level > & GetLog()
		{
			return lg;
		}		

		/// Fixed Me: Need to be public in order to be used
		/// with std::make_shared.
		/// Need to be improved to
		/// - Use paramerized directory name
		/// - Use paramerized file name
		/// - Include severity level in logging
		LogHandler()
		{		

			logging::add_file_log
				(
				keywords::file_name = "derivative.log",/*< file name pattern >*/
				keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
				keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
			    keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
				);
			    logging::add_common_attributes();

		}

	private:

		/// singleton instance
		static shared_ptr<LogHandler> m_instance;

		/// call once flag to make sure constructor executed
		/// only once.
		static std::once_flag m_flag;

		/// boost severity logger
		src::severity_logger< severity_level > lg;
	};

	shared_ptr<LogHandler> LogHandler::m_instance;
	std::once_flag LogHandler::m_flag;

/// Define macros for all severity levels
#define trclog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), trace)
#define dbglog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), debug)
#define warnlog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), warning)
#define errlog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), error)
#define fatallog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), fatal)
#define infolog BOOST_LOG_SEV(LogHandler::instance()->GetLog(), info)

}
/* namespace derivative */

#endif /* _DERIVATIVE_LOGHANDLER_H_ */

/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SERVICELOGGER_H_
#define _DERIVATIVE_SERVICELOGGER_H_

#include <memory>

#include "Global.hpp"
#include "IMessage.hpp"
#include "ThreadSafeQueue.hpp"

namespace derivative
{
	/// ServiceLogger is responsible logging the request and response into
	/// file or database.
	class ServiceLogger
	{
	public:	

		struct request
		{
			const std::string token;
			const std::string datetime;
			const std::string url;

			request(const std::string& t, const std::string& dt, const std::string& u)
				:token(t), datetime(dt), url(u)
			{}
		};

		/// constructor
		ServiceLogger();

		 /// destructor
		~ServiceLogger();
		
		void LogMessage(const std::string& token, const std::string& datetime, const std::string& url);

		void WriteRequest();

	private:

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(ServiceLogger);

		/// A thread safe queue push request
		threadsafe_queue<request> m_reqQueue;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_SERVICELOGGER_H_ */

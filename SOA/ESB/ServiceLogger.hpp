/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SERVICELOGGER_H_
#define _DERIVATIVE_SERVICELOGGER_H_

#include <memory>

#include "Global.hpp"
#include "IMessage.hpp"

namespace derivative
{
	/// ServiceLogger is responsible logging the request and response into
	/// file or database.
	class ServiceLogger
	{
	public:	

		/// constructor
		ServiceLogger();

		 /// destructor
		~ServiceLogger();
		
		void LogMessage(const std::shared_ptr<IMessage>& msg);

	private:

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(ServiceLogger);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_SERVICELOGGER_H_ */

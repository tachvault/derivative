/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IREQUESTINTERCEPTOR_H_
#define _DERIVATIVE_IREQUESTINTERCEPTOR_H_
#pragma once

#include "ClassType.hpp"

namespace derivative
{
	/// IRequestInterceptor interface for handlers that intercept
	/// service request from clients. (Ex: REST, TCP, SOAP).
	class IRequestInterceptor
	{
	public:

		enum {TYPEID = INTERFACE_REQUESTINTERCEPTOR_TYPE};

		/// start the service
		virtual void StartInterceptor() = 0;

		/// shutdown the service
		virtual void Shutdown() = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IRequestInterceptor() 
		{
		}  
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IREQUESTINTERCEPTOR_H_ */

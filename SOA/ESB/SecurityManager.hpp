/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SECURITYMANAGER_H_
#define _DERIVATIVE_SECURITYMANAGER_H_

#include <memory>

#include "Global.hpp"
#include "IMessage.hpp"

namespace derivative
{
	class UserInfoHolder;

	/// SecurityManager is responsible for authorizing request.
	class SecurityManager
	{

	public:	

		/// constructor
		SecurityManager();

		 /// destructor
		~SecurityManager();
		
		/// Authorize request based on message content and user info
		/// stored during client authorization sessions (not part of
		/// pricing engine).
		bool AuthorizeRequest(const std::string& token);

	private:

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(SecurityManager);

		std::unique_ptr<UserInfoHolder> m_userInfo;
	};
	
} /* namespace derivative */

#endif /* _DERIVATIVE_SECURITYMANAGER_H_ */

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#if defined _WIN32
/// disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
/// disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#endif


#ifndef _DERIVATIVE_USERINFOHOLDER_H_
#define _DERIVATIVE_USERINFOHOLDER_H_

#include <memory>

#include "ClassType.hpp"
#include "Global.hpp"

namespace derivative
{
	class UserInfoHolder
	{

	public:

		enum { TYPEID = CLASS_USERINFOHOLDER_TYPE};	

		/// default constructor
		UserInfoHolder();

		/// desstructor
		~UserInfoHolder();

		void LoadData();

		/// check if the given token is valid
		bool IsValid(const std::string& token);

	private:	

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(UserInfoHolder);

		std::vector<string> m_tokens;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_CURRENCYDATALOADER_H_ */

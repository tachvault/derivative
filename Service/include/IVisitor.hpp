/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IVISITOR_H_
#define _DERIVATIVE_IVISITOR_H_

#include <cpprest/json.h>
#include "global.hpp"
#include "ClassType.hpp"
#include "IObject.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef MESSAGES_EXPORTS
#ifdef __GNUC__
#define MESSAGES_DLL_API __attribute__ ((dllexport))
#else
#define MESSAGES_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define MESSAGES_DLL_API __attribute__ ((dllimport))
#else
#define MESSAGES_DLL_API __declspec(dllimport)
#endif
#endif
#define MESSAGES_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MESSAGES_DLL_API __attribute__ ((visibility ("default")))
#define MESSAGES_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define MESSAGES_DLL_API
#define MESSAGES_DLL_LOCAL
#endif
#endif

using namespace web;

/// This interface represents messages, as sent and received
/// by the messaging services component. 

namespace derivative
{
	class IMessage;
	class MESSAGES_DLL_API IVisitor
	{
	public:

		enum { TYPEID = INTERFACE_VISITOR_TYPE };

		virtual void Visit(const std::shared_ptr<IMessage>& msg, json::value& out) = 0;

		/// virtual inline dtor just in case
		virtual ~IVisitor()
		{};
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IVISITOR_H_ */


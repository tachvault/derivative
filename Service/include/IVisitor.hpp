/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IVISITOR_H_
#define _DERIVATIVE_IVISITOR_H_

#include <cpprest/json.h>
#include "global.hpp"
#include "ClassType.hpp"
#include "IObject.hpp"

using namespace web;

/// This interface represents messages, as sent and received
/// by the messaging services component. 

namespace derivative
{
	class IMessage;
	class IVisitor
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


/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IMESSAGESINK_H_
#define _DERIVATIVE_IMESSAGESINK_H_

#include "global.hpp"
#include "IObject.hpp"
#include "ClassType.hpp"
#include "IMessage.hpp"

/// This interface is implemented by objects which want to
/// recieve messages from IMessageDispatcher.  The
/// receive messages of a particular type, which are then
/// passed on using the Dispatch method. 

namespace derivative
{
	class IMessageSink
	{
	public:

		enum { TYPEID = INTERFACE_MESSAGESINK_TYPE };

		/// The activate member function is called when a stateful session bean instance 
		/// is activated from its "passive" state.
		virtual void Activate(const std::deque<boost::any>& agrs) = 0;

		/// The passivate member function is called before a stateful session bean instance 
		/// enters the "passive" state.
		virtual void Passivate() = 0;

		/// This method is called by the IMessageDispatcher
		/// interface whenever a message is received of the type
		virtual void Dispatch(std::shared_ptr<IMessage>& msg) = 0;

	};
}

#endif

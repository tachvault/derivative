/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SERVICEROUTER_H_
#define _DERIVATIVE_SERVICEROUTER_H_

#include <memory>

#include "Global.hpp"
#include "IMessage.hpp"

namespace derivative
{
	/// ServiceRouter is responsible for routing the request to a given
	/// app server based on routing logic. The Config.hpp should provide
	/// the destinations to this ServiceRouter.
	class ServiceRouter
	{

	public:	

		/// constructor
		ServiceRouter();

		 /// destructor
		~ServiceRouter();
		
		/// route message based on routing logic function.
		/// Currently the routing logic is constant and is round
		/// robin. When more custom routing algorithms are developed
		/// this function can be made generic with routing algorithm
		/// as template parameter.
		void RouteMessage(std::shared_ptr<IMessage>& msg);

	private:

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(ServiceRouter);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_SERVICEROUTER_H_ */

/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IMAKE_H_
#define _DERIVATIVE_IMAKE_H_

#include <deque>

#include "boost/any.hpp"
#include "IObject.hpp"
#include "Global.hpp"

namespace derivative
{

	class IMake : virtual public IObject
	{

	public:

		enum {TYPEID = INTERFACE_MAKE_TYPE};
				
		/// Make is a "virtual constructor" or "factory" method.  Objects
		/// implementing IMake should return an object of the same type as
		/// themselves, having the supplied Name. 
		virtual std::shared_ptr<IMake> Make (const Name &nm) = 0;

		/// Caller can make use of the method to construct the complete object
		/// rather than using setter functions repeatedly.
		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs) = 0;
		
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IMAKE_H_ */

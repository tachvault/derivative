/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IIRDATASRC_H_
#define _DERIVATIVE_IIRDATASRC_H_

#include <algorithm>
#include <set>
#include "ClassType.hpp"
#include "Global.hpp"

namespace derivative
{
	/// Base interface for any domain type that
	/// can provide data to construct IR curve
	class  IIRDataSrc
	{
	public:

		enum {TYPEID = INTERFACE_IRDATASRC_TYPE};

		typedef std::pair<int, double> cashflowType;

		typedef std::map<int, double> cashFlowSetType;

		virtual void generateCashFlow() = 0;

		virtual std::shared_ptr<IIRDataSrc::cashFlowSetType> getCashFlowMap() const = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IIRDataSrc()
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IIRDATASRC_H_ */

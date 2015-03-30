/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IZEROCOUPONBOND_H_
#define _DERIVATIVE_IZEROCOUPONBOND_H_

#include <memory>
#include "IBond.hpp"

namespace derivative
{
	class Country;
	class Exchange;

	/// Interface for ZeroCouponBond class
	class IZeroCouponBond: public virtual IBond
	{

	public:

		enum {TYPEID = INTERFACE_ZEROCOUPONBOND_TYPE};

		static Name ConstructName(const std::string& symbol)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
		}
				
	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IZeroCouponBond() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IZEROCOUPONBOND_H_ */

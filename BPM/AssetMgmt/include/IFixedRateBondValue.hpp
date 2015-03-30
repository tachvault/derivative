/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IFIXEDRATEBONDVALUE_H_
#define _DERIVATIVE_IFIXEDRATEBONDVALUE_H_

#include <string>
#include <memory>

#include "IAssetValue.hpp"
#include "IFixedRateBond.hpp"
#include "IBondValue.hpp"

namespace derivative
{
	/// IFixedRateBondValue interface exposes value of a coupon bond
	/// at a given time. 
	class IFixedRateBondValue : virtual public IBondValue
	{
	public:

		enum {TYPEID = INTERFACE_FIXEDRATEBONDVALUE_TYPE};

		static Name ConstructName(const std::string& symbol, const dd::date& tradedate)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("tradedate"), boost::any_cast<dd::date>(tradedate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, dd::date& tradedate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto j = keys.find("tradedate");
			tradedate = boost::any_cast<dd::date>(j->second);
		}

		virtual void convert( istringstream  &input) = 0;
				
		virtual std::shared_ptr<IFixedRateBond> GetFixedRateBond() const = 0;

		virtual void SetFixedRateBond(std::shared_ptr<IFixedRateBond> bond) = 0;

		/// you should know the derived type if you are deleting.
		virtual ~IFixedRateBondValue() 
		{
		}  
	};

	istringstream& operator >> (istringstream& input, std::shared_ptr<IFixedRateBondValue>& bondVal)
	{
		bondVal->convert(input);
		return input;
	}
}

/* namespace derivative */

#endif /* _IDERIVATIVE_FIXEDRATEBONDVALUE_H_ */
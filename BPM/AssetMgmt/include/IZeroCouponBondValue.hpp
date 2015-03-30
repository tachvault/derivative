/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IZEROCOUPONBONDVALUE_H_
#define _DERIVATIVE_IZEROCOUPONBONDVALUE_H_

#include <string>
#include <memory>

#include "IBondValue.hpp"
#include "IZeroCouponBond.hpp"

namespace derivative
{
	/// IZeroCouponBondValue interface exposes value of a bond
	/// at a given time. Current value of bond
	/// and day end of the bond will realize IZeroCouponBondValue
	class IZeroCouponBondValue : virtual public IBondValue
	{

	public:

		enum {TYPEID = INTERFACE_ZEROCOUPONBONDVALUE_TYPE};

		static Name ConstructName(const std::string& symbol, const dd::date& tradedate)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(tradedate)));
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

		virtual std::shared_ptr<IZeroCouponBond> GetZeroCouponBond() const = 0;

		virtual void SetZeroCouponBond(std::shared_ptr<IZeroCouponBond> bond) = 0;

	    /// you should know the derived type if you are deleting.
		virtual ~IZeroCouponBondValue() 
		{
		}  
	};

	istringstream& operator >> (istringstream& input, std::shared_ptr<IZeroCouponBondValue>& bondVal)
	{
		bondVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_ZEROCOUPONBONDVALUE_H_ */
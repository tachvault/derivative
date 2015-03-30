/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IFIXEDRATEBOND_H_
#define _DERIVATIVE_IFIXEDRATEBOND_H_

#include <memory>
#include "IBond.hpp"
#include "Country.hpp"
#include "Exchange.hpp"

namespace derivative
{
	/// Interface for FixedRateBond class
	class IFixedRateBond: public virtual IBond
	{

	public:

		enum {TYPEID = INTERFACE_FIXEDRATEBOND_TYPE};

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
		
		enum CouponPeriodType
		{
			NoCoupons= 0,
			Annual = 12,
			Semiannual = 6,
			TriAnnual = 4,
			Quarterly = 3, 
			BiMonthly = 2,
			Monthly = 1
		};

		virtual CouponPeriodType GetCouponPeriod() const  = 0;
				
		virtual double GetCouponRate() const  = 0;

		virtual void SetCouponPeriod(const CouponPeriodType& period) = 0;

		virtual void SetCouponRate(double rate) = 0;
		
	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IFixedRateBond() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IFIXEDRATEBOND_H_ */

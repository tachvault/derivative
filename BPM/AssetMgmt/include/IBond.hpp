/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IBOND_H_
#define _DERIVATIVE_IBOND_H_

#include <memory>
#include "IPrimitiveSecurity.hpp"
#include "DayCount.hpp"

namespace derivative
{
	class Country;
	class Exchange;

	/// Interface for ZeroCouponBond class
	class IBond: public virtual IPrimitiveSecurity
	{

	public:

		enum {TYPEID = INTERFACE_BOND_TYPE};

		enum CategoryType
		{
			Government = 1,
			Municipal = 2,
			Coporate = 3
		};		

		virtual const Country& GetCountry() const  = 0;

		virtual CategoryType GetCategory() const = 0;

		virtual DayCount::DayCountType GetDayCount() const = 0;

		virtual double GetFaceValue() const = 0;

		virtual void SetCountry(const Country& cntry) = 0;

		virtual void SetFaceValue(double val) = 0;

		virtual void SetCategory(const CategoryType& cat) = 0;

		virtual void SetDayCount(const DayCount::DayCountType& dayCnt)  = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IBond() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IZEROCOUPONBOND_H_ */

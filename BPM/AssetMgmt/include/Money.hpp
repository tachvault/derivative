/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MONEY_H_
#define _DERIVATIVE_MONEY_H_

#include "Currency.hpp"

#if defined PRIMARYASSET_EXT_EXPORTS
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#endif

namespace derivative 
{
	/// Cash value. Money is represented as value type
	/// class. (There is no Name associated with the class)
	class  PRIMARYASSET_EXT_API Money 
	{
	public:

		/// Constructors		
		Money();
		Money(const Currency& currency, double value);
		Money(double value, const Currency& currency);


		/// getter methods
		const Currency& GetCurrency() const;

		double value() const;

		/// convert into target currency
		Money Convert(const Currency& target) const; 

		/// Overloaded arithmetic operators
		Money operator+() const;
		Money operator-() const;
		Money& operator+=(const Money&);
		Money& operator-=(const Money&);
		Money& operator*=(double);
		Money& operator/=(double);	

	private:		

		double m_value;
		Currency m_currency;

		friend PRIMARYASSET_EXT_API Money operator+(const Money& m1, const Money& m2);
		friend PRIMARYASSET_EXT_API Money operator-(const Money& m1, const Money& m2);
	};


	// Operator overloads
	PRIMARYASSET_EXT_API Money operator+(const Money&, const Money&);
	PRIMARYASSET_EXT_API Money operator-(const Money&, const Money&);
	PRIMARYASSET_EXT_API Money operator*(const Money&, double);
	PRIMARYASSET_EXT_API Money operator*(double, const Money&);
	PRIMARYASSET_EXT_API Money operator/(const Money&, double);
	PRIMARYASSET_EXT_API double operator/(const Money&, const Money&);

	PRIMARYASSET_EXT_API bool operator==(const Money&, const Money&);
	PRIMARYASSET_EXT_API bool operator!=(const Money&, const Money&);
	PRIMARYASSET_EXT_API bool operator<(const Money&, const Money&);
	PRIMARYASSET_EXT_API bool operator<=(const Money&, const Money&);
	PRIMARYASSET_EXT_API bool operator>(const Money&, const Money&);
	PRIMARYASSET_EXT_API bool operator>=(const Money&, const Money&);

	PRIMARYASSET_EXT_API bool close(const Money&, const Money&, int n = 42);
	PRIMARYASSET_EXT_API bool close_enough(const Money&, const Money&, int n = 42);

	PRIMARYASSET_EXT_API Money operator*(double, const Currency&);
	PRIMARYASSET_EXT_API Money operator*(const Currency&, double);

	/// formatting
	PRIMARYASSET_EXT_API std::ostream& operator<<(std::ostream&, const Money&);	

}


#endif

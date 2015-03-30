/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_TSLINEAR_H_
#define _DERIVATIVE_TSLINEAR_H_

#include "TSBootStrap.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

namespace derivative
{
	/// Term structure using linear interpolation of zero coupon bond prices.
	class PRIMARYASSET_EXT_API TSLinear : public TSBootstrap
	{
	public:

		enum {TYPEID = CLASS_TSLINEAR_TYPE};

		/// General constructor.
		inline TSLinear(const Array<double,1>& xT,               ///< Time line of maturities .
			const Array<double,1>& xB                ///< Time T(0) forward zero coupon bond prices for these maturities - thus the first bond price is always 1
			) : TSBootstrap(xT.copy(),xB.copy()) 
		{ };
		
		/// Conversion constructor.
		TSLinear(const Array<double,1>& xT,const TermStructure& xts);
		
		/// Virtual copy constructor
		virtual std::shared_ptr<TermStructure> pointer_to_copy() const;
		
		/** Function returning the interpolated value, which for term structures of
		interest rates is the maturity t, time T(0) forward zero coupon bond price. */
		virtual double operator()(double t) const;
		
		/// For nested Visitor pattern.
		virtual const std::string& name() const;

		virtual ~TSLinear();

	private:

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(TSLinear);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_TSLINEAR_H_ */



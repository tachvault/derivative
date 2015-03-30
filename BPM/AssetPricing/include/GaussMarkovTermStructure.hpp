/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GAUSSMARKOVTERMSTRUCTURE_H_
#define _DERIVATIVE_GAUSSMARKOVTERMSTRUCTURE_H_

#include "GaussianHJM.hpp"
#include "TermStructure.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRICINGENGINE_EXPORTS
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllexport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllimport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif
#endif
#define PRICINGENGINE_LOCAL
#else
#if __GNUC__ >= 4
#define PRICINGENGINE_DLL_API __attribute__ ((visibility ("default")))
#define PRICINGENGINE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRICINGENGINE_DLL_API
#define PRICINGENGINE_LOCAL
#endif
#endif

namespace derivative
{
	using blitz::Array;
	using blitz::firstDim;

	class PRICINGENGINE_DLL_API GaussMarkovTermStructure : public TermStructure
	{
	public:

		enum {TYPEID = CLASS_GAUSSMARKOVTERMSTRUCTURE_TYPE};

		/// Constructor.
		GaussMarkovTermStructure(double xtoday,                         ///< Current time for which this term structure applies.
			const GaussianHJM& xhjm,               ///< Underlying Gauss/Markov Heath/Jarrow/Morton model.
			const Array<double,1>& xstatevariables ///< State variables describing the state of the model at the current time.
			);
		
		/// Virtual copy constructor
		virtual std::shared_ptr<TermStructure> pointer_to_copy() const;
		
		/// Reinitialise (if necessary) when zero coupon bond prices have changed
		virtual void reinitialise();
		
		/** Function returning the interpolated value, which for term structures of
		interest rates is the maturity t, time T(0) forward zero coupon bond price. */
		virtual double operator()(double t) const;    
		
		/// For nested Visitor pattern.
		virtual void accept(QFNestedVisitor& visitor) const;
		
		virtual const std::string& name() const;

	private:
		double                   today;
		const GaussianHJM&         hjm;
		Array<double,1> statevariables;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_GAUSSMARKOVTERMSTRUCTURE_H_ */



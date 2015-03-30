/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_FLATTERMSTRUCTURE_H_
#define _DERIVATIVE_FLATTERMSTRUCTURE_H_

#include <vector>
#include <memory>

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include "QFVisitor.hpp"
#include "TermStructure.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
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
	using blitz::Array;
	using blitz::firstDim;

	class DeterministicCashflow;

	/// Flat (constant yield) term structure.
	class PRIMARYASSET_EXT_API FlatTermStructure : public TermStructure
	{
		enum {TYPEID = CLASS_FLATTERMSTRUCTURE_TYPE};

	public:
		/// Flat term structure constructor.
		FlatTermStructure(double lvl,    ///< Flat level of continuously compounded interest rates.
			double tzero,  ///< Start of maturity time line.
			double horizon ///< End of maturity time line.
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
		double level;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(FlatTermStructure);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_FLATTERMSTRUCTURE_H_ */


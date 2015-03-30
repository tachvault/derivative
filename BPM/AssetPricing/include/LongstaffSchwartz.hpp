/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_LONGSTAFFSCHWARTZ_H_
#define _DERIVATIVE_LONGSTAFFSCHWARTZ_H_

#include <vector>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <boost/function.hpp>
#include "Regression.hpp"
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
	using blitz::Range;
	using blitz::fromStart;
	using blitz::firstDim;
	using blitz::secondDim;
	using blitz::thirdDim;
	using blitz::firstIndex;
	using blitz::secondIndex;

	/// Longstaff/Schwartz exercise boundary, single state variable, 
	/// deterministic discounting, polynomial least squares fit.
	class PRICINGENGINE_DLL_API LongstaffSchwartzExerciseBoundary1D
	{
	public:
		/// Constructor using polynomial regression.
		LongstaffSchwartzExerciseBoundary1D(const Array<double,1>& timeline,
			const TermStructure& xts,
			const Array<double,2>& state_variable_paths,
			std::function<double (double)> payoff,
			int degree,
			bool positive_payoff_only = true);
		/// Destructor.
		~LongstaffSchwartzExerciseBoundary1D();
		/// Apply the exercise to a Monte Carlo path.
		double apply(const Array<double,1>& path) const;

	private:
		std::vector<std::shared_ptr<PolynomialLeastSquaresFit> > fit;  ///< Exercise boundary functions (one for each point on the time line)
		Array<double,1>                           T;  ///< Time line
		const TermStructure&                     ts;
		std::function<double (double)>          f;
		bool                          positive_only;  ///< True if option should be exercised only if the payoff is positive.

	};

	/** Longstaff/Schwartz exercise boundary, arbitrary number of state variables, discounting by numeraire, 
	arbitrary set of basis functions for the least squares fit.
	*/
	class PRICINGENGINE_DLL_API LongstaffSchwartzExerciseBoundary 
	{
	public:
		/** Constructor. state_variable_paths is a (paths,timepoints,state variables) array of state variable realisations. 
		numeraire_values is a (paths,timepoints) array of numeraire value realisations. */
		LongstaffSchwartzExerciseBoundary(const Array<double,1>& timeline,
			const Array<double,3>& state_variable_paths,
			const Array<double,2>& numeraire_values,
			std::function<double (double,const Array<double,1>&)> payoff,
			const std::vector<std::function<double (double,const Array<double,1>&)> >& xbasis_functions,
			bool positive_payoff_only = true);
		/// Destructor.
		~LongstaffSchwartzExerciseBoundary();
		/// Apply the exercise to a Monte Carlo path.
		double apply(const Array<double,2>& path,const Array<double,1>& numeraire_values) const;
		inline int number_of_state_variables() const 
		{ return number_of_variables; 
		};
		inline const Array<double,1>& get_timeline() const
		{ 
			return T; 
		};

	private:
		const std::vector<std::function<double (double,const Array<double,1>&)> >& basis_functions; ///< Vector of basis functions for the regression.
		std::vector<std::shared_ptr<LeastSquaresFit> >                         fit; ///< Exercise boundary functions (one for each point on the time line)
		Array<double,1>                                         T; ///< Time line
		std::function<double (double,const Array<double,1>&)> f; ///< Early exercise payoff function taking time and state variables as arguments.
		bool                                        positive_only; ///< True if option should be exercised only if the payoff is positive.
		int                                   number_of_variables;

	};

	PRICINGENGINE_DLL_API void add_polynomial_basis_function(std::vector<std::function<double (double,const Array<double,1>&)> >& basisfunctions,const Array<int,1>& p);
	PRICINGENGINE_DLL_API void add_polynomial_basis_function(std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> >& basisfunctions,const Array<int,1>& p);

	/** Longstaff/Schwartz exercise boundary, arbitrary number of state variables (including path history), discounting by numeraire, 
	arbitrary set of basis functions for the least squares fit. Basis functions take as arguments observation time points and
	observed state variable values, where the latter is a (state variables) X (observation time points) Array.
	*/
	class PRICINGENGINE_DLL_API RegressionExerciseBoundary 
	{
		class BasisFunction 
		{
		public:
			inline BasisFunction(const std::function<double (const Array<double,1>&,const Array<double,2>&)> xf,const Array<double,1>& xT)
				: f(xf),T(xT) { };
			double operator()(const Array<double,1>& x) const;

		private:
			const std::function<double (const Array<double,1>&,const Array<double,2>&)> f;
			Array<double,1>                                                               T;

		};
		std::vector<BasisFunction*> basis; ///< Vector of BasisFunction objects for LeastSquaresFit
		void initialise(const Array<double,3>& state_variable_paths,const Array<double,2>& numeraire_values,std::function<double (const Array<double,1>&,const Array<double,2>&)> payoff,bool positive_payoff_only);

	public:
		/** Constructor. state_variable_paths is a (paths,timepoints,state variables) array of state variable realisations. 
		numeraire_values is a (paths,timepoints) array of numeraire value realisations. */
		RegressionExerciseBoundary(const Array<double,1>& timeline,
			const Array<double,3>& state_variable_paths,
			const Array<double,2>& numeraire_values,
			std::function<double (const Array<double,1>&,const Array<double,2>&)> payoff,
			const std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> >& xbasis_functions,
			bool positive_payoff_only = true,
			bool ini = true);
		/// Destructor.
		virtual ~RegressionExerciseBoundary();
		/// Apply the exercise to a Monte Carlo path.
		virtual double apply(const Array<double,2>& path,const Array<double,1>& numeraire_values);
		inline int number_of_state_variables() const { return number_of_variables; };
		inline const Array<double,1>& get_timeline() const { return T; };

	protected:
		const std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> >& basis_functions; ///< Vector of basis functions for the regression.
		std::vector<std::shared_ptr<LeastSquaresFit> >                         fit; ///< Exercise boundary functions (one for each point on the time line)
		Array<double,1>                                         T; ///< Time line
		std::function<double (const Array<double,1>&,const Array<double,2>&)> f; ///< Early exercise payoff function (not discounted!) taking time and state variable history as arguments.
		bool                                        positive_only; ///< True if option should be exercised only if the payoff is positive.
		int                                   number_of_variables;

	};

	PRICINGENGINE_DLL_API double LSArrayAdapter(double t,const Array<double,1>& x,std::function<double (double)> f,int idx);
	PRICINGENGINE_DLL_API double LSArrayAdapterT(double t,const Array<double,1>& x,std::function<double (double,double)> f,int idx);
	PRICINGENGINE_DLL_API double REBAdapter(const Array<double,1>& T,const Array<double,2>& x,std::function<double (double)> f,int idx); 
	PRICINGENGINE_DLL_API double REBAdapterT(const Array<double,1>& T,const Array<double,2>& x,std::function<double (double,double)> f,int idx); 

} /* namespace derivative */

#endif /* _DERIVATIVE_LONGSTAFFSCHWARTZ_H_ */
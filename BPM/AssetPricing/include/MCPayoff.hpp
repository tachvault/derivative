/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_MCPAYOFF_H_
#define _DERIVATIVE_MCPAYOFF_H_

#include <vector>
#include <list>
#include <memory>

#include "MSWarnings.hpp"
#include "MCGatherer.hpp"
#include "TermStructure.hpp"
#include "ClassType.hpp"

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
	using blitz::firstDim;
	using blitz::secondDim;

	template <class target_price_process,class controlvariate_price_process,class cv_random_variable> class MCControlVariateMapping;

	/** Abstract base class defining the common interface for classes mapping a path asset value (or state variable) realisations
	and numeraire value realisations to a discounted payoff.
	*/
	class PRICINGENGINE_DLL_API MCPayoff
	{
	public:

		enum {TYPEID = CLASS_MCPAYOFF_TYPE};

		Array<double,1> timeline;  ///< Time line collecting all event dates.
		Array<int,2>       index;  ///< A 2 x N matrix of indices, where each column represents the

		///< indices of an (asset,time) combination affecting the payoff.
		inline MCPayoff(const Array<double,1>& xtimeline,const Array<int,2>& xindex) 
			: timeline(xtimeline),index(xindex)
		{ };

		inline MCPayoff(const Array<double,1>& xtimeline,int number_of_indices) 
			: timeline(xtimeline),index(2,number_of_indices)
		{ };

		inline MCPayoff(int number_of_event_dates,int number_of_indices) 
			: timeline(number_of_event_dates+1),index(2,number_of_indices) 
		{ };

		/// Calculate discounted payoff.
		virtual double operator()(const Array<double,1>& underlying_values, ///< Underlying values for the (asset,time) combinations in index Array.
			const Array<double,1>& numeraire_values   ///< Numeraire values for the dates in timeline Array.
			) = 0;

		/// Allow for multidimensional payoff (i.e. portfolio) with meaningful default (one-dimensional) behaviour.
		virtual Array<double,1> payoffArray(const Array<double,1>& underlying_values, ///< Underlying values for the (asset,time) combinations in index Array.
			const Array<double,1>& numeraire_values   ///< Numeraire values for the dates in timeline Array.
			);
	};

	class PRICINGENGINE_DLL_API MCPayoffList : public MCPayoff 
	{
	public:

		enum {TYPEID = CLASS_MCPAYOFFLIST_TYPE};

		inline MCPayoffList() : MCPayoff(1,1),underlying_tmp(16),numeraire_tmp(16) 
		{ };

		void push_back(std::shared_ptr<MCPayoff> xpayoff,double xcoeff = 1.0);

		/// Calculate discounted payoff.
		virtual double operator()(const Array<double,1>& underlying_values,const Array<double,1>& numeraire_values);

		/// Multidimensional payoff (i.e. portfolio).
		virtual Array<double,1> payoffArray(const Array<double,1>& underlying_values, ///< Underlying values for the (asset,time) combinations in index Array.
			const Array<double,1>& numeraire_values   ///< Numeraire values for the dates in timeline Array.
			);

	private:
		std::list<std::shared_ptr<MCPayoff> > payoffs;
		std::list<double> coefficients;
		std::list<std::shared_ptr<Array<int,1> > > time_mappings;
		std::list<std::shared_ptr<Array<int,1> > > index_mappings;
		Array<double,1> underlying_tmp;
		Array<double,1>  numeraire_tmp;
		void debug_print();
	};

	class PRICINGENGINE_DLL_API MCEuropeanVanilla : public MCPayoff
	{
	public:

		enum { TYPEID = CLASS_MCEUROPEANVANILLA_TYPE };

		MCEuropeanVanilla(double tzero, double tend, int asset_index, double strike, short sign);
		virtual double operator()(const Array<double, 1>& underlying_values, const Array<double, 1>& numeraire_values);
		inline double& strike()
		{
			return K;
		};

	private:
		double K; ///< Strike.

		short m_sign; /// indicate call or put
	};

	class PRICINGENGINE_DLL_API MCEuropeanCall : public MCPayoff
	{
	public:

		enum {TYPEID = CLASS_MCEUROPEANCALL_TYPE};

		MCEuropeanCall(double tzero,double tend,int asset_index,double strike);
		virtual double operator()(const Array<double,1>& underlying_values,const Array<double,1>& numeraire_values);
		inline double& strike() 
		{
			return K;
		};

	private:
		double K; ///< Strike.
	};

	class PRICINGENGINE_DLL_API MCDiscreteArithmeticMeanFixedStrike : public MCPayoff 
	{
	public:

		enum {TYPEID = CLASS_MCDISCRETEARITHMETICMEANFIXEDSTRIKE_TYPE};

		MCDiscreteArithmeticMeanFixedStrike(int asset_index,const Array<double,1>& T,double xK,int number_of_observations_in_existing_average,double existing_average);
		virtual double operator()(const Array<double,1>& underlying_values,const Array<double,1>& numeraire_values);

	private:
		double                                        K; ///< Strike.
		int number_of_observations_in_existing_average_;
		double                        existing_average_;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_MCPAYOFF_H_ */
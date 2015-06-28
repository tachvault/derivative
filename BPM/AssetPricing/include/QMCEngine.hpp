/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_QMCENGINE_H_
#define _DERIVATIVE_QMCENGINE_H_

#include "MCPayoff.hpp"

#if defined PRICINGENGINE_EXPORTS
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	/** Template that maps a random shift to a set of quasi-random (Sobol) numbers to a stochastic process to a Monte Carlo payoff.

	The template parameter class price_process must implement a member function dimension(),
	which returns the number of assets (i.e. the dimension) of the price process.
	It must also implement the member function set_timeline(const Array<double,1>& timeline),
	which sets the timeline for which asset prices will be generated.
	Furthermore, it must provide operator()(Array<double,2>& underlying_values,const random_variable& x,const TermStructure& ts) 
	and operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const random_variable& x,const TermStructure& ts,int numeraire_index),
	which generate the values of the price process at the dates in the required timeline using
	a draw x of the quasi-random variable (the type of which is given by the template parameter random_variable),
	under the martingale measure associated with, respectively, deterministic bond prices or a chosen numeraire.
	These requirements are for example implemented by the class GeometricBrownianMotion.

	@see GeometricBrownianMotion

	The template parameter class QR_generator must implement a member function number_of_points(), 
	which returns the maximum number of quasi-random points which the instance of class QR_generator can provide.
	This determines the number of quasi-random points used to generate the return value of the member functions
	mapping() and mappingArray() of RandomShiftQMCMapping. To generate these points, the class QR_generator must 
	also implement a member function random(random_variable x) (as determined by the template parameter class 
	random_variable). In the call random(random_variable x), x determines the random shift of the quasi-random numbers
	in the sequence. Furthermore, the class QR_generator must implement a member function reset(), which resets the quasi-random
	number generator to the start of its sequence.
	*/
	template <class price_process,class random_variable,class QR_generator>
	class RandomShiftQMCMapping 
	{
	private:
		MCMapping<price_process,random_variable> mc_mapping;
		QR_generator&                                 QRgen;
	public:
		/// Constructor.
		inline RandomShiftQMCMapping(QR_generator& xQRgen,MCPayoff& xpayoff,price_process& xprocess,const TermStructure& xts,int xnumeraire_index = -1)
			: mc_mapping(xpayoff,xprocess,xts,xnumeraire_index),QRgen(xQRgen) 
		{ }; 
		/// Choose the numeraire asset for the equivalent martingale measure under which the simulation is carried out.
		inline bool set_numeraire_index(int xnumeraire_index) 
		{
			return mc_mapping.set_numeraire_index(xnumeraire_index); 
		};
		/// The function mapping a realisation of the shift to the (often multidimensional) quasi-random variable x to the discounted payoff.
		double mapping(random_variable x);
		/// The function mapping a realisation of the shift to the (often multidimensional) quasi-random variable x to multiple discounted payoffs.
		Array<double,1> mappingArray(random_variable x);
	};

	template <class price_process,class random_variable,class QR_generator>
	double RandomShiftQMCMapping<price_process,random_variable,QR_generator>::mapping(random_variable x)
	{
		size_t i;
		double result = 0.0;
		QRgen.reset();
		for (i = 0; i<QRgen.number_of_points(); i++) result += mc_mapping.mapping(QRgen.random(x));
		return result/QRgen.number_of_points();
	}

	template <class price_process,class random_variable,class QR_generator>
	Array<double,1> RandomShiftQMCMapping<price_process,random_variable,QR_generator>::mappingArray(random_variable x)
	{
		size_t i;
		QRgen.reset();
		Array<double, 1> result(mc_mapping.mappingArray(QRgen.random(x)));
		for (i = 1; i<QRgen.number_of_points(); i++) result += mc_mapping.mappingArray(QRgen.random(x));
		result /= QRgen.number_of_points();
		return result;
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_PAYOFF_H_ */

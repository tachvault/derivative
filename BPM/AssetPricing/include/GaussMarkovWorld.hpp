/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GAUSSMARKOVWORLD_H_
#define _DERIVATIVE_GAUSSMARKOVWORLD_H_

#include <stdexcept>
#include <vector>
#include <memory>

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include "BlackScholesAsset.hpp"
#include "GaussianHJM.hpp"
#include "MultivariateNormal.hpp"
#include "GaussianEconomy.hpp"
#include "GaussMarkovTermStructure.hpp"
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
	using blitz::Array;
	using blitz::firstDim;
	using blitz::secondDim;
	using blitz::Range;
	using blitz::toEnd;
	using blitz::firstIndex;
	using blitz::secondIndex;

	/** In order to implement the closed form the multicurrency Gauss/Markov HJM model is 
	represented by a collection of C++ classes. At the top level the entire multicurrency model
	is encapsulated in the class GaussMarkovWorld.

	An instance of GaussMarkovWorld is constructed from its constituent (single–currency)
	economies and from the exchange rates (of each foreign currency in units of
	domestic currency), characterised by their initial values and their volatilities.

	The GaussMarkovWorld constructor takes as arguments,
	1) a reference to a std::vector of shared pointers to instances of GaussianEconomy
	(representing each constituent economy)
	2) a reference to a std::vector of shared pointers to instances of DeterministicAssetVol 
	(representing	the volatility of each exchange rate) 
	3) an Array<double,1> of initial values	of the exchange rates.	
	*/
	class PRICINGENGINE_DLL_API GaussMarkovWorld
	{
	public:

		enum {TYPEID = CLASS_GAUSSMARKOVWORLD_TYPE};

		class reportable 
		{
		public:
			int currency_index;
			int    asset_index;
			double    maturity; 
		};
		std::vector<reportable> reportable_list;

		/// Constructor
		GaussMarkovWorld(std::vector<std::shared_ptr<GaussianEconomy> > xeconomies,
			std::vector<std::shared_ptr<DeterministicAssetVol> > xvols,
			Array<double,1> xinitial_exchange_rates);

		/// Construct from a CSV file specification
		GaussMarkovWorld(const char* path);

		/// Destructor
		~GaussMarkovWorld();

		/// Determine which asset values should be reported back by operator()
		int set_reporting(int currency_index,int asset_index,double maturity = -1.0);

		/// To do: Set drifts, etc. to simulate under a particular choice of numeraire.
		bool set_numeraire(int num);

		/// Query the dimension of the process.
		int dimension() const;

		/// To do: Query the number of random variables (per period) driving the process.
		inline int factors() const 
		{
			return max_rank; 
		};

		/// To do: Set process timeline - need var/covar matrix for state variable increments.
		bool set_timeline(const Array<double,1>& timeline);

		/// Get timeline of asset price realisations reported by operator().
		inline const Array<double,1>& get_timeline() const 
		{ 
			return *T; 
		};

		/// Get number of steps in process time discretisation.
		inline int number_of_steps() const
		{ 
			return T->extent(firstDim)-1; 
		};

		/// Generating a realisation of the process under the martingale measure associated with deterministic bond prices is not applicable in a stochastic term structure model.
		void operator()(Array<double,2>& underlying_values,const Array<double,2>& x,const TermStructure& ts);

		/// Generate a realisation of the process under the martingale measure associated with a given numeraire asset. underlying_values is an asset x (time points) Array.
		void operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const Array<double,2>& x,const TermStructure& ts,int numeraire_index);

		/** Generate a realisation of the process under the martingale measure associated with a given numeraire asset, 
		returning the state variables. underlying_values is an asset x (time points) Array. */
		const Array<double,2>& generate_state_variables(Array<double,1>& numeraire_values,const Array<double,2>& x,int numeraire_index);

		/// Get the current (simulated term structure at the i-th time in the time line, for the j-th currency
		std::shared_ptr<GaussMarkovTermStructure> get_TermStructure(int i,int j);

		inline const Array<double,1>& get_initial_exchange_rates() const 
		{ 
			return initial_exchange_rates; 
		};

		inline std::vector<std::shared_ptr<GaussianEconomy> >  get_economies() const 
		{ 
			return economies; 
		};

		inline std::vector<std::shared_ptr<DeterministicAssetVol> > get_FXvolatilities() const 
		{ 
			return vols; 
		};

		inline double get_forward_exchange_rate(int i,double mat) const 
		{ 
			return initial_exchange_rates(i-1)*(*(economies[i]->initialTS))(mat)/(*(economies[0]->initialTS))(mat); 
		};

		inline double get_terminal_forward_asset(int currency,int asset) const
		{ 
			return terminal_fwd_asset(currency,asset); 
		};

		inline double time_horizon() const 
		{ 
			return (*T)(T->extent(firstDim)-1); 
		};

	private:

		/// "Economies" in different currencies. First entry is taken to be 
		/// the reference (domestic) currency.
		std::vector<std::shared_ptr<GaussianEconomy> >  economies;

		/// Volatilities of exchange rates (in terms of units of "domestic" 
		/// currency per unit of each of the "foreign" currencies.
		std::vector<std::shared_ptr<DeterministicAssetVol> > vols;

		Array<double,1>          initial_exchange_rates;
		Array<double,1>      terminal_fwd_exchange_rate;
		Array<double,2>              terminal_fwd_asset;

		/// Process timeline.
		std::shared_ptr<Array<double,1> > T; 

		/// Simulation path is represented internally as a (number of state variables) 
		/// x (number of process time points) Array
		std::shared_ptr<Array<double,2> >              state_variables;

		/// (number of state variables) x (number of process time steps) Array
		std::shared_ptr<Array<double,2> >         state_variable_drifts;

		int                   number_of_state_variables;

		/// Dimension of the driving Brownian motion
		int                         diffusion_dimension;

		/// maximum rank of covariance matrices of the multivariate normal distributions 
		/// of state variable increments
		int                                    max_rank;

		/// Index of first exchange rate state variable
		int                        currency_start_index;

		Array<double,1>    termstructure_statevariables;

		/// index of first state variable relevant to each economy
		Array<int,1>                economy_start_index;

		/// multivariate normal distributions of state variable increments
		std::vector<std::shared_ptr<MultivariateNormal> > mvn;
		int                           current_numeraire;

		/// index of the numeraire asset in the list of reportable assets (if applicable)
		int                  numeraire_reportable_index;

		/// index of the domestic terminal zero coupon bond in the list of reportable assets (if applicable), 
		/// to convert terminal forward to spot value of the numeraire
		int                    numeraire_reportable_ZCB;

		/// return true if the index i corresponds to an index
		bool is_asset_index(int i) const;

		/// return the position value of an economy on which the asset index j is part of
		inline int economy_index(int j) const;  

		void initialise();

		bool check_inputs();

		const DeterministicAssetVol& get_vol(int index);

		/// for debugging
		double zsum,z2sum; 

		/// for debugging
		int zn; 
		void propagate_state_variables(const Array<double,2>& x);	
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_GAUSSMARKOVWORLD_H_ */


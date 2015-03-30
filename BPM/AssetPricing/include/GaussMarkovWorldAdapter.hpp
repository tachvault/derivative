/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#ifndef _DERIVATIVE_GAUSSMARKOVWORLDADAPTER_H_
#define _DERIVATIVE_GAUSSMARKOVWORLDADAPTER_H_

#include <memory>
#include <blitz/array.h>

#include "Global.hpp"
#include "ClassType.hpp"
#include "Name.hpp"
#include "GaussMarkovWorld.hpp"

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

	class Country;
	class GaussianEconomyAdapter;
	class GaussianEconomy;
	class IExchangeRateValue;
	class TermStructure;
	class GaussMarkovTermStructure;
	class DeterministicAssetVol;

	/// Adapter class for GaussMarkovWorld
	class PRICINGENGINE_DLL_API GaussMarkovWorldAdapter
	{
	public:

		enum {TYPEID = CLASS_GAUSSMARKOVWORLDADAPTER_TYPE};

		/// Constructor only taking GaussianEconomies. Rest of the constructor parameters
		/// for GaussMarkovWorld can be derived from the currencies involved.
		GaussMarkovWorldAdapter(const Country& domestic, std::vector<std::shared_ptr<GaussianEconomyAdapter> > xeconomies);

		void initialise(std::vector<std::shared_ptr<GaussianEconomyAdapter> > xeconomies);

		/// Destructor
		~GaussMarkovWorldAdapter()
		{
			m_exchangeRates.clear();
		}

		/// Determine which asset values should be reported back by operator()
		int set_reporting(int currency_index,int asset_index,double maturity = -1.0)
		{
			return m_GaussMarkovWorld->set_reporting(currency_index,asset_index,maturity);
		}

		/// To do: Set drifts, etc. to simulate under a particular choice of numeraire.
		bool set_numeraire(int num)
		{
			return m_GaussMarkovWorld->set_numeraire(num);
		}

		/// Query the dimension of the process.
		int dimension() const
		{
			return m_GaussMarkovWorld->dimension();
		}

		/// To do: Query the number of random variables (per period) driving the process.
		inline int factors() const 
		{
			return m_GaussMarkovWorld->factors();
		};

		/// To do: Set process timeline - need var/covar matrix for state variable increments.
		bool set_timeline(const Array<double,1>& timeline)
		{
			return m_GaussMarkovWorld->set_timeline(timeline);
		}

		/// Get timeline of asset price realisations reported by operator().
		inline const Array<double,1>& get_timeline() const 
		{ 
			return m_GaussMarkovWorld->get_timeline();
		};

		/// Get number of steps in process time discretisation.
		inline int number_of_steps() const
		{ 
			return m_GaussMarkovWorld->number_of_steps(); 
		};

		/// Generating a realisation of the process under the martingale measure associated with deterministic bond prices is not applicable in a stochastic term structure model.
		void operator()(Array<double,2>& underlying_values,const Array<double,2>& x,const TermStructure& ts)
		{
			m_GaussMarkovWorld->operator()(underlying_values,x,ts);
		}

		/// Generate a realisation of the process under the martingale measure associated with a given numeraire asset. underlying_values is an asset x (time points) Array.
		void operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const Array<double,2>& x,const TermStructure& ts,int numeraire_index)
		{
			m_GaussMarkovWorld->operator()(underlying_values,numeraire_values,x,ts,numeraire_index);
		}

		/** Generate a realisation of the process under the martingale measure associated with a given numeraire asset, 
		returning the state variables. underlying_values is an asset x (time points) Array. */
		const Array<double,2>& generate_state_variables(Array<double,1>& numeraire_values,const Array<double,2>& x,int numeraire_index)
		{
			return m_GaussMarkovWorld->generate_state_variables(numeraire_values,x,numeraire_index);
		}

		/// Get the current (simulated term structure at the i-th time in the time line, for the j-th currency
		std::shared_ptr<GaussMarkovTermStructure> get_TermStructure(int i,int j)
		{
			return m_GaussMarkovWorld->get_TermStructure(i,j);
		}

		inline const Array<double,1>& get_initial_exchange_rates() const 
		{ 
			return m_GaussMarkovWorld->get_initial_exchange_rates(); 
		};

		inline std::vector<std::shared_ptr<GaussianEconomy> >  get_economies() const 
		{ 
			return m_GaussMarkovWorld->get_economies();
		};

		inline std::vector<std::shared_ptr<DeterministicAssetVol> > get_FXvolatilities() const 
		{ 
			return m_GaussMarkovWorld->get_FXvolatilities();
		};

		inline double get_forward_exchange_rate(int i,double mat) const 
		{ 
			return m_GaussMarkovWorld->get_forward_exchange_rate(i,mat);
		};

		inline double get_terminal_forward_asset(int currency,int asset) const
		{ 
			return m_GaussMarkovWorld->get_terminal_forward_asset(currency,asset); 
		};

		inline double time_horizon() const 
		{ 
			return m_GaussMarkovWorld->time_horizon(); 
		};

		std::shared_ptr<GaussMarkovWorld> GetOrigin() const
		{
			return m_GaussMarkovWorld;
		}

	private:

		std::shared_ptr<GaussMarkovWorld> m_GaussMarkovWorld;

		std::vector<std::shared_ptr<IExchangeRateValue> > m_exchangeRates;

		const Country& m_domestic;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(GaussMarkovWorldAdapter);
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_GAUSSMARKOVWORLDADAPTER_H_ */


/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MEXOTICS_H_
#define _DERIVATIVE_MEXOTICS_H_

#include <memory>

#include "MBinaryPayoffs.hpp"
#include "MCPayoff.hpp"
#include "MCMapping.hpp"
#include "BlackScholesAssetAdapter.hpp"
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
	namespace exotics 
	{
		/** Option to exchange one asset for another.

		Payoff: \f[ [K_1S_1(T)-K_2S_2(T)]^+ \f]
		*/
		class PRICINGENGINE_DLL_API Margrabe 
		{

		public:

			enum {TYPEID = CLASS_MARGRABE_TYPE};

			Margrabe(const std::shared_ptr<BlackScholesAssetAdapter>& S1,const std::shared_ptr<BlackScholesAssetAdapter>& S2,\
				double t,double T,double xK1,double xK2,const TermStructure& xts);

			~Margrabe();

			inline double price() 
			{ 
				return K1*M1->price(0)-K2*M2->price(0); 
			};

			inline double price(const Array<double,1>& T,const Array<double,2>& history) 
			{ 
				return K1*M1->price(T,history,0)-K2*M2->price(T,history,0);
			};

			/// For American option pricing by Monte Carlo
			double early_exercise_payoff(const Array<double,1>& T,const Array<double,2>& history) const;

			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff     part1;
			MBinaryPayoff     part2;
			std::unique_ptr<MBinary>  M1;
			std::unique_ptr<MBinary>  M2;
			double K1,K2;
		};

		/** Standard Option.

		Payoff: \f[ [s\cdot(S(T)-K)]^+ \f]
		*/
		class PRICINGENGINE_DLL_API StandardOption 
		{

		public:

			enum {TYPEID = CLASS_STANDARDOPTION_TYPE};

			StandardOption(const std::shared_ptr<BlackScholesAssetAdapter>& S,double t,double T,double xK,\
				const TermStructure& xts,int xsign = 1);

			~StandardOption();

			inline double price() 
			{ 
				return sign*(M1->price(0)-K*M2->price(0)); 
			};

			inline double price(double t,double S) 
			{ 
				return sign*(M1->price(t,S,0)-K*M2->price(t,S,0)); 
			};

			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff     part1;
			MBinaryPayoff     part2;
			std::unique_ptr<MBinary> M1;
			std::unique_ptr<MBinary> M2;
			int                sign;
			double K;
		};

		/** Power Option.

		Payoff: \f[ [S(T)^{\alpha}-K]^+ \f]
		*/
		class PRICINGENGINE_DLL_API PowerOption
		{
		public:

			enum {TYPEID = CLASS_POWEROPTION_TYPE};

			PowerOption(const std::shared_ptr<BlackScholesAssetAdapter>& S,double alpha,double t,double T,double xK,const TermStructure& xts);
			~PowerOption();
			inline double price() 
			{
				return M1->price(0)-K*M2->price(0); 
			};
			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff     part1;
			MBinaryPayoff     part2;
			std::unique_ptr<MBinary> M1;
			std::unique_ptr<MBinary> M2;
			double K;
		};

		/** Option on the discretely sampled geometric mean of an asset price process.

		Payoff: \f[ [\sqrt[n]{\prod_{i=1}^n S(T_i)}-K]^+ \f]
		*/
		class PRICINGENGINE_DLL_API DiscreteGeometricMeanFixedStrike 
		{

		public:

			enum {TYPEID = CLASS_DISCRETEGEOMETRICMEANFIXEDSTRIKE_TYPE};

			DiscreteGeometricMeanFixedStrike(const std::shared_ptr<BlackScholesAssetAdapter>& S,const Array<double,1>& T,double xK, \
				const TermStructure& xts,int number_of_observations_in_existing_average,double existing_average);

			DiscreteGeometricMeanFixedStrike(const GaussMarkovWorld& world,const Array<double,1>& T,double xK, \
				int reportable_asset_index,int number_of_observations_in_existing_average,double existing_average);

			~DiscreteGeometricMeanFixedStrike();

			inline double price() 
			{ 
				return factor*Mgeo->price(0)-K*MBB->price(0);
			};

			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff       geo;
			MBinaryPayoff        BB;        
			std::unique_ptr<MBinary> Mgeo;
			std::unique_ptr<MBinary> MBB;
			double                K;
			double           factor;  ///< Factor for existing average.

		};

		/** Option on the discretely sampled geometric mean of an asset price process.

		Payoff: \f[ [S(T_n)-K\sqrt[n]{\prod_{i=1}^n S(T_i)}]^+ \f]
		*/
		class PRICINGENGINE_DLL_API DiscreteGeometricMeanFloatingStrike 
		{

		public:

			enum {TYPEID = CLASS_DISCRETEGEOMETRICMEANFLOATINGSTRIKE_TYPE};

			DiscreteGeometricMeanFloatingStrike(const std::shared_ptr<BlackScholesAssetAdapter>& S,\
				const Array<double,1>& T,double xK,const TermStructure& xts,int number_of_observations_in_existing_average,\
				double existing_average);
			~DiscreteGeometricMeanFloatingStrike()
			{}
			inline double price()
			{ 
				return MAB->price(0)-K*factor*Mgeo->price(0);
			};
			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff       geo;
			MBinaryPayoff        AB;        
			std::unique_ptr<MBinary> Mgeo;
			std::unique_ptr<MBinary> MAB;
			double                K;
			double           factor;  ///< Factor for existing average.
		};

		class PRICINGENGINE_DLL_API DiscreteBarrierOut
		{

		public:

			enum {TYPEID = CLASS_DISCRETEBARRIEROUT_TYPE};

			DiscreteBarrierOut(const std::shared_ptr<BlackScholesAssetAdapter>& S,  ///< Underlying asset.
				const Array<double,1>& T,    ///< Barrier monitoring time points.
				double xK,                   ///< Strike.
				double barrier,              ///< Barrier.
				const TermStructure& xts,    ///< (Deterministic) term structure of interest rates.
				int xcallput = 1,            ///< Call (1) or put (-1). Call is default. 
				int updown   = -1            ///< Up (1) or down (-1) option. Down is default.
				);
			~DiscreteBarrierOut();
			inline double price(unsigned long n = 100) 
			{ 
				return callput*(MAB->price(n)-K*MBB->price(n)); 
			};
			std::shared_ptr<MCPayoffList> get_payoff() const;
			// Accessors
			inline const Array<double,2>& covariance_matrix() const
			{ 
				return MAB->covariance_matrix(); 
			};
			inline const Array<double,1>& eigenvalues() const 
			{ 
				return MAB->eigenvalues(); 
			};

		private:      
			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff        AB;
			MBinaryPayoff        BB;        
			std::unique_ptr<MBinary> MAB;
			std::unique_ptr<MBinary> MBB;
			double                K;
			int             callput;
		};

		class PRICINGENGINE_DLL_API ProductOption
		{		
		public:

			enum {TYPEID = CLASS_PRODUCTIONOPTION_TYPE};

			ProductOption(const std::shared_ptr<BlackScholesAssetAdapter>& S1,const std::shared_ptr<BlackScholesAssetAdapter>& S2, \
				double t,double T,double xK1,const TermStructure& xts,int xsign = 1);
			~ProductOption();
			inline double price() 
			{ 
				return sign*(M1->price(0)-K1*M2->price(0)); 
			};
			inline double price(const Array<double,1>& T,const Array<double,2>& history)
			{ 
				return sign*(M1->price(T,history,0)-K1*M2->price(T,history,0)); 
			};
			/// For American option pricing by Monte Carlo
			double early_exercise_payoff(const Array<double,1>& T,const Array<double,2>& history) const;
			std::shared_ptr<MCPayoffList> get_payoff() const;

		private:

			const TermStructure& ts;  ///< (Deterministic) term structure of interest rates.
			MBinaryPayoff     part1;
			MBinaryPayoff     part2;
			std::unique_ptr<MBinary> M1;
			std::unique_ptr<MBinary> M2;
			double K1;
			int sign;

		};
	}
}/* namespace derivative */

#endif /* _DERIVATIVE_PAYOFF_H_ */

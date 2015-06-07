/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#ifndef _DERIVATIVE_BLACKSCHOLESASSETADAPTER_H_
#define _DERIVATIVE_BLACKSCHOLESASSETADAPTER_H_

#include <memory>
#include <utility>

#include "Global.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "BlackScholesAsset.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef DERIVATIVEASSET_EXPORTS
#ifdef __GNUC__
#define DERIVATIVEASSET_DLL_API __attribute__ ((dllexport))
#else
#define DERIVATIVEASSET_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define DERIVATIVEASSET_DLL_API __attribute__ ((dllimport))
#else
#define DERIVATIVEASSET_DLL_API __declspec(dllimport)
#endif
#endif
#define DERIVATIVEASSET_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define DERIVATIVEASSET_DLL_API __attribute__ ((visibility ("default")))
#define DERIVATIVEASSET_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define DERIVATIVEASSET_DLL_API
#define DERIVATIVEASSET_DLL_LOCAL
#endif
#endif

namespace derivative 
{
	class IAssetValue;

	/// Adapter class for BlackScholesAsset. Integrated with the real time data model.	
	/// Objects of type BlackScholesAssetAdapter are copyable, assignable and movable.
	class DERIVATIVEASSET_DLL_API BlackScholesAssetAdapter
	{
	public:

	    enum {TYPEID = CLASS_BLACKSCHOLESASSETADAPTER_TYPE};
		
		/// constructor for constructing asset value.
		BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue> &val);

		/// constructor for constructing asset value and deterministic
		/// volatility.
		BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue> &val,
			std::shared_ptr<DeterministicAssetVol> xv  ///< Volatility function.
			);

		/// constructor for constructing asset value, deterministic
		/// volatility and dividend yield
		BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue> &val,
			std::shared_ptr<DeterministicAssetVol> xv, ///< Volatility function.
			std::shared_ptr<TermStructure> div
			);

		/// Copy constructor.
		BlackScholesAssetAdapter(const BlackScholesAssetAdapter& xasset);

		/// define the assignment operator
		BlackScholesAssetAdapter& operator=(const BlackScholesAssetAdapter& xasset);

		BlackScholesAssetAdapter(const char* path);
		
		inline const BlackScholesAsset& GetOrigin() const
		{
			return *m_backScholesAsset;
		}

		inline const std::shared_ptr<IAssetValue>& GetAssetValue() const
		{
			return m_asset;
		}

		/// (Forward) dividend yield between times u and v.
		inline double  GetDivYield(double u,double v) const 
		{
			return m_backScholesAsset->dividend_yield(u,v);
		};  

		/// (Forward) dividend discount factor between times u and v.
		inline double GetDivDiscount(double u,double v) const 
		{
			return m_backScholesAsset->dividend_discount(u,v);
		}

		inline void SetDivYield(double ini)
		{
			m_backScholesAsset->dividend_yield(ini);
		}

		/// Price a European call or put option (call is default).
		inline double option(double mat,double K,double r,int sign = 1,double today = 0.0) const
		{
			return m_backScholesAsset->option(mat, K, r, sign, today);
		}

		/// Delta of a European call or put option (call is default).
		inline double delta(double mat,double K,double r,int sign = 1) const
		{
			return m_backScholesAsset->delta(mat,K,r,sign);
		}

		/// Gamma of a European call or put option (call is default).
		inline double gamma(double mat,double K,double r,int sign = 1) const
		{
			return m_backScholesAsset->gamma(mat,K,r,sign);
		}

		/// Vega of a European call or put option (call is default).
		inline double vega(double mat, double K, double r, int sign = 1) const
		{
			return m_backScholesAsset->vega(mat, K, r, sign);
		}

		/// Theta of a European call or put option (call is default).
		inline double theta(double mat, double K, double r, int sign = 1) const
		{
			return m_backScholesAsset->theta(mat, K, r, sign);
		}

		/// Option to exchange one asset for another.
		inline double Margrabe(std::shared_ptr<BlackScholesAssetAdapter>& S,double mat,double K, int sign = 1) const
		{
			return m_backScholesAsset->Margrabe(S->GetOrigin(),mat,K, sign);
		}
		
		inline double DoleansExp(double t,double T,const Array<double,1>& dW) const
		{
			return m_backScholesAsset->DoleansExp(t,T,dW);
		}

		inline double GetVolatility(double t,double ttm) const 
		{ 
			return m_backScholesAsset->volatility(t, ttm); 
		};

		/// Change the volatility function.
		inline void SetVolatility(std::unique_ptr<DeterministicAssetVol> xv) 
		{ 
			return m_backScholesAsset->set_volatility(std::move(xv));
		}

		/// Covariance of logarithmic asset prices.
		inline double GetCovariance(double t,double ttm, const std::shared_ptr<BlackScholesAssetAdapter>& S) const 
		{
			return m_backScholesAsset->covariance(t, ttm, S->GetOrigin());
		};

		/// Access the volatility function.
		inline const DeterministicAssetVol& GetVolatilityFunction() const 
		{ 
			return m_backScholesAsset->volatility_function(); 
		};

		/// Calculate the implied volatility for a given price.
		double CalculateImpliedVolatility(double price,double mat,double K,double r,int sign = 1) const
		{
			return m_backScholesAsset->implied_volatility(price,mat,K,r,sign);
		}

	private:

		/// BlackScholes asset that behind this adapter class.
		std::unique_ptr<BlackScholesAsset> m_backScholesAsset;

		/// underlying asset value
		std::shared_ptr<IAssetValue> m_asset;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_BLACKSCHOLESASSET_H_ */

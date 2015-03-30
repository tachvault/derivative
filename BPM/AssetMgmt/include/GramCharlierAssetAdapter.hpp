/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#ifndef _DERIVATIVE_GRAMCHARLIERASSETADAPTER_H_
#define _DERIVATIVE_GRAMCHARLIERASSETADAPTER_H_

#include <memory>
#include <boost/math/distributions/normal.hpp>

#include "IObject.hpp"
#include "IMake.hpp"

#include "GramCharlierAsset.hpp"
#include "ClassType.hpp"
#include "Global.hpp"
#include "Name.hpp"

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
	/// Adapter Class for GramCharlierAsset.
	class DERIVATIVEASSET_DLL_API GramCharlierAssetAdapter : public virtual IObject,
		public virtual IMake
	{
	public:

		enum {TYPEID = CLASS_GRAMCHARLIERASSETADAPTER_TYPE};

		static Name ConstructName(const std::string& symbol, int maturity)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + to_string(maturity)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("maturity"), boost::any_cast<int>(maturity));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, int& maturity)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto j = keys.find("maturity");
			maturity = boost::any_cast<int>(j->second);
		}

		/// Constructor with Exemplar for the Creator GramCharlierAssetAdapter object
		GramCharlierAssetAdapter (const Exemplar &ex);
		
		/// Constructor.
		GramCharlierAssetAdapter(GramCharlier& xgc, ///< Gram/Charlier expanded density for the standardised risk-neutral distribution.
			const std::shared_ptr<IAssetValue>& asset, int maturity ///< maturity in days
			);
		
		static std::shared_ptr<GramCharlierAssetAdapter> Create(GramCharlier& xgc,const std::shared_ptr<IAssetValue>& asset, int maturity);

		/// IMake method; not currently supported.
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		/// IMake method; not currently supported.
		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		inline const std::shared_ptr<IAssetValue>& GetAssetValue() const
		{
			return m_asset;
		}
				
		inline double GetInitialValue() const
		{
			return m_gramCharlierAsset->initial_value();
		};  ///< Query the initial ("time zero") value.

		inline void SetInitialValue(double ini)
		{
			return m_gramCharlierAsset->initial_value(ini);
		}; ///< Set the initial ("time zero") value.

		inline double GetMaturity() const
		{
			return m_gramCharlierAsset->maturity();
		};
		/// Price a European call option.

		double call(double K,double domestic_discount,double foreign_discount = 1.0) const
		{
			return m_gramCharlierAsset->call(K, domestic_discount, foreign_discount);
		};

		/// Best fit calibration to a given set of Black/Scholes implied volatilities.
		double calibrate(double domestic_discount,double foreign_discount,int highest_moment);

		/// Best fit calibration to a given set of Black/Scholes implied volatilities.
		double calibrate(std::shared_ptr<const Array<double,1> > xstrikes, std::shared_ptr<const Array<double,1> > xvols, \
			double domestic_discount,double foreign_discount,int highest_moment)
		{
			return m_gramCharlierAsset->calibrate(xstrikes,xvols, domestic_discount,foreign_discount,highest_moment);
		};

		inline double GetStandardDeviation() const
		{
			return m_gramCharlierAsset->standard_deviation();
		};

		inline double GetSkewness() const
		{
			return m_gramCharlierAsset->skewness();
		};

		inline double GetExcessKurtosis() const 
		{
			return m_gramCharlierAsset->excess_kurtosis();
		};

	private:

		/// GramCharlierAsset asset that behind this adapter class.
		std::unique_ptr<GramCharlierAsset> m_gramCharlierAsset;

		/// underlying asset value
		const std::shared_ptr<IAssetValue> m_asset;

		/// maturity duration in days from trade date.
		int m_maturity;

		/// Name(TYPEID, std::hash<std::string>()(symbol)) 
		/// Key[0] => "symbol"
		/// key[1] => maturity in days
		Name m_name;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_GRAMCHARLIERASSET_H_ */

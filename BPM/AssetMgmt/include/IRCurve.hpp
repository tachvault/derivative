/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IRCURVE_H_
#define _DERIVATIVE_IRCURVE_H_

#include <string>
#include <memory>
#include "ClassType.hpp"
#include "IObject.hpp"
#include "Global.hpp"
#include "TSLinear.hpp"
#include "Country.hpp"
#include "DeterministicVol.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
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
	class IIBORValue;
	class IZeroCouponBondValue;
	class IFixedRateBondValue;
	class IIRValue;
	
	/// Class IRCurve represents interest rate curve for the given country on a given date.
	class PRIMARYASSET_EXT_API IRCurve : public IObject
	{

	public:

		enum {TYPEID = CLASS_IRCURVE_TYPE};

		enum DataSourceType 
		{
			BOND = 1,
			LIBOR = 2,
			YIELD = 3,
			SWAP = 4
		};

		static Name ConstructName(const string& cntry, const dd::date& processDate, unsigned int src)
		{
			Name nm(TYPEID, std::hash<std::string>()(cntry + dd::to_simple_string(processDate) + std::to_string(src)));
			nm.AppendKey(string("country"), boost::any_cast<string>(cntry));
			nm.AppendKey(string("processDate"), boost::any_cast<dd::date>(processDate));
			nm.AppendKey(string("source"), src);
			return nm;
		}

		inline static void GetKeys(const Name& nm, string& cntry, dd::date& processDate, unsigned int src)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("country");
			cntry = boost::any_cast<string>(i->second);
			auto j = keys.find("processDate");
			processDate = boost::any_cast<dd::date>(j->second);	
			auto k = keys.find("source");
			src = boost::any_cast<unsigned int>(k->second);
		}

		/// Constructor with Exemplar for the Creator of term structure of interest rates.
		IRCurve (const Exemplar &ex);

		/// construct IRCurve from a given Name
		IRCurve (const Name& nm);

		/// construct IRCurve for a country in a given date.
		IRCurve(const Country& cntry, const dd::date& processDate, IRCurve::DataSourceType src);

		const Name& GetName()
		{
			return m_name;
		}

		const Country& GetCountry() const
		{
			return m_country;
		}
		
		const dd::date& GetProcessDate() const
		{
			return m_processDate;
		}

		std::shared_ptr<TermStructure> GetTermStructure() const
		{
			return m_termStructure;
		}

		std::shared_ptr<DeterministicAssetVol> GetVolatility() const
		{
			return m_volatility;
		}

		void BuildIRCurve();

	private:

		void FindZCBBondValues(std::vector<std::shared_ptr<IZeroCouponBondValue> >& zcbs) ;

		void FindCouponBondValues(std::vector<std::shared_ptr<IFixedRateBondValue> >& cbonds);

		void FindLiborRateValues(std::vector<std::shared_ptr<IIBORValue> >& rates) ;

		void FindIRRateValues(std::vector<std::shared_ptr<IIRValue> >& rates);
		
		void BuildIRCurveFromBond();

		void BuildIRCurveFromIIBOR();

		void BuildIRCurveFromIR();

		/// The country for the this IR curve is applicable.
		Country m_country;

		/// The date on which this IRCurve is built
		dd::date m_processDate;

		IRCurve::DataSourceType m_src;

		/// Name(IRCurve::TYPEID, std::hash<std::string>() \
		///  (country code, process date))
		/// Key[0] => "country code"
		/// Key[1] => "process date"
		Name m_name;

		std::shared_ptr<TSLinear> m_termStructure;

		std::shared_ptr<DeterministicAssetVol> m_volatility;
	};

	/// utility function to build IRCurve information given
	/// country and effective date
	PRIMARYASSET_EXT_API std::shared_ptr<IRCurve> BuildIRCurve(IRCurve::DataSourceType src, const std::string& cntryCode, const dd::date& edate);
}

/* namespace derivative */

#endif /* _DERIVATIVE_IRCURVE_H_ */
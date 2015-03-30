/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#ifndef _DERIVATIVE_GAUSSIANECONOMYADAPTER_H_
#define _DERIVATIVE_GAUSSIANECONOMYADAPTER_H_

#include <stdexcept>
#include <vector>
#include <memory>

#include "Global.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "classType.hpp"
#include "IRCurve.hpp"

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
	class GaussianEconomy;
	class Country;

	/// This is a named adapter class GaussianEconomy.
	class PRICINGENGINE_DLL_API GaussianEconomyAdapter : public IObject
	{
	public:

		enum {TYPEID = CLASS_GAUSSIANECONOMYADAPTER_TYPE};

		static Name ConstructName(const std::string& symbol)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + std::to_string(economy_id++)));
			nm.SetSymbol(symbol);
			return nm;
		}

		GaussianEconomyAdapter::GaussianEconomyAdapter(const Name& nm, const Country& cntry, \
		  const std::vector<Name>& assets, IRCurve::DataSourceType src);

		inline const Country& GetCountry() const
		{
			return m_country;
		}

		inline std::shared_ptr<GaussianEconomy> GetOrigin() const
		{
			return m_economy;
		}
		
	private:

		/// shared pointer to GaussianEconomy.
		std::shared_ptr<GaussianEconomy> m_economy;

		/// The country corresponding to the Gaussian Economy.
		const Country& m_country;

		/// Hash code of economy_id used as Object ID of this class instances.
		static int economy_id;

		Name m_name;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(GaussianEconomyAdapter);
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_GAUSSIANECONOMYADAPTER_H_ */


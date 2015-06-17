/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEVANILLAOPTION_H_
#define _DERIVATIVE_EXCHANGERATEVANILLAOPTION_H_

#include "Global.hpp"
#include "Name.hpp"
#include "IMessage.hpp"
#include "IMessageSink.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "IRCurve.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef FACADES_EXPORTS
#ifdef __GNUC__
#define FACADES_DLL_API __attribute__ ((dllexport))
#else
#define FACADES_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define FACADES_DLL_API __attribute__ ((dllimport))
#else
#define FACADES_DLL_API __declspec(dllimport)
#endif
#endif
#define FACADES_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define FACADES_DLL_API __attribute__ ((visibility ("default")))
#define FACADES_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define FACADES_DLL_API
#define FACADES_DLL_LOCAL
#endif
#endif

namespace derivative
{
	class IExchangeRateValue;
	class TermStructure;
	class VolatilitySurface;
	class BlackScholesAssetAdapter;
	class MCPayoff;
	class DeterministicAssetVol;
	class ExchangeRateVanillaOptMessage;


	/// Create the ExchangeRateVanillaOption class
	class FACADES_DLL_API ExchangeRateVanillaOption : virtual public IObject,
		virtual public IMake,
		virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EXCHANGERATEVANILLAOPTION_TYPE };

		/// Constructor with Exemplar for the Creator ExchangeRateVanillaOption object
		ExchangeRateVanillaOption(const Exemplar &ex);

		/// constructor for Make
		ExchangeRateVanillaOption(const Name& nm);

		/// initalize the object. Vanillaed by constructor and Activate functions
		void Init(std::string& symbol, double strike, dd::date maturity);

		/// Destructor
		virtual ~ExchangeRateVanillaOption()
		{}

		const Name& GetName()
		{
			return m_name;
		}

		virtual std::shared_ptr<IMake> Make(const Name &nm);

		virtual std::shared_ptr<IMake> Make(const Name &nm, const std::deque<boost::any>& agrs);

		virtual void Activate(const std::deque<boost::any>& agrs);

		/// Nothing to do here.
		virtual void Passivate();

		virtual void Dispatch(std::shared_ptr<IMessage>& msg);

		/// Evalue American call option with Binomial
		virtual void ValueAmericanWithBinomial(int N = 1000);

		virtual void ValueEuropeanWithBinomial(int N = 1000);
		
	private:

		void GetRates(const std::shared_ptr<ExchangeRateVanillaOptMessage>& optMsg, IRCurve::DataSourceType src);

		/// name
		Name m_name;

		/// domestic currency symbol
		std::string m_domestic;

		/// foreign currency symbol
		std::string m_foreign;

		/// option type is either CALL=1 or PUT=-1
		int m_optType;

		/// underlying value
		std::shared_ptr<IExchangeRateValue> m_ExchangeRateVal;

		/// BlackScholesAssetAdapter class
		std::shared_ptr<BlackScholesAssetAdapter> m_ExchangeRate;

		std::shared_ptr<DeterministicAssetVol>  m_vol;

		/// term strucure corresponding to the
		/// country of domestic currency.
		std::shared_ptr<TermStructure> m_termLocal;

		/// term strucure corresponding to the
		/// country of foreign currency.
		std::shared_ptr<TermStructure> m_termForeign;

		/// interest rate resulted from term structure interpolation of local currency
		double m_localRate;

		/// interest rate resulted from term structure interpolation of foreign currency
		double m_foreignRate;

		/// strike price
		double m_strike;

		/// maturity date
		dd::date m_maturity;
		
		/// option values resulted from different pricing models.
		double m_binomial;
		
		double m_closedForm;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEVANILLAOPTION_H_ */

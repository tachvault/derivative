/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEOPTION_H_
#define _DERIVATIVE_EXCHANGERATEOPTION_H_

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
	class VanillaOptMessage;

	/// Create the ExchangeRateOption class
	class FACADES_DLL_API ExchangeRateOption : virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EXCHANGERATEOPTION_TYPE };
		
		/// constructor for Make
		ExchangeRateOption();

		virtual void Activate(const std::deque<boost::any>& agrs);

		/// Nothing to do here.
		virtual void Passivate();

		virtual void Dispatch(std::shared_ptr<IMessage>& msg) = 0;
				
	protected:

		virtual void ProcessVol(const std::shared_ptr<VanillaOptMessage>& optMsg);

		void ProcessRate(const std::shared_ptr<VanillaOptMessage>& optMsg);

		void ValidateResponse(const std::shared_ptr<VanillaOptMessage>& optMsg);

		void GetRates(const std::shared_ptr<VanillaOptMessage>& optMsg, IRCurve::DataSourceType src);

		/// domestic currency symbol
		std::string m_domestic;

		/// foreign currency symbol
		std::string m_foreign;

		/// option type is either CALL=1 or PUT=-1
		int m_optType;

		/// underlying value
		std::shared_ptr<IExchangeRateValue> m_exchangeRateVal;

		/// BlackScholesAssetAdapter class
		std::shared_ptr<BlackScholesAssetAdapter> m_exchangeRate;

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
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEOPTION_H_ */

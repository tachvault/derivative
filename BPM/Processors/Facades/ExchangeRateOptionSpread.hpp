/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEOPTIONSPREAD_H_
#define _DERIVATIVE_EXCHANGERATEOPTIONSPREAD_H_

#include "Global.hpp"
#include "ClassType.hpp"
#include "Name.hpp"
#include "IMessage.hpp"
#include "IMessageSink.hpp"
#include "IObject.hpp"	
#include "IMake.hpp"
#include "IRCurve.hpp"
#include "ExchangeRateOptionSpreadMessage.hpp"

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
	class DeterministicAssetVol;
	class ExchangeRateVolatilitySurface;

	/// Create the ExchangeRateOptionSpread class
	class FACADES_DLL_API ExchangeRateOptionSpread : virtual public IObject,
		virtual public IMake,
		virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EXCHANGERATEOPTIONSPREAD_TYPE };

		//// define option price resulted from MC
		struct MCValue
		{
			int path;
			double value;
			double lowerBound;
			double upperBound;

			MCValue(int p, double v, double l, double u)
				:path(p), value(v), lowerBound(l), upperBound(u)
			{};
		};

		typedef MCValue MCValueType;

		/// Constructor with Exemplar for the Creator ExchangeRateOptionSpread object
		ExchangeRateOptionSpread(const Exemplar &ex);

		/// constructor for Make
		ExchangeRateOptionSpread(const Name& nm);

		/// initalize the object. Vanillaed by constructor and Activate functions
		void Init(std::string& symbol, double strike, dd::date maturity);

		/// Destructor
		virtual ~ExchangeRateOptionSpread()
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

		void ProcessSpreadLeg(ExchangeRateOptionSpreadMessage::ResponseLeg& res, const ExchangeRateOptionSpreadMessage::Leg&, \
			ExchangeRateOptionSpreadMessage::PricingMethodEnum, ExchangeRateOptionSpreadMessage::OptionStyleEnum);

	private:
		
		void GetRates(const std::shared_ptr<ExchangeRateOptionSpreadMessage>& optMsg, IRCurve::DataSourceType src);

		/// name
		Name m_name;

		/// domestic currency symbol
		std::string m_domestic;

		/// foreign currency symbol
		std::string m_foreign;

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

		std::shared_ptr<ExchangeRateVolatilitySurface> m_volSurface;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEOPTIONSPREAD_H_ */

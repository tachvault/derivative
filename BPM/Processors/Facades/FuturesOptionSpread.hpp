/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESOPTIONSPREAD_H_
#define _DERIVATIVE_FUTURESOPTIONSPREAD_H_

#include "Global.hpp"
#include "ClassType.hpp"
#include "Name.hpp"
#include "IMessage.hpp"
#include "IMessageSink.hpp"
#include "IObject.hpp"	
#include "IMake.hpp"
#include "FuturesOptionSpreadMessage.hpp"

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
	class IFuturesValue;
	class TermStructure;
	class VolatilitySurface;
	class BlackScholesAssetAdapter;
	class DeterministicAssetVol;
	class FuturesVolatilitySurface;

	/// Create the FuturesOptionSpread class
	class FACADES_DLL_API FuturesOptionSpread : virtual public IObject,
		virtual public IMake,
		virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EQUITYOPTIONSPREAD_TYPE };

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

		/// Constructor with Exemplar for the Creator FuturesOptionSpread object
		FuturesOptionSpread(const Exemplar &ex);

		/// constructor for Make
		FuturesOptionSpread(const Name& nm);

		/// initalize the object. Vanillaed by constructor and Activate functions
		void Init(std::string& symbol, double strike, dd::date maturity);

		/// Destructor
		virtual ~FuturesOptionSpread()
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

		void ProcessSpreadLeg(FuturesOptionSpreadMessage::ResponseLeg& res, const FuturesOptionSpreadMessage::Leg&, \
			FuturesOptionSpreadMessage::PricingMethodEnum, FuturesOptionSpreadMessage::OptionStyleEnum);

	private:

		/// name
		Name m_name;

		/// futures symbol
		std::string m_symbol;
		
		dd::date m_delivery;

		/// underlying value
		std::shared_ptr<IFuturesValue> m_futuresVal;

		/// BlackScholesAssetAdapter class
		std::shared_ptr<BlackScholesAssetAdapter> m_futures;

		std::shared_ptr<DeterministicAssetVol>  m_vol;

		/// term strucure corresponding to the
		/// country of underlying.
		std::shared_ptr<TermStructure> m_term;

		std::shared_ptr<FuturesVolatilitySurface> m_volSurface;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESOPTIONSPREAD_H_ */

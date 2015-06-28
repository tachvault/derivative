/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYOPTION_H_
#define _DERIVATIVE_EQUITYOPTION_H_

#include "Global.hpp"
#include "Name.hpp"
#include "IMessage.hpp"
#include "IMessageSink.hpp"
#include "IObject.hpp"
#include "IMake.hpp"

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
	class IStockValue;
	class TermStructure;
	class VolatilitySurface;
	class BlackScholesAssetAdapter;
	class MCPayoff;
	class DeterministicAssetVol;
	class VanillaOptMessage;

	/// Create the EquityOption class
	class FACADES_DLL_API EquityOption : virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EQUITYOPTION_TYPE };
		
		/// constructor for Make
		EquityOption();

		virtual void Activate(const std::deque<boost::any>& agrs);

		/// Nothing to do here.
		virtual void Passivate();

		virtual void Dispatch(std::shared_ptr<IMessage>& msg) = 0;
				
	protected:

		virtual void ProcessVol(const std::shared_ptr<VanillaOptMessage>& optMsg);

		void ProcessRate(const std::shared_ptr<VanillaOptMessage>& optMsg);

		/// stock symbol
		std::string m_symbol;

		/// option type is either CALL=1 or PUT=-1
		int m_optType;

		/// underlying value
		std::shared_ptr<IStockValue> m_stockVal;

		/// BlackScholesAssetAdapter class
		std::shared_ptr<BlackScholesAssetAdapter> m_stock;

		std::shared_ptr<DeterministicAssetVol>  m_vol;

		/// term strucure corresponding to the
		/// country of underlying.
		std::shared_ptr<TermStructure> m_term;

		/// interest rate resulted from term structure interpolation
		double m_termRate;

		/// strike price
		double m_strike;

		/// maturity date
		dd::date m_maturity;		
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYOPTION_H_ */

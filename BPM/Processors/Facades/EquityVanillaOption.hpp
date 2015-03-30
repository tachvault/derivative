/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYVANILLAOPTION_H_
#define _DERIVATIVE_EQUITYVANILLAOPTION_H_

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

	/// Create the EquityVanillaOption class
	class FACADES_DLL_API EquityVanillaOption : virtual public IObject,
		virtual public IMake,
		virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EQUITYVANILLAOPTION_TYPE };

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

		/// Constructor with Exemplar for the Creator EquityVanillaOption object
		EquityVanillaOption(const Exemplar &ex);

		/// constructor for Make
		EquityVanillaOption(const Name& nm);

		/// initalize the object. Vanillaed by constructor and Activate functions
		void Init(std::string& symbol, double strike, dd::date maturity);

		/// Destructor
		virtual ~EquityVanillaOption()
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
		
		/// evaluate with Monte-Carlo
		void ValueEuropeanWithMC(size_t minpaths = 100000, size_t maxpaths = 100000, size_t N = 10, size_t train = 100, int degree = 2, double ci = 0.95);

		void ValueAmericanWithMC(size_t minpaths = 100000, size_t maxpaths = 100000, size_t N = 10, size_t train = 100, int degree = 2, double ci = 0.95);

	private:

		/// name
		Name m_name;

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

		/// option values resulted from different pricing models.
		double m_binomial;
		
		std::vector<MCValueType> m_mc;
		
		double m_closedForm;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYVANILLAOPTION_H_ */

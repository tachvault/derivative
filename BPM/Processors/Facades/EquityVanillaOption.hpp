/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYVANILLAOPTION_H_
#define _DERIVATIVE_EQUITYVANILLAOPTION_H_

#include "EquityOption.hpp"
#include "IMake.hpp"
#include "IMessageSink.hpp"
#include "Name.hpp"

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
	class FACADES_DLL_API EquityVanillaOption : virtual public EquityOption,
		virtual public IObject,
		virtual public IMake,
		virtual public IMessageSink
	{
	public:

		enum { TYPEID = CLASS_EQUITYVANILLAOPTION_TYPE };

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

		virtual void Dispatch(std::shared_ptr<IMessage>& msg);
				
	private:

		/// name
		Name m_name;	
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYVANILLAOPTION_H_ */

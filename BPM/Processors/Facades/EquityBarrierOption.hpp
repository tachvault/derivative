/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYBARRIEROPTION_H_
#define _DERIVATIVE_EQUITYBARRIEROPTION_H_

#include "Global.hpp"
#include "Name.hpp"
#include "IMessage.hpp"
#include "IMessageSink.hpp"
#include "IObject.hpp"
#include "IMake.hpp"
#include "EquityOption.hpp"

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

	/// Create the EquityBarrierOption class
	class FACADES_DLL_API EquityBarrierOption : virtual public EquityOption,
		virtual public IObject,
		virtual public IMake
	{
	public:

		enum { TYPEID = CLASS_EQUITYBARRIEROPTION_TYPE };

		/// Constructor with Exemplar for the Creator EquityBarrierOption object
		EquityBarrierOption(const Exemplar &ex);

		/// constructor for Make
		EquityBarrierOption(const Name& nm);

		/// Destructor
		virtual ~EquityBarrierOption()
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

		int m_barrierType;

		double m_barrier;		
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYBARRIEROPTION_H_ */
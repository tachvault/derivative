/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_VANILLAOPTMESSAGE_H_
#define _DERIVATIVE_VANILLAOPTMESSAGE_H_

#include <atomic>
#include <cpprest/json.h>
#include "IMessage.hpp"
#include "IVisitor.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef MESSAGES_EXPORTS
#ifdef __GNUC__
#define MESSAGES_DLL_API __attribute__ ((dllexport))
#else
#define MESSAGES_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define MESSAGES_DLL_API __attribute__ ((dllimport))
#else
#define MESSAGES_DLL_API __declspec(dllimport)
#endif
#endif
#define MESSAGES_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MESSAGES_DLL_API __attribute__ ((visibility ("default")))
#define MESSAGES_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define MESSAGES_DLL_API
#define MESSAGES_DLL_LOCAL
#endif
#endif

using namespace web;

/// This interface represents messages, as sent and received
/// by the messaging services component. 
namespace derivative
{
	class MESSAGES_DLL_API VanillaOptMessage : virtual public IMessage
	{
	public:

		enum OptionTypeEnum { CALL = 0, PUT = 1, TYPE_UNKNOWN = 2 };

		enum PricingMethodEnum { CLOSED = 0, LATTICE = 1, MONTE_CARLO = 2 };

		enum OptionStyleEnum { EUROPEAN = 0, AMERICAN = 1 };

		enum RateTypeEnum { YIELD = 0, LIBOR = 1 };

		struct Greeks
		{
			Greeks()
				:delta(0.0), gamma(0.0), vega(0.0), theta(0.0)
			{}

			double delta;

			double gamma;

			double vega;

			double theta;
		};

		struct Request
		{
			Request()
				:style(EUROPEAN),
				method(LATTICE),
				rateType(LIBOR),
				option(TYPE_UNKNOWN),
				strike(0.0),
				vol(0.0)
			{}

			OptionTypeEnum option;

			OptionStyleEnum style;

			PricingMethodEnum method;

			RateTypeEnum rateType;

			std::string underlying;

			dd::date maturity;

			double strike;

			double vol;

			bool validate()
			{
				if (option == OptionTypeEnum::TYPE_UNKNOWN || \
					maturity.is_not_a_date() || \
					(fabs(strike) < std::numeric_limits<double>::epsilon()))
				{
					return false;
				}
				return true;
			}
		};

		struct Response
		{
			Response()
				:optPrice(0.0),
				underlyingTradePrice(0.0)
			{}

			dd::date underlyingTradeDate;

			double underlyingTradePrice;

			double optPrice;

			Greeks greeks;
		};

		VanillaOptMessage(int extMsgId)
			:m_msgSeq(++extMsgId)
		{}

		virtual ~VanillaOptMessage()
		{}

		virtual  msgType GetMsgId() const = 0;

		virtual const MsgSequence& GetMsgSequence() const
		{
			return m_msgSeq;
		}

		virtual const Request& GetRequest() const
		{
			return m_req;
		}

		virtual const Response& GetResponse() const
		{
			return m_res;
		}

		virtual const SystemResponse& GetSystemResponse() const
		{
			return m_sysRes;
		}

		virtual void SetMsgSequence(const MsgSequence& seq)
		{
			m_msgSeq = seq;
		}

		virtual void SetSystemResponse(const SystemResponse& res)
		{
			m_sysRes = res;
		}

		virtual void SetRequest(const Request& req)
		{
			m_req = req;
		}

		virtual void SetResponse(const Response& res)
		{
			m_res = res;
		}

		virtual void accept(const shared_ptr<IVisitor>& visitor, json::value& out) = 0;

		virtual json::value AsJSON() = 0;

	protected:

		MsgSequence m_msgSeq;

		Request m_req;

		Response m_res;

		SystemResponse m_sysRes;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_VANILLAOPTMESSAGE_H_ */


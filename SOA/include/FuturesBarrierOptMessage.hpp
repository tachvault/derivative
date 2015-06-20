/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESBARRIEROPTMESSAGE_H_
#define _DERIVATIVE_FUTURESBARRIEROPTMESSAGE_H_

#include <atomic>
#include <cpprest/json.h>
#include "VanillaOptMessage.hpp"
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
	class MESSAGES_DLL_API FuturesBarrierOptMessage : public std::enable_shared_from_this<FuturesBarrierOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = FUTURES_BARRIER_OPTION };

		enum BarrierOptionTypeEnum { KDI = 0, KDO = 2, KUI = 3, KUO = 4 };

		struct BarrierRequest : public Request
		{
			BarrierRequest()
			{}

			BarrierOptionTypeEnum barrierType;		

			double barrier;
		};

		struct BarrierResponse : public Response
		{
			BarrierResponse()
			{}

			double barierOptPrice;
		};

		virtual ~FuturesBarrierOptMessage()
		{}

		virtual  msgType GetMsgId() const
		{
			return MSGID;
		}

		virtual void accept(const shared_ptr<IVisitor>& visitor, json::value& out)
		{
			visitor->Visit(shared_from_this(), out);
		}

		json::value AsJSON();
		
		virtual const BarrierRequest& GetRequest() const
		{
			return m_req;
		}

		virtual void SetRequest(const BarrierRequest& req)
		{
			m_req = req;
		}

		virtual const BarrierResponse& GetResponse() const
		{
			return m_res;
		}

		virtual void SetResponse(const BarrierResponse& res)
		{
			m_res = res;
		}
		
		virtual FuturesBarrierOptMessage::BarrierOptionTypeEnum  ParseBarrierType(const std::map<string_t, string_t>& query_strings);
		
		inline void ParseDeliveryDate(FuturesBarrierOptMessage::BarrierRequest &req, const std::map<string_t, string_t>& query_strings);

	private:
		
		BarrierRequest m_req;

		BarrierResponse m_res;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESBARRIEROPTMESSAGE_H_ */


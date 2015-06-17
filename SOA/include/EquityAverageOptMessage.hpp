/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYAVERAGEOPTMESSAGE_H_
#define _DERIVATIVE_EQUITYAVERAGEOPTMESSAGE_H_

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
	class MESSAGES_DLL_API EquityAverageOptMessage : public std::enable_shared_from_this<EquityAverageOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = EQUITY_AVERAGE_OPTION };

		enum AverageTypeEnum {FIXED_STRIKE = 0, FLOATING_STRIKE = 1 };

		struct MESSAGES_DLL_API AverageOptRequest : public Request
		{
			AverageOptRequest()
			{}

			AverageTypeEnum averageType;
		};

		struct AverageOptResponse : public Response
		{
			AverageOptResponse()
			{}

			double averageOptPrice;
		};

		virtual ~EquityAverageOptMessage()
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

		virtual const AverageOptRequest& GetRequest() const
		{
			return m_req;
		}

		virtual void SetRequest(const AverageOptRequest& req)
		{
			m_req = req;
		}

		virtual const AverageOptResponse& GetResponse() const
		{
			return m_res;
		}

		virtual void SetResponse(const AverageOptResponse& res)
		{
			m_res = res;
		}

		EquityAverageOptMessage::AverageTypeEnum ParseAverageType(const std::map<string_t, string_t>& query_strings);

	private:

		static std::atomic<int> extMsgId;

		AverageOptRequest m_req;

		AverageOptResponse m_res;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYAVERAGEOPTMESSAGE_H_ */


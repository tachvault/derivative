/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYMARGRABEOPTMESSAGE_H_
#define _DERIVATIVE_EQUITYMARGRABEOPTMESSAGE_H_

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
	class MESSAGES_DLL_API EquityMargrabeOptMessage : public std::enable_shared_from_this<EquityMargrabeOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = EQUITY_MARGRABE_OPTION };
		
		struct MargrabeOptRequest : public Request
		{
			MargrabeOptRequest()
			{}

			std::string numeraire;
		};

		virtual ~EquityMargrabeOptMessage()
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

		virtual const MargrabeOptRequest& GetRequest() const
		{
			return m_req;
		}

		virtual void SetRequest(const MargrabeOptRequest& req)
		{
			m_req = req;
		}
		
		inline virtual void ParseStrike(VanillaOptMessage::Request &req, const std::map<string_t, string_t>& query_strings);

		inline VanillaOptMessage::PricingMethodEnum  EquityMargrabeOptMessage::ParsePricingMethod(const std::map<string_t, string_t>& query_strings);

	private:

		MargrabeOptRequest m_req;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYMARGRABEOPTMESSAGE_H_ */
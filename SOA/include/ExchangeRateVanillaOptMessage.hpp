/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEVANILLAOPTMESSAGE_H_
#define _DERIVATIVE_EXCHANGERATEVANILLAOPTMESSAGE_H_

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
	class MESSAGES_DLL_API ExchangeRateVanillaOptMessage : public std::enable_shared_from_this<ExchangeRateVanillaOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = EXCHANGERATE_VANILLA_OPTION };

		struct MESSAGES_DLL_API ExchangeRateRequest : public Request
		{
			ExchangeRateRequest()
			{}
		};

		virtual ~ExchangeRateVanillaOptMessage()
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
		
		virtual const ExchangeRateRequest& GetRequest() const
		{
			return m_req;
		}

		virtual void SetRequest(const ExchangeRateRequest& req)
		{
			m_req = req;
		}

	private:
	
		ExchangeRateRequest m_req;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEVANILLAOPTMESSAGE_H_ */


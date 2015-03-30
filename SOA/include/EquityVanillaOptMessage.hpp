/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYVANILLAOPTMESSAGE_H_
#define _DERIVATIVE_EQUITYVANILLAOPTMESSAGE_H_

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
	class MESSAGES_DLL_API EquityVanillaOptMessage : public std::enable_shared_from_this<EquityVanillaOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = EQUITY_VANILLA_OPTION };

		EquityVanillaOptMessage()
			:VanillaOptMessage(extMsgId)
		{}

		virtual ~EquityVanillaOptMessage()
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

	private:

		static std::atomic<int> extMsgId;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYVANILLAOPTMESSAGE_H_ */


/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EQUITYCHOOSEROPTMESSAGE_H_
#define _DERIVATIVE_EQUITYCHOOSEROPTMESSAGE_H_

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
	class MESSAGES_DLL_API EquityChooserOptMessage : public std::enable_shared_from_this<EquityChooserOptMessage>,
		virtual public VanillaOptMessage
	{
	public:

		enum { MSGID = EQUITY_CHOOSER_OPTION };

		struct ChooserOptResponse : public Response
		{
			ChooserOptResponse()
			{}

			double chooserOptPrice;
		};

		virtual ~EquityChooserOptMessage()
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

		virtual const ChooserOptResponse& GetResponse() const
		{
			return m_res;
		}

		virtual void SetResponse(const ChooserOptResponse& res)
		{
			m_res = res;
		}

	private:

		ChooserOptResponse m_res;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_EQUITYCHOOSEROPTMESSAGE_H_ */


/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IMESSAGE_H_
#define _DERIVATIVE_IMESSAGE_H_

#include "global.hpp"
#include "MsgType.hpp"
#include "IObject.hpp"

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

/// This interface represents messages, as sent and received
/// by the messaging services component. 

namespace derivative
{
	class MESSAGES_DLL_API IMessage
	{
	public:

		enum { MSGID = ALL_MSG };

		enum OutcomeEnum {Success = 0, SystemError = 1, RequestError = 2, RegistrationError = 3};

		struct MsgSequence
		{
			MsgSequence(long extId)
				:m_extReqID(extId), m_intReqID(0)
			{}

			/// the request ID inserted into the message
			/// by the external system (By load balancer)
			long m_extReqID;

			/// the request ID inserted into the message
			/// by the MessageDispatcher of each app server
			long m_intReqID;
		};

		struct SystemResponse
		{
			OutcomeEnum outcome;

			std::string outText;

			SystemResponse()
				:outcome(Success), outText("Success")
			{}
		};

		virtual msgType  GetMsgId() const = 0;

		virtual const MsgSequence& GetMsgSequence() const = 0;

		virtual void SetMsgSequence(const MsgSequence& seq) = 0;

		virtual const SystemResponse& GetSystemResponse() const = 0;

		virtual void SetSystemResponse(const SystemResponse& res) = 0;
		
		/// virtual inline dtor just in case
		virtual ~IMessage()
		{};
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IMESSAGE_H_ */

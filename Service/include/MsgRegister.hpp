/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MSGPREGISTER_H_
#define _DERIVATIVE_MSGPREGISTER_H_

#include <memory>
#include <boost/preprocessor/cat.hpp>

#include "Global.hpp"
#include "IObject.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef MESSAGEDISPATCHER_EXPORTS
#ifdef __GNUC__
#define MESSAGEDISPATCHER_DLL_API __attribute__ ((dllexport))
#else
#define MESSAGEDISPATCHER_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define MESSAGEDISPATCHER_DLL_API __attribute__ ((dllimport))
#else
#define MESSAGEDISPATCHER_DLL_API __declspec(dllimport)
#endif
#endif
#define MESSAGEDISPATCHER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MESSAGEDISPATCHER_DLL_API __attribute__ ((visibility ("default")))
#define MESSAGEDISPATCHER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define MESSAGEDISPATCHER_DLL_API
#define MESSAGEDISPATCHER_DLL_LOCAL
#endif
#endif

namespace derivative
{

#define MSG_REGISTER(msg, cls) \
	namespace  \
	{  \
	MsgRegister BOOST_PP_CAT(__ar,__LINE__) (msg::MSGID,  cls::TYPEID);  \
}
	/// This class is used to setup the msg processor with msg type
	class MESSAGEDISPATCHER_DLL_API MsgRegister
	{
	public:

		enum { TYPEID = CLASS_MSGREGISTER };

		MsgRegister(grpType msgId, grpType processorId);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(MsgRegister);

	};

} /* namespace derivative */

#endif /*_DERIVATIVE_MSGPREGISTER_H_ */
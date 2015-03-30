/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#pragma once
#ifndef _DERIVATIVE_MSGPROCESSORMANAGER_H_
#define _DERIVATIVE_MSGPROCESSORMANAGER_H_
#pragma warning (push)
#pragma warning (disable : 4251)

#include <memory>
#include <mutex>
#include <limits>   
#include <unordered_map>
#include <deque>

#include "Global.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "EntityGroup.hpp"

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
	/// MsgProcessorManager is a singleton object responsible for
	/// maintaining named object identity and entity object persistence.
	class  MESSAGEDISPATCHER_DLL_API MsgProcessorManager
	{

	public:

		/// Hash map type for <Msg object ID (EquityCallOption), Msg Processor Group ID (Ex: AmericanCallFacade)>
		typedef std::unordered_multimap<msgType, grpType> HashMsgType;

		/// destructor
		~MsgProcessorManager();

		/// Get the MsgProcessorManager singleton instance
		static MsgProcessorManager& getInstance();

		/// Register msg with the processor Id.
		void registerProcessor(msgType msgId, grpType processorId);

		/// return the first processor name for the msg
		unsigned int findProcessor(msgType msgId);

	private:

		/// constructor
		MsgProcessorManager();

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(MsgProcessorManager);

		/// MsgProcessorManager singleton instance
		static unique_ptr<MsgProcessorManager> m_instance;

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		/// Provides mutually exclusive access to 
		/// MsgProcessorManager class members.
		std::mutex m_mutex;

		HashMsgType m_msgToProcessor;
	};

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_MSGPROCESSORMANAGER_H_ */

/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#pragma once
#ifndef _DERIVATIVE_ESBMANAGER_H_
#define _DERIVATIVE_ESBMANAGER_H_
#pragma warning (push)
#pragma warning (disable : 4251)

#include <memory>
#include <mutex>
#include <deque>

#include "Global.hpp"
#include "Name.hpp"
#include "IObject.hpp"
#include "IMessage.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef ESB_EXPORTS
#ifdef __GNUC__
#define ESB_DLL_API __attribute__ ((dllexport))
#else
#define ESB_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define ESB_DLL_API __attribute__ ((dllimport))
#else
#define ESB_DLL_API __declspec(dllimport)
#endif
#endif
#define ESB_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define ESB_DLL_API __attribute__ ((visibility ("default")))
#define ESB_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ESB_DLL_API
#define ESB_DLL_LOCAL
#endif
#endif

namespace derivative
{
	class ServiceRouter;
	class ServiceLogger;
	class SecurityManager;

	/// ESBManager is a singleton object responsible form receiving from
	/// inceptors and push them into destination message dispatchers.
	/// The destinations could be in the same machine or different machine
	/// as single process running with this or different process.
	/// ESBManager does all these by delegation.
	class  ESB_DLL_API ESBManager
	{

	public:

		/// destructor
		~ESBManager();

		/// Get the ESBManager singleton instance
		static ESBManager& getInstance();

		/// handle requests from interceptors. The handler should
		/// - Log request
		/// - Call ServiceRouter to route the request
		void HandleRequest(std::shared_ptr<IMessage> msg);

		/// authorize the request. The interceptors can use this function
		/// to authorize request
		bool Authorize(const std::string& token);

		void LogRequest(const std::string& token, const std::string& datetime, const std::string& url);

	private:

		/// constructor
		ESBManager();

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(ESBManager);

		/// ESBManager singleton instance
		static unique_ptr<ESBManager> m_instance;

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		/// Provides mutually exclusive access to 
		/// ESBManager class members.
		std::mutex m_mutex;

		/// service router instance
		std::unique_ptr<ServiceRouter> m_router;

		/// Authorizer instance
		std::unique_ptr<SecurityManager> m_security;

		/// Service logger
		std::unique_ptr<ServiceLogger> m_logger;
	};

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_ESBMANAGER_H_ */

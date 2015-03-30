/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#pragma once
#ifndef _DERIVATIVE_SYSTEMMANAGER_H_
#define _DERIVATIVE_SYSTEMMANAGER_H_
#pragma warning (push)
#pragma warning (disable : 4251)

#include <memory>
#include <mutex>

#include "Global.hpp"
#include "Config.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef ENTITYMGMT_EXPORTS
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllexport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllimport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ENTITY_MGMT_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define ENTITY_MGMT_DLL_API __attribute__ ((visibility ("default")))
    #define ENTITY_MGMT_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define ENTITY_MGMT_DLL_API
    #define ENTITY_MGMT_DLL_LOCAL
  #endif
#endif

namespace derivative
{
	/// SystemManager is a singleton object responsible managing
	/// system wide tasks and variables
	class  ENTITY_MGMT_DLL_API SystemManager
	{

	public:	

		/// destructor
		~SystemManager();

		/// Get the SystemManager singleton instance
		static SystemManager& getInstance();

		/// get and set run mode
		runModeEnum GetRunMode() const
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			return m_mode;
		}

		void SetRunMode(runModeEnum mode)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			m_mode = mode;
		}
				
	private:

		/// constructor
		SystemManager();

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(SystemManager);

		/// SystemManager singleton instance
		static unique_ptr<SystemManager> m_instance;

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		/// Provides mutually exclusive access to 
		/// SystemManager class members.
		mutable std::mutex m_mutex;		

		runModeEnum m_mode;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_SYSTEMMANAGER_H_ */

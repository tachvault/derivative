/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_BPMLOADER_H_
#define _DERIVATIVE_BPMLOADER_H_

#include <memory>
#include <mutex>

#include "Global.hpp"
#include "Config.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef BPMLOADER_EXPORTS
#ifdef __GNUC__
#define BPMLOADER_DLL_API __attribute__ ((dllexport))
#else
#define BPMLOADER_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define BPMLOADER_DLL_API __attribute__ ((dllimport))
#else
#define BPMLOADER_DLL_API __declspec(dllimport)
#endif
#endif
#define BPMLOADER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define BPMLOADER_DLL_API __attribute__ ((visibility ("default")))
#define BPMLOADER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define BPMLOADER_DLL_API
#define BPMLOADER_DLL_LOCAL
#endif
#endif

namespace derivative
{
	/// BPMLoader is a singleton object responsible managing
	/// system wide tasks and variables
	class  BPMLOADER_DLL_API BPMLoader
	{

	public:	

		/// destructor
		~BPMLoader();

		/// Get the BPMLoader singleton instance
		static BPMLoader& getInstance();

		/// get and set run mode
		void LoadLIBORRates();

		void LoadYield()
		{}
				
	private:

		/// constructor
		BPMLoader();

		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(BPMLoader);

		/// BPMLoader singleton instance
		static unique_ptr<BPMLoader> m_instance;

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		/// Provides mutually exclusive access to 
		/// BPMLoader class members.
		mutable std::mutex m_mutex;

		static std::vector<std::string> LIBORCurrencies;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_BPMLOADER_H_ */

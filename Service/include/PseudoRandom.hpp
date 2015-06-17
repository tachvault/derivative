/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#pragma once
#ifndef _DERIVATIVE_PSEUDORANDOM_H_
#define _DERIVATIVE_PSEUDORANDOM_H_
#pragma warning (push)
#pragma warning (disable : 4251)

#include <memory>
#include <mutex>

#include "Global.hpp"
#include "SpinLock.hpp"

namespace derivative
{
	/// PseudoRandom is a singleton object responsible for
	/// providing thread safe access to Blitz++ Random number
	/// generators
	template <class random_number_generator_type, class rntype>
	class  PseudoRandom
	{

	public:	

		/// constructor
		PseudoRandom()
		{
			/// instantiate random number generator
			RNG = std::make_shared<random_number_generator_type>();
			LOG(INFO) << "PseudoRandom constructor is called " << endl;
		}

		/// destructor
		~PseudoRandom()
		{
			LOG(INFO) << "PseudoRandom destructor is called " << endl;
		}

		/// Get the PseudoRandom singleton instance
		static PseudoRandom& getInstance()
		{
			static std::unique_ptr<PseudoRandom<random_number_generator_type, rntype> > _instance;
			if (!m_initialized)
			{
				_instance.reset(new PseudoRandom);
				m_initialized = true;
				LOG(INFO) << "PseudoRandom is initialized" << endl;
			}
			return *_instance;
		}

		inline rntype random()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return RNG->random();
		}

	private:
		
		/// use copy and assignment
		DISALLOW_COPY_AND_ASSIGN(PseudoRandom);

		/// this flag to make sure constructor executed
		/// only once.
		static bool m_initialized;

		std::shared_ptr<random_number_generator_type> RNG;

		/// Provides mutually exclusive access to 
		/// PseudoRandom class members.
		mutable SpinLock m_lock;		
	};

	template <class random_number_generator_type, class rntype>
	bool PseudoRandom<random_number_generator_type, rntype>::m_initialized = false;

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_IPSEUDORANDOM_H_ */

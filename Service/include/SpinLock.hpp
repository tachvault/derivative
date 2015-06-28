#ifndef _DERIVATIVE_SPINLOCK_H_
#define _DERIVATIVE_SPINLOCK_H_

#include <atomic>

namespace derivative
{
	class SpinLock
	{
	public:
		SpinLock()
#ifdef __GNUC__
			:flag(ATOMIC_FLAG_INIT)
		{}
#else
		{
			flag.clear();
		}
#endif
		void lock()
		{
			while (flag.test_and_set(std::memory_order_acquire));
		}
		void unlock()
		{
			flag.clear(std::memory_order_release);
		}

	private:

		SpinLock(const SpinLock&) = delete;
		SpinLock& operator=(const SpinLock&) = delete;
		SpinLock& operator=(const SpinLock&) volatile = delete;

		std::atomic_flag flag;
	};

} /* namespace derivative */

#endif  /// _DERIVATIVE_SPINLOCK_H_

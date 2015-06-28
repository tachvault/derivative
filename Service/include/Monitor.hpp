#ifndef _DERIVATIVE_MONITOR_H_
#define _DERIVATIVE_MONITOR_H_

#include <memory>
#include <mutex>

namespace derivative
{
	template<class Lock, class T>
	class monitor
	{
	private:
		mutable T t;
		mutable Lock m_lock;

	public:
		monitor(T t_)
			:t(t_)
		{ }

		template<typename F>
		auto operator()(F f) const-> decltype(f(t))
		{
			std::lock_guard<Lock> hold(m_lock);
			return f(t);
		}
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_MONITOR_H_ */

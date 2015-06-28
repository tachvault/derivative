#ifndef _DERIVATIVE_OBJECTPOOL_H_
#define _DERIVATIVE_OBJECTPOOL_H_

#include <atomic>
#include "Global.hpp"
#include "ThreadSafeQueue.hpp"

namespace derivative
{
	/// Thread safe ObjectPool manages a pool objects of the same Named type
	/// All objects in a pool should be of the same type
	template <typename T>
	class ObjectPool
	{
	public:

		ObjectPool(grpType id)
			:m_grpId(id),
			m_objCount(1)
		{}

		inline void wait_and_pop(std::shared_ptr<T>& value)
		{
			return m_queue.wait_and_pop(value);
		}

		inline bool try_pop(std::shared_ptr<T>& value)
		{
			return m_queue.try_pop(value);
		}

		inline std::shared_ptr<T> wait_and_pop()
		{
			return m_queue.wait_and_pop();
		}

		inline std::shared_ptr<T> try_pop()
		{
			return m_queue.try_pop();
		}

		inline bool empty() const
		{
			return m_queue.empty();
		}

		inline void push(std::shared_ptr<T>& new_value)
		{
			m_queue.push(new_value);
		}

		inline int increment()
		{
			return ++m_objCount;
		}

		inline int GetObjCount()
		{
			return m_objCount.load();
		}

	private:
		
		/// group id of a objects in a pool
		const grpType m_grpId;

		/// number of connections
		std::atomic<int> m_objCount;

		/// thread safe queue to pool passive resources.
		threadsafe_queue<T> m_queue;

		/// delete complier generated copy, move and assignment functions.
		ObjectPool(const ObjectPool&) = delete;
		ObjectPool& operator=(const ObjectPool&) = delete;
		ObjectPool(ObjectPool&& o) = delete;
	};

} /* namespace derivative */

#endif

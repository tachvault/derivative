#ifndef _DERIVATIVE_THREADSAFERESPQUEUE_H_
#define _DERIVATIVE_THREADSAFERESPQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

#include <IMessage.hpp>

namespace derivative
{
	class threadsafe_resp_queue
	{
	private:
		mutable std::mutex mut;
		std::queue<std::shared_ptr<IMessage> > data_queue;
		std::condition_variable data_cond;
	public:
		threadsafe_resp_queue()
		{}

		void wait_and_pop(std::shared_ptr<IMessage>& value, int reqId)
		{
			std::unique_lock<std::mutex> lk(mut);
			auto now = std::chrono::system_clock::now();
			if (data_cond.wait_until(lk, now + std::chrono::milliseconds(1800000), [this, reqId]{
				return (!data_queue.empty() && data_queue.front()->GetMsgSequence().m_intReqID == reqId); }))
			{
				value = data_queue.front();
				data_queue.pop();
			}
			else
			{
				throw std::runtime_error("System takes longer time to process");
			}
		}

		bool try_pop(std::shared_ptr<IMessage>& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			if(data_queue.empty())
				return false;
			value=data_queue.front();
			data_queue.pop();
		}

		std::shared_ptr<IMessage> wait_and_pop(int reqId)
		{
			std::unique_lock<std::mutex> lk(mut);
			auto now = std::chrono::system_clock::now();
			if (data_cond.wait_until(lk, now + std::chrono::milliseconds(1800000), [this, reqId]{
				return (!data_queue.empty() && data_queue.front()->GetMsgSequence().m_intReqID == reqId); }))
			{
				std::shared_ptr<IMessage> res = data_queue.front();
				data_queue.pop();
				return res;
			}
			else
			{
				throw std::runtime_error("System takes longer time to process");
			}
		}

		std::shared_ptr<IMessage> try_pop()
		{
			std::lock_guard<std::mutex> lk(mut);
			if(data_queue.empty())
				return std::shared_ptr<IMessage>();
			std::shared_ptr<IMessage> res=data_queue.front();
			data_queue.pop();
			return res;
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}

		void push(std::shared_ptr<IMessage>& new_value)
		{
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(new_value);
			data_cond.notify_all();
		}
	};

} /* namespace derivative */

#endif

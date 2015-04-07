/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "MessageDispatcher.hpp"
#include "IMessageSink.hpp"
#include "MsgProcessorManager.hpp"
#include "DException.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	const int MessageDispatcher::defaultCntMax = 1000;

	MessageDispatcher::MessageDispatcher(const Exemplar &ex)
		:m_name(TYPEID)
	{}

	MessageDispatcher::MessageDispatcher(const Name& nm)
		: m_name(nm)
	{}

	MessageDispatcher::~MessageDispatcher()
	{
	}

	/// IMake functions
	std::shared_ptr<IMake> MessageDispatcher::Make(const Name &nm)
	{
		return std::make_shared<MessageDispatcher>(nm);
	}

	std::shared_ptr<IMake> MessageDispatcher::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Not supported");

	}

	void MessageDispatcher::Spin(int wait)
	{
		/// try pop a message
		std::shared_ptr<IMessage>& msg = m_reqQueue.wait_and_pop();

		/// Start processing the message asynchronously in a new thread
		auto future = std::async(std::launch::async, &MessageDispatcher::Dispatch, this, msg);
	}
	
	void MessageDispatcher::HandleMessage(std::shared_ptr<IMessage>& msg)
	{
		/// push the message into the reqQueue for processing
		m_reqQueue.push(msg);

		/// now wait for the response message
		std::shared_ptr<IMessage> respMsg = m_respQueue.wait_and_pop(msg->GetMsgSequence().m_intReqID);
	}

	void MessageDispatcher::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		try
		{
			/// Now find the MsgSink corresponding to the message
			std::shared_ptr<IMessageSink> processor = getMsgSink(msg->GetMsgId());

			/// Call the processor to execute business action
			processor->Dispatch(msg);

			/// push the MessageSink to pool after passivate then
			/// release the processor back to the pool
			processor->Passivate();
			auto obj = dynamic_pointer_cast<IObject>(processor);
			std::unique_lock<SpinLock> lock(m_lock);
			m_pool.at(obj->GetName().GetGrpId())->push(processor);
			lock.unlock();
		}
		catch (std::exception& e)
		{
			/// update the message with the error condition
			IMessage::SystemResponse sysRes;
			sysRes.outcome = IMessage::SystemError;
			sysRes.outText = e.what();
			msg->SetSystemResponse(sysRes);
		}
		
		/// now push the message to the response queue
		m_respQueue.push(msg);
	}

	std::shared_ptr<IMessageSink> MessageDispatcher::getMsgSink(msgType msgId)
	{
		/// Get the MsgProcessorManager instance
		MsgProcessorManager& msgMgr = MsgProcessorManager::getInstance();

		/// Get the MsgSink group ID for the given message type
		grpType grpId = msgMgr.findProcessor(msgId);

		/// if no connection pool created for the Message Processor then insert a ConnectionPool
		/// instance that is associated with the given group ID
		std::unique_lock<SpinLock> lock(m_lock);
		if (m_pool.find(grpId) == m_pool.end())
		{
			lock.unlock();
			std::shared_ptr<ObjectPool<IMessageSink> > pool = std::make_shared<ObjectPool<IMessageSink> >(grpId);
			lock.lock();
			m_pool.insert(std::pair<grpType, std::shared_ptr<ObjectPool<IMessageSink> > >(grpId, pool));
		}
		
		/// now check if there is a passive processor in the resource pool
		std::shared_ptr<IMessageSink> sink;
		if (m_pool.at(grpId)->try_pop(sink))
		{
			return sink;
		}
		lock.unlock();
		/// We need to construct MsgSink from MsgSink exemplar; get the examplar object for the MsgSink
		/// Exemplar objects should be initialized during global initialization time.
		try
		{
			/// Get the EntityManager instance
			EntityManager& entMgr = EntityManager::getInstance();

			/// we have MsgSinkGrpId, we can retrive exemplar for MsgSink
			/// if exemplar not registered with EntityManager, EntityManager
			/// would throw an exception and caller needs to handle the exception
			std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(grpId));

			/// Now we have the exampler object. 
			/// Make the exemplar MsgSink to construct MsgSink for the given entity
			/// Remember each Entity object associated with a MsgSink
			/// increment the connection count; if more than max then asset
			lock.lock();
			int objId = m_pool.at(grpId)->increment();
			lock.unlock();
			assert(objId <= defaultCntMax);
			std::shared_ptr<IMake> obj = (dynamic_pointer_cast<IMake>(exemplar))->Make(Name(grpId, objId));
			std::shared_ptr<IMessageSink> msgProcessor = dynamic_pointer_cast<IMessageSink>(obj);

			/// now return the created message processor
			return msgProcessor;
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " EntityGroup for the given Message Processor type not found for " << grpId << endl;
			LOG(WARNING) << e.what() << endl;
			throw e;
		}
	}

	std::shared_ptr<MessageDispatcher> BuildMessageDispatcher(const Name& nm)
	{
		static bool firstTime = true;
		EntityManager& entMgr = EntityManager::getInstance();
		if (firstTime)
		{
			/// register the exemplar here until the bug in visual C++ is fixed.
			GroupRegister MessageDispatcherGrp(MessageDispatcher::TYPEID, std::make_shared<MessageDispatcher>(Exemplar()));
			firstTime = false;
		}

		/// try to get from registry
		std::shared_ptr<MessageDispatcher>  msgDisp;
		try
		{
			msgDisp = dynamic_pointer_cast<MessageDispatcher>(entMgr.findObject(nm));
		}
		catch (RegistryException& e)
		{
			cout << "MessageDispatcher with Name " << nm << " not in registry" << endl;
		}

		if (msgDisp == nullptr)
		{
			/// construct HistoricalExchangeRateInfo object
			msgDisp = std::make_shared<MessageDispatcher>(nm);
			try
			{
				entMgr.registerObject(nm, msgDisp);
			}
			catch (RegistryException& e)
			{
				LOG(ERROR) << "Throw exception.. The object in registry and unable to register again" << endl;
				throw e;
			}
		}
		return msgDisp;

	}
}
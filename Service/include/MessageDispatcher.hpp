/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MESSAGEDISPATCHER_H_
#define _DERIVATIVE_MESSAGEDISPATCHER_H_

#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio.hpp>

#include "global.hpp"
#include "IObject.hpp"
#include "EntityManager.hpp"
#include "ClassType.hpp"
#include "IMessage.hpp"
#include "ThreadSafeQueue.hpp"
#include "ThreadSafeRespQueue.hpp"
#include "IMake.hpp"
#include "IMessageSink.hpp"

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

using boost::asio::ip::tcp;
namespace derivative
{
	class MESSAGEDISPATCHER_DLL_API MessageDispatcher : virtual public IObject,
		virtual public IMake
	{
		/// connection pool per Message processor type 
		class ConnectionPool
		{
		public:

			ConnectionPool(grpType id)
				:m_grpId(id)
			{
				m_connections = defaultCntInit;
			}

			/// group id of a Message proessor type
			const grpType m_grpId;

			/// number of connections
		    int m_connections;

			/// thread safe queue to pool passive resources.
			threadsafe_queue<IMessageSink> m_queue;
		};

	public:

		enum { TYPEID = CLASS_MESSAGEDISPATCHER_TYPE };

		/// Constructor with Exemplar
		MessageDispatcher(const Exemplar &ex);
		
		/// constructor for the message dispatcher, called by the Create 
		/// member function. If the socket is empty then no TCP message
		/// server will be created and initialized.
		MessageDispatcher(const Name& nm);

		~MessageDispatcher();

		const Name& GetName()
		{
			return m_name;
		}

		/// IMake functions
		std::shared_ptr<IMake> Make(const Name &nm);

		std::shared_ptr<IMake> Make(const Name &nm, const std::deque<boost::any>& agrs);

		/// Used by local message router or TCP/IP router to
		/// push the message into the queue
		void HandleMessage(std::shared_ptr<IMessage>& msg);

		/// Dispatch the message to message processors
		void Dispatch(std::shared_ptr<IMessage>& msg);

		void Spin(int wait);

		static std::shared_ptr<MessageDispatcher> Create(const tcp::socket& cnx);

	private:

		std::shared_ptr<IMessageSink> getMsgSink(msgType msgId);

		Name m_name;

		/// declare the connection pool. For each msgId there will be a 
		/// message pool of processors setup. The maximum number
		/// of processors setup 
		std::map<msgType, std::shared_ptr<ConnectionPool> > m_pool;
		
		/// A thread safe queue to allocate to queue incoming req messages
		threadsafe_queue<IMessage> m_reqQueue;

		/// A thread safe queue to allocate to queue outgoing resp messages
		threadsafe_resp_queue m_respQueue;

		const static int defaultCntInit;

		const static int defaultCntMax;
	};

	/// utility function to build MessageDispatcher given a name
	MESSAGEDISPATCHER_DLL_API std::shared_ptr<MessageDispatcher> BuildMessageDispatcher(const Name& nm);
}

/* namespace derivative */

#endif /* _DERIVATIVE_IMESSAGEDISPATCHER_H_ */


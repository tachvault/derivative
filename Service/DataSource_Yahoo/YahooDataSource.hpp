/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_YAHOODATASOURCE_H_
#define _DERIVATIVE_YAHOODATASOURCE_H_

#include <memory>
#include <mutex>

#include "ThreadSafeQueue.hpp"
#include "IDataSource.hpp"
#include "Name.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DATASOURCE_YAHOO_EXPORTS
    #ifdef __GNUC__
      #define DATASOURCE_DLL_API2 __attribute__ ((dllexport))
    #else
      #define DATASOURCE_DLL_API2 __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DATASOURCE_DLL_API2 __attribute__ ((dllimport))
    #else
      #define DATASOURCE_DLL_API2 __declspec(dllimport)
    #endif
  #endif
  #define DATASOURCE_DLL_LOCAL2
#else
  #if __GNUC__ >= 4
    #define DATASOURCE_DLL_API2 __attribute__ ((visibility ("default")))
    #define DATASOURCE_DLL_LOCAL2  __attribute__ ((visibility ("hidden")))
  #else
    #define DATASOURCE_DLL_API2
    #define DATASOURCE_DLL_LOCAL2
  #endif
#endif

namespace derivative
{
	class IDAO;

	/// IDataSource interface for persistency 
	/// of Stock entity object.
	class DATASOURCE_DLL_API2 YahooDataSource : public IDataSource
	{
	public:

		enum {TYPEID = CLASS_YAHOODATASOURCE_TYPE};

		virtual const Name& GetName()
		{
			return m_name;
		}

		YahooDataSource (const Exemplar &ex);

		YahooDataSource (const Name& nm);

		/// virtual destructor
		virtual ~YahooDataSource ();

		virtual const unsigned short getSource()  const
		{
			return m_source;
		}

		/// IMake functions
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// IDataSource function
		virtual std::shared_ptr<IObject> GetEntity(const Name &nm); 

		/// IDataSource function
		void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities);

		/// IDataSource function.
		virtual void flush(const std::shared_ptr<IObject>& obj);

		/// IDataSource function.
		virtual void insert(const std::shared_ptr<IObject>& obj);

		/// refresh from Yahoo now. Essentially delete the refresh
		/// operation to DAO
		bool refreshObject(std::shared_ptr<IObject>& obj);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(YahooDataSource);

		/// Provides mutually exclusive access to 
		/// YahooDataSource class members.
		std::mutex m_mutex;

		/// get the DAO type id for given Name (Ex: Name(IStockValue::TYPEID, 2010303))
		/// and delegate to getDAO(grpType) for actually getting DAO
		std::shared_ptr<IDAO> getDAO(const Name& nm);

		/// get the DAO type id for given object
		/// and delegate to getDAO(grpType) for actually getting DAO
		std::shared_ptr<IDAO> getDAO(const std::shared_ptr<IObject>& obj);
	
		/// implements the retrieval of DAO
		std::shared_ptr<IDAO> getDAO(grpType daoTypeId);

		/// A thread safe queue to allocate free 
		/// pool free DAO objects and allocate DAO
		/// objects when available and block new request
		/// until one available (pushed back)
		threadsafe_queue<IDAO> m_DAOQueue;

		/// current count of DAO objects in the pool
		unsigned int m_DAOCount;

		/// Max number of DAO objects. When m_DAOCount
		/// reached maxPooledDAO new request would be blocked
		/// if the queue is empty
		static unsigned short maxPooledDAO; 

		Name m_name;

		/// Indicate the external sounce.
		/// Initialized with YAHOO.
		const unsigned short m_source;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_YAHOODATASOURCE_H_ */

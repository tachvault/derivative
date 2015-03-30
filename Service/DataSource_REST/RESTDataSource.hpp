/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_RESTDATASOURCE_H_
#define _DERIVATIVE_RESTDATASOURCE_H_

#include <memory>
#include <mutex>
#include <set>
#include <atomic>

#include "ThreadSafeQueue.hpp"
#include "IDataSource.hpp"
#include "Name.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DATASOURCE_REST_EXPORTS
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
	class DATASOURCE_DLL_API2 RESTDataSource : public IDataSource
	{
	public:

		enum {TYPEID = CLASS_RESTDATASOURCE_TYPE};

		virtual const Name& GetName()
		{
			return m_name;
		}

		RESTDataSource (const Exemplar &ex);

		RESTDataSource (const Name& nm);

		/// virtual destructor
		virtual ~RESTDataSource ();

		virtual bool InSource(unsigned int source)  const
		{
			return (m_source.find(source) != m_source.end()) ? true : false;
		}

		/// IMake functions
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// IDataSource function
		virtual std::shared_ptr<IObject> GetEntity(const Name &nm, unsigned int source);

		/// IDataSource function
		void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities, unsigned int source);

		/// IDataSource function.
		virtual void flush(const std::shared_ptr<IObject>& obj);

		/// IDataSource function.
		virtual void insert(const std::shared_ptr<IObject>& obj);

		/// refresh from REST now. Essentially delete the refresh
		/// operation to DAO
		bool refreshObject(std::shared_ptr<IObject>& obj, unsigned int source);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(RESTDataSource);

		/// Provides mutually exclusive access to 
		/// RESTDataSource class members.
		std::mutex m_mutex;

		/// get the DAO type id for given Name (Ex: Name(IStockValue::TYPEID, 2010303))
		/// and delegate to getDAO(grpType) for actually getting DAO
		std::shared_ptr<IDAO> getDAO(const Name& nm, unsigned int source);

		/// get the DAO type id for given object
		/// and delegate to getDAO(grpType) for actually getting DAO
		std::shared_ptr<IDAO> getDAO(const std::shared_ptr<IObject>& obj, unsigned int source);
	
		/// implements the retrieval of DAO
		std::shared_ptr<IDAO> getDAO(grpType daoTypeId);

		Name m_name;

		/// Indicate the external sounce.
		/// Initialized with REST.
		std::set<unsigned short> m_source;
 
		std::atomic<int>  m_DAOCount;

		const static int defaultCntInit;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_RESTDATASOURCE_H_ */

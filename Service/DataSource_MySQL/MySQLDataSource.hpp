/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MYSQLDATASOURCE_H_
#define _DERIVATIVE_MYSQLDATASOURCE_H_

#include <memory>
#include "IDataSource.hpp"
#include "ObjectPool.hpp"
#include "Name.hpp"
#include "SpinLock.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DATASOURCE_MYSQL_EXPORTS
    #ifdef __GNUC__
      #define DATASOURCE_DLL_API __attribute__ ((dllexport))
    #else
      #define DATASOURCE_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DATASOURCE_DLL_API __attribute__ ((dllimport))
    #else
      #define DATASOURCE_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define DATASOURCE_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DATASOURCE_DLL_API __attribute__ ((visibility ("default")))
    #define DATASOURCE_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DATASOURCE_DLL_API
    #define DATASOURCE_DLL_LOCAL
  #endif
#endif
 
namespace derivative
{
	class IDAO;

	/// IDataSource interface for persistency 
	/// of Stock entity object.
	class DATASOURCE_DLL_API MySQLDataSource : virtual public IDataSource
	{
	public:

		enum {TYPEID = CLASS_MYSQLDATASOURCE_TYPE};

		virtual const Name& GetName()
		{
			return m_name;
		}

		MySQLDataSource (const Exemplar &ex);

		MySQLDataSource (const Name& nm);

		/// virtual destructor
		virtual ~MySQLDataSource();

		virtual bool InSource(unsigned int source)  const
		{
		   return (m_source == source) ? true : false;
		}

		/// IMake functions
		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// IDataSource function
		virtual std::shared_ptr<IObject> GetEntity(const Name &nm, unsigned int source = MYSQL);

		/// IDataSource function
        virtual void find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities, unsigned int source = MYSQL);

		/// IDataSource function.
		virtual void flush(const std::shared_ptr<IObject>& obj);

		/// IDataSource function.
		virtual void insert(const std::shared_ptr<IObject>& obj);

		bool refreshObject(std::shared_ptr<IObject>& obj, unsigned int source = MYSQL)
		{
			throw std::logic_error("Not applicable for MySQL now");
		}

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(MySQLDataSource);

		std::shared_ptr<IDAO> getDAO(const Name& nm);

		Name m_name;

		const unsigned short m_source;

		/// declare the Data access object pool. For each entity type (Ex: IStock) 
		/// there will be a object pool of DAOs. 
		std::map<grpType, std::shared_ptr<ObjectPool<IDAO> > > m_pool;

		mutable SpinLock m_lock;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_MYSQLDATASOURCE_H_ */

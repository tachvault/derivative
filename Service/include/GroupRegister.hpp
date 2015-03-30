/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_GROUPREGISTER_H_
#define _DERIVATIVE_GROUPREGISTER_H_

#include <memory>
#include <boost/preprocessor/cat.hpp>

#include "Global.hpp"
#include "IObject.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef ENTITYMGMT_EXPORTS
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllexport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define ENTITY_MGMT_DLL_API __attribute__ ((dllimport))
    #else
      #define ENTITY_MGMT_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ENTITY_MGMT_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define ENTITY_MGMT_DLL_API __attribute__ ((visibility ("default")))
    #define ENTITY_MGMT_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define ENTITY_MGMT_DLL_API
    #define ENTITY_MGMT_DLL_LOCAL
  #endif
#endif

namespace derivative
{
#define GROUP_REGISTER(cls) \
	namespace  \
	{  \
	GroupRegister BOOST_PP_CAT(__gr,__LINE__) (cls::TYPEID,  std::make_shared<cls>(Exemplar()));  \
}

#define ALIAS_REGISTER(cls, alias) \
	namespace  \
	{  \
	AliasRegister BOOST_PP_CAT(__ar,__LINE__) (cls::TYPEID,  alias::TYPEID);  \
}

#define DAO_REGISTER(cls, source, dao) \
	namespace  \
	{  \
	DAORegister BOOST_PP_CAT(__ar,__LINE__) (cls::TYPEID, source, dao::TYPEID);  \
}

	/// This class is used to register a group ID and its
	/// (optional) exemplar when a component is loaded. This
	/// class is intended to be declared in the global scope of
	/// a component. This causes the constructor to be run at
	/// global initialization time when the component is loaded.
	class ENTITY_MGMT_DLL_API GroupRegister
	{

	public:

		enum {TYPEID = CLASS_GROUPREGISTER};

		GroupRegister (unsigned short grpId, std::shared_ptr<IObject> const& exemplar);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(GroupRegister);
	};


	/// This class is used to setup the delegation of an interface
	/// group to one of its realising concrete groups at component
	/// loadtime. The concrete and interface typeIDs must both be
	/// registered when the constructor is executed. Also the types
	/// must both fall within their respective typeID ranges.
	class ENTITY_MGMT_DLL_API AliasRegister
	{

	public:

		enum {TYPEID = CLASS_ALIASREGISTER};

		AliasRegister(unsigned short concreteType, unsigned short interfaceType);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(AliasRegister);

	};

	/// This class is used to setup the DAO object for 
	/// an Entity's primary interface. There cannot be
	/// the two interfaces pointing to the same DAO.
	/// Ex: Setup StockDAO for IStock interface
	/// Based on the above restriction, one cannot
	/// register StockDAO for IAsset and IStock 
	/// interfaces.
	class ENTITY_MGMT_DLL_API DAORegister
	{

	public:

		enum {TYPEID = CLASS_DAOREGISTER};

		DAORegister(unsigned short entityId, unsigned short source, unsigned short DAOId);

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(DAORegister);

	};

} /* namespace derivative */

#endif /*_DERIVATIVE_GROUPREGISTER_H_ */

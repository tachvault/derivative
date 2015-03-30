/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_NAME_H_
#define _DERIVATIVE_NAME_H_
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)

#include <ostream> 
#include <unordered_map>
#include <string>

#include "boost/any.hpp"
#include "Global.hpp"

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

using std::string;

namespace derivative
{
	/// Class Name represents an object uniquely within an 
	/// instance of Derivative application. The group Id is
	/// unique across class type. The object id is assigned with
	/// hash code of the primary key or a unique composite 
	/// key that uniquely identify the object of a class type
	class ENTITY_MGMT_DLL_API Name
	{

	public:

		typedef std::unordered_map<std::string, boost::any> KeyMapType;

		/// Construction of a EXAMPLAR Name.
		explicit Name (grpType grpId);

		/// Construct a Name from a group Id and an object
		/// id (hash code).  A symbolic name may also be specified.
		explicit Name (grpType grpId, size_t objId, const string& sym = string());

		/// Copy constructor
		Name (const Name& n);

		/// return true if the Name is for exemplar object
		inline bool isExemplar() const
		{
			return m_exemplar;
		}

		/// Return the group ID of the name.
		inline grpType GetGrpId () const
		{
			return m_grpId;
		};

		/// Return the object ID of the name.
		inline size_t GetObjId () const
		{
			return m_ObjId;
		};

		/// Get the symbolic name.
		inline const string& GetSymbol () const
		{
			return m_symbol;
		};

		inline const KeyMapType& GetKeyMap() const
		{
			return m_keys;
		}

		template <typename T>
		T GetValue(const std::string& key) const;

		/// Set the symbolic name.
		void SetSymbol (const string& sym);

		void SetKeys(const KeyMapType& keys);

		/// Append key value
		void AppendKey(const std::string& key, boost::any value);

		bool operator ==(const Name &  right) const;

		bool operator !=(const Name &  right) const;

		inline bool operator!() const;

	private:

		/// factor out common constructor code into one method
		void Init( grpType grpId, size_t objId, const string& sym);

	private:

		/// group ID
		grpType m_grpId;

		/// same type as std::hash
		size_t m_ObjId;   

		/// Indiciate if this Name is for
		/// exemplar object type
		bool m_exemplar;

		/// Symbolic name
		std::string m_symbol;

		/// <Key, Value> pairs
		/// Used by Entity objects to store the key values
		/// which can be used by DAO to retrieve data from
		/// external sources. Ex: Symbol and Trade date from Web Services
		KeyMapType m_keys;
	};

	inline void Name::SetSymbol (const string& sym)
	{
		m_symbol = sym;
	}

	inline bool Name::operator == (const Name &  right) const
	{
		return (m_grpId == right.m_grpId && m_ObjId == right.m_ObjId);
	}

	inline bool Name::operator !=(const Name &  right) const
	{
		return (m_grpId != right.m_grpId || m_ObjId != right.m_ObjId);
	}

	inline bool Name::operator!() const
	{
		return m_ObjId == 0;
	}	
	
	inline std::ostream& operator << (std::ostream &stream, const Name& right)
	{
		stream << "Name( Grp ID = " << right.GetGrpId() << ": Obj ID = " << right.GetObjId() << ")";

		return stream;
	}

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_NAME_H_ */
/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/
#include <exception>
#include <cassert>

#include "Name.hpp"

namespace derivative
{
	Name::Name (grpType grpId)
		:m_grpId(grpId), m_exemplar(true), m_ObjId(0)
	{
		/// GrpId cannot be negative
		assert(grpId >= 0);
	}

	Name::Name (grpType grpId, size_t objId, const string& sym)
		:m_grpId(grpId), m_exemplar(false), m_ObjId(objId), m_symbol(sym)
	{
		/// GrpId cannot be negative
		assert(grpId >= 0);
	}

	Name::Name (const Name& n)
		:m_grpId(n.m_grpId), m_exemplar(n.m_exemplar), m_ObjId(n.m_ObjId), m_symbol(n.m_symbol)
	{
		m_keys = n.m_keys;
	}

	void Name::AppendKey(const std::string& key, boost::any value)
	{
		m_keys.insert(std::make_pair(key, value));
	}

	void Name::SetKeys(const KeyMapType& keys)
	{
		m_keys = keys;
	}

	template <typename T>
	T Name::GetValue(const std::string& key) const
	{
		auto iter = m_keys.find(key);
		if (iter != m_keys.end())
		{
			return boost::any_cast<T>(iter->second);
		}
		throw std::logic_error("Key not found");
	}

} /* namespace derivative */

/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include <functional> 
#include <algorithm>

#include "IDataSource.hpp"
#include "SymbolMap.hpp"

#include "SymbolMapMySQLDAO.hpp"

namespace derivative
{
	class SymbolMap::Impl
	{  
	public:
		
		/// Load Exchange data from database during system startup
		void LoadData(ushort dataSource);

		/// get alias symbol given our ticker symbol
		const std::string& GetSymbolAlias(ushort src, const std::string& symbol);

	private:	
		
		SymbolMapType  m_symbols;   
	};

	SymbolMap::SymbolMap()	
		:m_Impl(std::unique_ptr<SymbolMap::Impl>(new SymbolMap::Impl())),
		m_initialized(false)
	{
		LOG(INFO) << " Constructor is called " << endl;
	}

	SymbolMap::~SymbolMap()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	SymbolMap& SymbolMap::getInstance()
	{
		/// SymbolMap singleton instance
		static std::unique_ptr<SymbolMap> instance;

		if (!instance)
		{
			instance.reset(new SymbolMap());

			LOG(INFO) << "SymbolMap is initialized" << endl;
		}

		return *instance;      
	}

	void SymbolMap::Init(ushort source)
	{
		if (!m_initialized)
		{
		   m_Impl->LoadData(source);
		   m_initialized = true;
		}
	}

	const std::string& SymbolMap::GetSymbolAlias(ushort src, const std::string& symbol)
	{
		return m_Impl->GetSymbolAlias(src, symbol);
	}

	void SymbolMap::Impl::LoadData(ushort source)
	{
		if (source == MYSQL)
		{
			SymbolMapMySQLDAO dao;
			dao.GetSymbolMap(m_symbols);
		}	
	}

	const std::string& SymbolMap::Impl::GetSymbolAlias(ushort src, const std::string& symbol)
	{
		std::pair <ushort, std::string> srcSymbol;
        srcSymbol = std::make_pair (src, symbol);
		auto i = m_symbols.find(srcSymbol);
		if (i != m_symbols.end())
		{
		   return i->second;
		}

		throw std::invalid_argument("Invalid datasrc or symbol code passed");
	}

} /* namespace derivative */
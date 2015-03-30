/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <exception>
#include <functional> 
#include <algorithm>

#include "IDataSource.hpp"
#include "ExchangeExt.hpp"

#include "ExchangeExtMySQLDAO.hpp"

namespace derivative
{
	class ExchangeExt::Impl
	{  
	public:
		
		/// Load Exchange data from database during system startup
		void LoadData(ushort dataSource);

		/// get extension of a given exchange
		const std::string& GetExchangeExt(ushort src, const std::string& exchange);

	private:	
		
		ExchangeExtType  m_extensions;   
	};

	ExchangeExt::ExchangeExt()	
		:m_Impl(std::unique_ptr<ExchangeExt::Impl>(new ExchangeExt::Impl())),
		m_initialized(false)
	{
		LOG(INFO) << " Constructor is called " << endl;
	}

	ExchangeExt::~ExchangeExt()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	ExchangeExt& ExchangeExt::getInstance()
	{
		/// ExchangeExt singleton instance
		static std::unique_ptr<ExchangeExt> instance;

		if (!instance)
		{
			instance.reset(new ExchangeExt());

			LOG(INFO) << "ExchangeExt is initialized" << endl;
		}

		return *instance;      
	}

	void ExchangeExt::Init(ushort source)
	{
		if (!m_initialized)
		{
		   m_Impl->LoadData(source);
		   m_initialized = true;
		}
	}

	const std::string& ExchangeExt::GetExchangeExt(ushort src, const std::string& exchange)
	{
		return m_Impl->GetExchangeExt(src, exchange);
	}

	void ExchangeExt::Impl::LoadData(ushort source)
	{
		if (source == MYSQL)
		{
			ExchangeExtMySQLDAO dao;
			dao.GetExchangeExt(m_extensions);
		}	
	}

	const std::string& ExchangeExt::Impl::GetExchangeExt(ushort src, const std::string& exchange)
	{
		std::pair <ushort, std::string> exchangeExt;
		exchangeExt = std::make_pair(src, exchange);
		auto i = m_extensions.find(exchangeExt);
		if (i != m_extensions.end())
		{
		   return i->second;
		}

		throw std::invalid_argument("Invalid datasrc or symbol code passed");
	}

} /* namespace derivative */
/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <deque>

#include "IDataSource.hpp"
#include "ExchangeHolder.hpp"

#include "ExchangeMySQLDAO.hpp"

namespace derivative
{
	/// This is a private implementation ("Pimpl Idiom")
	/// does not need to be exported.
	class ExchangeHolder::Impl
	{  
	public:

		/// Load Exchange data from database during system startup
		void LoadData(ushort dataSource);

		/// get Exchange given iso code
		Exchange& GetExchange(const std::string& code);

	private:

		std::deque<Exchange> m_data;
	};
	
	ExchangeHolder::ExchangeHolder()	
		:m_Impl(std::unique_ptr<ExchangeHolder::Impl>(new ExchangeHolder::Impl())),
		m_initialized(false)
	{
		LOG(INFO) << " Constructor is called " << endl;
	}

	ExchangeHolder::~ExchangeHolder()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}
	
	ExchangeHolder& ExchangeHolder::getInstance()
	{
		static std::unique_ptr<ExchangeHolder> instance;

		if (!instance)
		{
			instance.reset(new ExchangeHolder());

			LOG(INFO) << "ExchangeHolder is initialized" << endl;
		}

		return *instance;      
	}

	void ExchangeHolder::Init(ushort source)
	{
		if (!m_initialized)
		{
		   m_Impl->LoadData(source);
		   m_initialized = true;
		}
	}

	void ExchangeHolder::Impl::LoadData(ushort source)
	{
		if (source == MYSQL)
		{
			ExchangeMySQLDAO dao;
			dao.GetEntities(m_data);
		}
	}

	Exchange& ExchangeHolder::Impl::GetExchange(const std::string& name)
	{
		auto  result = find_if(m_data.begin(), m_data.end(),[name](Exchange& ex) { return ex.GetExchangeName() == name;});

        /// Print the result.
        if (result != m_data.end()) 
		{
			return *result;
		}

		throw std::invalid_argument("Invalid Exchange code passed");
	}

	const Exchange& ExchangeHolder::GetExchange(const std::string& name)
	{
		return m_Impl->GetExchange(name);
	}
}
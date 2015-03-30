/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <exception>

#include "IDataSource.hpp"
#include "CurrencyHolder.hpp"
#include "Exchange.hpp"

#include "CurrencyMySQLDAO.hpp"

namespace derivative
{
	/// This is a private implementation ("Pimpl Idiom")
	/// does not need to be exported.
	class CurrencyHolder::Impl
	{  
	public:

		/// Load Currency data from database during system startup
		void LoadData(ushort dataSource);

		/// get currency given iso code
		Currency& GetCurrency(const std::string& code);


	private:
			
		std::deque<Currency> m_data;
	};

	CurrencyHolder::CurrencyHolder()	
		:m_Impl(std::unique_ptr<CurrencyHolder::Impl>(new CurrencyHolder::Impl())),
		m_initialized(false)
	{
		LOG(INFO) << " Constructor is called " << endl;
	}

	CurrencyHolder::~CurrencyHolder()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}

	CurrencyHolder& CurrencyHolder::getInstance()
	{
		static std::unique_ptr<CurrencyHolder> instance;
		if (!instance)
		{
			instance.reset(new CurrencyHolder());

			LOG(INFO) << "CurrencyHolder is initialized" << endl;
		}

		return *instance;      
	}

	void CurrencyHolder::Init(ushort source)
	{
		if (!m_initialized)
		{
		   m_Impl->LoadData(source);
		   m_initialized = true;
		}
	}

	void CurrencyHolder::Impl::LoadData(ushort source)
	{
		if (source == MYSQL)
		{
			CurrencyMySQLDAO dao;
			dao.GetEntities(m_data);
		}
	}

	Currency& CurrencyHolder::Impl::GetCurrency(const std::string& code)
	{
		auto  result = find_if(m_data.begin(), m_data.end(),[code](Currency& obj) { return obj.GetCode() == code;});

        /// Print the result.
        if (result != m_data.end()) 
		{
			return *result;
		}

		throw std::invalid_argument("Invalid currency code passed");
	}

	const Currency& CurrencyHolder::GetCurrency(const std::string& code)
	{
		return m_Impl->GetCurrency(code);
	}
}
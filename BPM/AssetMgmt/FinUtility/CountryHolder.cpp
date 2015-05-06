/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <deque>

#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "CurrencyHolder.hpp"

#include "CountryMySQLDAO.hpp"

namespace derivative
{
	/// This is a private implementation ("Pimpl Idiom")
	/// does not need to be exported.
	class CountryHolder::Impl
	{  
	public:

		/// Load Country data from database during system startup
		void LoadData(ushort dataSource);

		/// get Country given iso code
		Country& GetCountry(const std::string& code);

		void GetCountry(const std::string& name, std::vector<Country>& countries);

	private:

		std::deque<Country> m_data;
	};
	
	CountryHolder::CountryHolder()	
		:m_Impl(std::unique_ptr<CountryHolder::Impl>(new CountryHolder::Impl())),
		m_initialized(false)
	{
		LOG(INFO) << " Constructor is called " << endl;
	}

	CountryHolder::~CountryHolder()
	{
		LOG(INFO) << " Destructor is called " << endl;
	}
	
	CountryHolder& CountryHolder::getInstance()
	{
		static std::unique_ptr<CountryHolder> instance;

		if (!instance)
		{
			instance.reset(new CountryHolder());

			LOG(INFO) << "CountryHolder is initialized" << endl;
		}

		return *instance;      
	}

	void CountryHolder::Init(ushort source)
	{
		if (!m_initialized)
		{
		   m_Impl->LoadData(source);
		   m_initialized = true;
		}
	}

	void CountryHolder::Impl::LoadData(ushort source)
	{
		if (source == MYSQL)
		{
			CountryMySQLDAO dao;
			dao.GetEntities(m_data);
		}
	}

	Country& CountryHolder::Impl::GetCountry(const std::string& name)
	{
		auto  result = find_if(m_data.begin(), m_data.end(),[name](Country& ex) { return ex.GetCode() == name;});

        /// Print the result.
        if (result != m_data.end()) 
		{
			return *result;
		}

		throw std::invalid_argument("Invalid Country code passed");
	}

	void CountryHolder::Impl::GetCountry(const std::string& currCode, std::vector<Country>& countries)
	{
		for (auto& cntry : m_data)
		{
			if (cntry.GetCurrency().GetCode().compare(currCode) == 0)
			{
				countries.push_back(cntry);
			}
		}
	}

	const Country& CountryHolder::GetCountry(const std::string& name)
	{
		return m_Impl->GetCountry(name);
	}

	void CountryHolder::GetCountry(const std::string& currCode, std::vector<Country>& countries)
	{
		m_Impl->GetCountry(currCode, countries);
	}
}
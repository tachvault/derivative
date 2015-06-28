/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <utility>
#include <exception>
#include <functional> 
#include <algorithm>
#include "BPMLoader.hpp"
#include "DException.hpp"
#include "IRCurve.hpp"

namespace derivative
{
	bool BPMLoader::m_initialized = false;
	std::vector<std::string> BPMLoader::Countries = { "USA" , "CAN"};

	/// Supports singleton. But NOT lazy initialization.
	/// When this DLL is getting loaded, the getInstance should
	/// be called as part of attach method.
	BPMLoader& BPMLoader::getInstance()
	{
		static std::unique_ptr<BPMLoader> _instance;
		if (!m_initialized)
		{
            _instance.reset(new BPMLoader);
			m_initialized = true;
			LOG(INFO) << "BPMLoader is initialized" << endl;
		}
        return *_instance;      
	}

	BPMLoader::BPMLoader()
	{
		LOG(INFO) << "BPMLoader constructor is called " << endl;
	}

	BPMLoader::~BPMLoader()
	{
		LOG(INFO) << "BPMLoader destructor is called " << endl;
	}

	/// get and set run mode
	void BPMLoader::LoadLIBORRates()
	{
		/// Get domestic interest rate for each country
		dd::date today = dd::day_clock::local_day();
		for (auto &cntry : Countries)
		{
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, cntry, today);
			auto term = irCurve->GetTermStructure();
		}
	}

	void BPMLoader::LoadRates()
	{
		/// Get domestic interest rate for each country
		dd::date today = dd::day_clock::local_day();
		for (auto &cntry : Countries)
		{
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, cntry, today);
			auto term = irCurve->GetTermStructure();
		}
	}

} /* namespace derivative */
/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/



#include <string>
#include <deque>

#include "HistoricalStockInfo.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "EntityManager.hpp"

namespace derivative
{
	namespace
	{
		/// Due to Visual C++ compiler where mutex lock  hangs if called from .dll static variable inicialization, while the dll is loaded 
		/// during loading of dependency libraries on startup of an .exe file.
		///	GroupRegister StockValGrp(HistoricalStockInfo::TYPEID,  std::make_shared<HistoricalStockInfo>(Exemplar())); 
	}

	HistoricalStockInfo::HistoricalStockInfo (const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	HistoricalStockInfo::HistoricalStockInfo (const Name& nm)
		:m_name(nm)
	{
	}	

	/// construct HistoricalStockInfo from 
	/// Stock symbol, Exchange, start date and end date
	HistoricalStockInfo::HistoricalStockInfo (const std::string& symbol, \
		const dd::date& startDate, const dd::date& endDate)
		:m_symbol(symbol), m_startDate(startDate), m_endDate(endDate),
		m_name(ConstructName(symbol, startDate, endDate))
	{
	}

	void HistoricalStockInfo::BuildStockInfo(unsigned int source)
	{
		/// Get the EntityMgrUtil fund the object from this object's
		/// name and the given data source
		try
		{
			auto thisObject = EntityMgrUtil::findObject(GetName(), source);
		}
		catch(RegistryException& e)
		{
			LOG(ERROR) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch(...)
		{
			LOG(ERROR) << " Unknown Exception thrown " << endl;
			throw;
		}
	}

	std::shared_ptr<HistoricalStockInfo> BuildHistoricalStockInfo(ushort source, const const std::string& symbol, const dd::date& start, const dd::date& end)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister StockValGrp(HistoricalStockInfo::TYPEID,  std::make_shared<HistoricalStockInfo>(Exemplar())); 

		/// construct HistoricalStockInfo object
		std::shared_ptr<HistoricalStockInfo>  histStockInfo = std::make_shared<HistoricalStockInfo>(symbol, start, end);
		std::shared_ptr<IObject> obj;
		bool found = false;

		Name nm = histStockInfo->GetName();
		/// check if this object is already registered with EntityManager
		EntityManager& entMgr= EntityManager::getInstance();
		/// check if the object is already registered with EntityManager
		try
		{
			obj = entMgr.findObject(nm);			
		}
		catch(RegistryException& e)
		{
			LOG(INFO) << e.what();
		}
		/// if the object is not found then register
		if (!obj)
		{
			try
			{
				entMgr.registerObject(nm, histStockInfo);
			}
			catch(RegistryException& e)
			{
				LOG(ERROR) << "Throw exception.. The object not in registry and unable to register or build" << endl;
				LOG(ERROR) << e.what();
				throw e;
			}

			/// now build the DailyStockInfo
			std::shared_ptr<IObject> obj = EntityMgrUtil::fetch(nm, source);
			histStockInfo = dynamic_pointer_cast<HistoricalStockInfo>(obj);
		}
		else
		{
			histStockInfo = dynamic_pointer_cast<HistoricalStockInfo>(obj);
		}
		return histStockInfo;
	}

	void StoreHistoricalStockInfo(ushort source, const std::shared_ptr<HistoricalStockInfo> & histStock)
	{
		std::shared_ptr<IDataSource> src = EntityMgrUtil::getDataSourceHandler(histStock->GetName(), source);
		src->insert(histStock);
	}
}
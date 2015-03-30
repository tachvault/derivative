# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# add MySQL Server and connector includes
include_directories ("${MYSQL_ROOT}/include")
include_directories ("${MYSQL_CONN_ROOT}")
include_directories ("${MYSQL_CONN_ROOT}/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")

SET(MYSQLDATAACCESS_SOURCE
	ExchangeRateMySQLDAO.cpp
	ExchangeRateMySQLDAO.hpp
	HistoricalExchangeRateInfoMySQLDAO.cpp
	HistoricalExchangeRateInfoMySQLDAO.hpp
	HistoricalStockInfoMySQLDAO.cpp
	HistoricalStockInfoMySQLDAO.hpp
	IRMySQLDAO.cpp
	IRMySQLDAO.hpp
	IRValueMySQLDAO.cpp
	IRValueMySQLDAO.hpp
	LIBORMySQLDAO.cpp
	LIBORMySQLDAO.hpp
	LIBORValueMySQLDAO.cpp
	LIBORValueMySQLDAO.hpp
	StockMySQLDAO.cpp
	StockMySQLDAO.hpp
	FuturesMySQLDAO.cpp
	FuturesMySQLDAO.hpp
	FuturesValueMySQLDAO.hpp
	FuturesValueMySQLDAO.cpp
	ZeroCouponBondMySQLDAO.hpp
	ZeroCouponBondMySQLDAO.cpp
	ZeroCouponBondValueMySQLDAO.hpp
	ZeroCouponBondValueMySQLDAO.cpp
	FixedRateBondMySQLDAO.hpp
	FixedRateBondMySQLDAO.cpp
	FixedRateBondValueMySQLDAO.hpp
	FixedRateBondValueMySQLDAO.cpp
	DailyEquityOptionValueMySQLDAO.hpp
	DailyEquityOptionValueMySQLDAO.cpp
	DailyFuturesOptionValueMySQLDAO.hpp
	DailyFuturesOptionValueMySQLDAO.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${MYSQL_ROOT})
link_directories("${MYSQL_CONN_ROOT}/driver")
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(MYSQLDATAACCESS SHARED ${MYSQLDATAACCESS_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (MYSQLDATAACCESS ${GLOG_LIBRARY})
target_link_libraries (MYSQLDATAACCESS ${GFLAG_LIBRARY})
target_link_libraries (MYSQLDATAACCESS ${MYSQL_LIBRARY})
target_link_libraries (MYSQLDATAACCESS ${MYSQL_CONN_LIBRARY})
target_link_libraries (MYSQLDATAACCESS ENTITYMGMT)
target_link_libraries (MYSQLDATAACCESS SERVICEUTIL)
target_link_libraries (MYSQLDATAACCESS DATASOURCE_MYSQL)
target_link_libraries (MYSQLDATAACCESS FINUTILITY)
target_link_libraries (MYSQLDATAACCESS PRIMARYASSET_EXT)

# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

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

#define DLL EXPORTS for this DLL
add_definitions(-DFINUTILITY_EXPORTS)

SET(FINUTILITY_SOURCE
    ${PROJECT_SOURCE_DIR}/Service/include/SpinLock.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/Country.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/CountryHolder.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/Currency.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/Exchange.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/ExchangeHolder.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/ExchangeExt.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/DayCount.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/Maturity.hpp
	Country.cpp
	CountryHolder.cpp
	CountryMySQLDAO.cpp
	CountryMySQLDAO.hpp
	Currency.cpp
	CurrencyHolder.cpp
	CurrencyMySQLDAO.cpp
	CurrencyMySQLDAO.hpp
	FinUtilDLLMain.cpp
	Exchange.cpp
	ExchangeHolder.cpp
	ExchangeMySQLDAO.cpp
	ExchangeMySQLDAO.hpp
	ExchangeExt.cpp
	ExchangeExtMySQLDAO.cpp
	ExchangeExtMySQLDAO.hpp
	DayCount.cpp
	Maturity.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${MYSQL_ROOT})
link_directories("${MYSQL_CONN_ROOT}/driver")
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(FINUTILITY SHARED ${FINUTILITY_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (FINUTILITY ${GLOG_LIBRARY})
target_link_libraries (FINUTILITY debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (FINUTILITY ${MYSQL_LIBRARY})
target_link_libraries (FINUTILITY ${MYSQL_CONN_LIBRARY})
target_link_libraries (FINUTILITY ENTITYMGMT)
target_link_libraries (FINUTILITY SERVICEUTIL)
target_link_libraries (FINUTILITY DATASOURCE_MYSQL)

# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")

#define DLL EXPORTS for this DLL
add_definitions(-DPRIMARYASSET_EXT_EXPORTS)

SET(PRIMARYASSET_EXT_SOURCE
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/CSVReader.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/FlatTermStructure.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/HistoricalExchangeRateInfo.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/HistoricalStockInfo.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyExchangeRateValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyStockValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TermStructure.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TSBootstrap.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TSLinear.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TSLogLinear.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TSPayoff.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/RESTConnectionUtil.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IRCurve.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/PrimaryAssetUtil.hpp
	CSVReader.cpp
	dllmain.cpp
	FlatTermStructure.cpp
	HistoricalExchangeRateInfo.cpp
	HistoricalStockInfo.cpp
	Money.cpp
	TermStructure.cpp
	TSBootstrap.cpp
	TSLinear.cpp
	TSLogLinear.cpp
	IRCurve.cpp
	PrimaryAssetUtil.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(PRIMARYASSET_EXT SHARED ${PRIMARYASSET_EXT_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRIMARYASSET_EXT ${GLOG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT ${GFLAG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT ENTITYMGMT)
target_link_libraries (PRIMARYASSET_EXT FINUTILITY)
target_link_libraries (PRIMARYASSET_EXT SERVICEUTIL)
target_link_libraries (PRIMARYASSET_EXT ASSET_PROPERTIES)

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
add_definitions(-DPRIMARYTESTASSET_EXT_EXPORTS)

SET(PRIMARYTESTASSET_EXT_SOURCE
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/TestUtil.hpp
	TestUtil.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(PRIMARYTESTASSET_EXT ${PRIMARYTESTASSET_EXT_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRIMARYTESTASSET_EXT ${GLOG_LIBRARY})
target_link_libraries (PRIMARYTESTASSET_EXT debug ${GTEST_DEBUG_LIBRARY} optimized ${GTEST_REL_LIBRARY})
target_link_libraries (PRIMARYTESTASSET_EXT ENTITYMGMT)
target_link_libraries (PRIMARYTESTASSET_EXT FINUTILITY)
target_link_libraries (PRIMARYTESTASSET_EXT SERVICEUTIL)
target_link_libraries (PRIMARYTESTASSET_EXT ASSET_PROPERTIES)

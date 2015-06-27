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
add_definitions(-DASSET_PROPERTIES_EXPORTS)

SET(ASSET_PROPERTIES_SOURCE
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/ConstVol.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/DeterministicAssetVolDiff.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/DeterministicCashflow.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/DeterministicVol.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/DeterministicVolMediator.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/ExponentialVol.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/PiecewiseVol.hpp
	ConstVol.cpp
	DeterministicAssetVolDiff.cpp
	DeterministicCashflow.cpp
	DeterministicVol.cpp
	DeterministicVolMediator.cpp
	ExponentialVol.cpp
	PiecewiseVol.cpp
	Asset_Properties.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(ASSET_PROPERTIES SHARED ${ASSET_PROPERTIES_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (ASSET_PROPERTIES ${GLOG_LIBRARY})
target_link_libraries (ASSET_PROPERTIES debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (ASSET_PROPERTIES SERVICEUTIL)


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

SET(DERIVATIVEASSET_SOURCE
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/BlackScholesAsset.hpp
	BlackScholesAsset.cpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/BlackScholesAssetAdapter.hpp
	BlackScholesAssetAdapter.cpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/GenericBlackScholes.hpp
	GenericBlackScholes.cpp	
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/GramCharlierAsset.hpp	
	GramCharlierAsset.cpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/GramCharlierAssetAdapter.hpp	
	GramCharlierAssetAdapter.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(DERIVATIVEASSET SHARED ${DERIVATIVEASSET_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (DERIVATIVEASSET ${GLOG_LIBRARY})
target_link_libraries (DERIVATIVEASSET debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (DERIVATIVEASSET ENTITYMGMT)
target_link_libraries (DERIVATIVEASSET FINUTILITY)
target_link_libraries (DERIVATIVEASSET SERVICEUTIL)
target_link_libraries (DERIVATIVEASSET PRIMARYASSET_EXT)

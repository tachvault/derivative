# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")

SET(XIGNITEDATAACCESS_SOURCE
	IRValueXigniteDAO.cpp
	IRValueXigniteDAO.hpp
        LIBORValueXigniteDAO.cpp
	LIBORValueXigniteDAO.hpp
        HistoricalExchangeRateInfoXigniteDAO.hpp
        HistoricalExchangeRateInfoXigniteDAO.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${RESTSDK_ROOT}/Binaries/Win32/${CMAKE_BUILD_TYPE}")
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(XIGNITEDATAACCESS SHARED ${XIGNITEDATAACCESS_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (XIGNITEDATAACCESS ${GLOG_LIBRARY})
target_link_libraries (XIGNITEDATAACCESS debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (XIGNITEDATAACCESS debug ${RESTSDK_DEBUG_LIBRARY} optimized ${RESTSDK_REL_LIBRARY})
target_link_libraries (XIGNITEDATAACCESS ENTITYMGMT)
target_link_libraries (XIGNITEDATAACCESS SERVICEUTIL)
target_link_libraries (XIGNITEDATAACCESS DATASOURCE_REST)
target_link_libraries (XIGNITEDATAACCESS FINUTILITY)
target_link_libraries (XIGNITEDATAACCESS PRIMARYASSET_EXT)

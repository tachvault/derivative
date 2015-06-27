# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

#POCO includes
include_directories("${POCO_ROOT}/Foundation/include")
include_directories("${POCO_ROOT}/Net/include")

# add glog/gflag/gTest directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")
include_directories("${GTEST_ROOT}/include")

# add MySQL Server and connector includes
include_directories ("${MYSQL_ROOT}/include")
include_directories ("${MYSQL_CONN_ROOT}")
include_directories ("${MYSQL_CONN_ROOT}/include")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")
include_directories("${PROJECT_SOURCE_DIR}/AssetPricing/include")

SET(CMEFILELOADER_SOURCES
	CMEFileLoader.cpp
    FuturesValueMySQLDAO.hpp
	FuturesValueMySQLDAO.cpp
    FuturesOptionValueMySQLDAO.hpp
	FuturesOptionValueMySQLDAO.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(CMEFILELOADER ${CMEFILELOADER_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (CMEFILELOADER ${GLOG_LIBRARY})
target_link_libraries (CMEFILELOADER debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (CMEFILELOADER debug ${RESTSDK_DEBUG_LIBRARY} optimized ${RESTSDK_REL_LIBRARY})
target_link_libraries (CMEFILELOADER debug ${POCOFOUNDATION_DEBUG_LIBRARY} optimized ${POCOFOUNDATION_REL_LIBRARY})
target_link_libraries (CMEFILELOADER debug ${POCONET_DEBUG_LIBRARY} optimized ${POCONET_REL_LIBRARY})
target_link_libraries (CMEFILELOADER ENTITYMGMT)
target_link_libraries (CMEFILELOADER SERVICEUTIL)
target_link_libraries (CMEFILELOADER FINUTILITY)
target_link_libraries (CMEFILELOADER PRIMARYASSET_EXT)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
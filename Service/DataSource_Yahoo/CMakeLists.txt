# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")

#define DLL EXPORTS for this DLL
add_definitions(-DDATASOURCE_YAHOO_EXPORTS)

SET(DATASOURCE_YAHOO_SOURCE
	YahooDataSource.hpp
	YahooDataSource.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(DATASOURCE_YAHOO SHARED ${DATASOURCE_YAHOO_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (DATASOURCE_YAHOO ${GLOG_LIBRARY})
target_link_libraries (DATASOURCE_YAHOO ${GFLAG_LIBRARY})
target_link_libraries (DATASOURCE_YAHOO ENTITYMGMT)
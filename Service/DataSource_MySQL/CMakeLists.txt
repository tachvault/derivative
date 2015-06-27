# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")

#define DLL EXPORTS for this DLL
add_definitions(-DDATASOURCE_MYSQL_EXPORTS)

SET(DATASOURCE_MYSQL_SOURCE
	MySQLDataSource.cpp
	MySQLDataSource.hpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(DATASOURCE_MYSQL SHARED ${DATASOURCE_MYSQL_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (DATASOURCE_MYSQL ${GLOG_LIBRARY})
target_link_libraries (DATASOURCE_MYSQL debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (DATASOURCE_MYSQL ENTITYMGMT)
target_link_libraries (DATASOURCE_MYSQL SERVICEUTIL)
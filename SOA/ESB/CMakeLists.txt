# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# add MySQL Server and connector includes
include_directories ("${MYSQL_ROOT}/include")
include_directories ("${MYSQL_CONN_ROOT}")
include_directories ("${MYSQL_CONN_ROOT}/include")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/SOA/include")

#define DLL EXPORTS for this DLL
add_definitions(-DESB_EXPORTS)

SET(ESB_SOURCES
    	${PROJECT_SOURCE_DIR}/SOA/include/ESBManager.hpp
	ESBManager.cpp
	ServiceRouter.hpp
        ServiceRouter.cpp
        SecurityManager.hpp
        SecurityManager.cpp
        ServiceLogger.hpp
        ServiceLogger.cpp
        UserInfoHolder.hpp
        UserInfoHolder.cpp
        UserInfoMySQLDAO.hpp
        UserInfoMySQLDAO.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${MYSQL_ROOT})
link_directories("${MYSQL_CONN_ROOT}/driver")
link_directories("${RESTSDK_ROOT}/Binaries/Win32/${CMAKE_BUILD_TYPE}")
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(ESB SHARED ${ESB_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (ESB ${GLOG_LIBRARY})
target_link_libraries (ESB ${GFLAG_LIBRARY})
target_link_libraries (ESB ${MYSQL_LIBRARY})
target_link_libraries (ESB ${MYSQL_CONN_LIBRARY})
target_link_libraries (ESB ${RESTSDK_LIBRARY})
target_link_libraries (ESB ENTITYMGMT)
target_link_libraries (ESB SERVICEUTIL)
target_link_libraries (ESB MESSAGEDISPATCHER)
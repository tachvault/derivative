# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/SOA/include")

#define DLL EXPORTS for this DLL
add_definitions(-DESB_EXPORTS)

SET(WEBSERVICE_SOURCES
    	${PROJECT_SOURCE_DIR}/SOA/include/IRequestInterceptor.hpp
	${PROJECT_SOURCE_DIR}/SOA/include/IRESTJSONRequestInterceptor.hpp
	RESTJSONRequestInterceptor.hpp
        RESTJSONRequestInterceptor.cpp
        WebServiceUtil.hpp
        WebServiceUtil.cpp      
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(WEBSERVICE SHARED ${WEBSERVICE_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (WEBSERVICE ${GLOG_LIBRARY})
target_link_libraries (WEBSERVICE debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (WEBSERVICE debug ${RESTSDK_DEBUG_LIBRARY} optimized ${RESTSDK_REL_LIBRARY})
target_link_libraries (WEBSERVICE ENTITYMGMT)
target_link_libraries (WEBSERVICE ESB)
target_link_libraries (WEBSERVICE MESSAGES)
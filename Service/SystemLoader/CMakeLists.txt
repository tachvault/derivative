# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/SOA/include")

SET(SYSTEMLOADER_SOURCES
        BPMLoader.hpp        
        BPMLoader.cpp
    	Loader.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(SYSTEMLOADER ${SYSTEMLOADER_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (SYSTEMLOADER ${GLOG_LIBRARY})
target_link_libraries (SYSTEMLOADER ${GFLAG_LIBRARY})
target_link_libraries (SYSTEMLOADER ENTITYMGMT)
target_link_libraries (SYSTEMLOADER SERVICEUTIL)
target_link_libraries (SYSTEMLOADER MESSAGEDISPATCHER)
target_link_libraries (SYSTEMLOADER ESB)
target_link_libraries (SYSTEMLOADER WEBSERVICE)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
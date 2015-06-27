# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")

#define DLL EXPORTS for this DLL
add_definitions(-DMESSAGEDISPATCHER_EXPORTS)

SET(MESSAGEDISPATCHER_SOURCES
    	${PROJECT_SOURCE_DIR}/Service/include/MsgProcessorManager.hpp
	    ${PROJECT_SOURCE_DIR}/Service/include/MsgRegister.hpp
		${PROJECT_SOURCE_DIR}/Service/include/MessageDispatcher.hpp
		${PROJECT_SOURCE_DIR}/Service/include/ObjectPool.hpp
	    MsgDispatcherDLLMain.cpp
	    MsgProcessorManager.cpp
	    MsgRegister.cpp        
        MessageDispatcher.cpp 
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${RESTSDK_ROOT}/Binaries/Win32/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(MESSAGEDISPATCHER SHARED ${MESSAGEDISPATCHER_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (MESSAGEDISPATCHER ${GLOG_LIBRARY})
target_link_libraries (MESSAGEDISPATCHER debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (MESSAGEDISPATCHER ENTITYMGMT)
target_link_libraries (MESSAGEDISPATCHER SERVICEUTIL)
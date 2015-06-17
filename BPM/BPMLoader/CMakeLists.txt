# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")

SET(BPMLOADER_SOURCE
        ${PROJECT_SOURCE_DIR}/BPM/include/BPMLoader.hpp
	BPMLoader.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(BPMLOADER SHARED ${BPMLOADER_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (BPMLOADER ${GLOG_LIBRARY})
target_link_libraries (BPMLOADER ${GFLAG_LIBRARY})
target_link_libraries (BPMLOADER ENTITYMGMT)
target_link_libraries (BPMLOADER FINUTILITY)
target_link_libraries (BPMLOADER SERVICEUTIL)
target_link_libraries (BPMLOADER PRIMARYASSET_EXT)
target_link_libraries (BPMLOADER DERIVATIVEASSET)

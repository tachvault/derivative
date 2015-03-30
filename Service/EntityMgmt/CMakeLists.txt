# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")

#define DLL EXPORTS for this DLL
add_definitions(-DENTITYMGMT_EXPORTS)

SET(ENTITY_MGMT_SOURCES
    ${PROJECT_SOURCE_DIR}/Service/include/EntityGroup.hpp
	${PROJECT_SOURCE_DIR}/Service/include/EntityManager.hpp
	${PROJECT_SOURCE_DIR}/Service/include/SystemManager.hpp
	${PROJECT_SOURCE_DIR}/Service/include/GroupRegister.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Name.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Config.hpp
	EntityMgmtDLLMain.cpp
	EntityGroup.cpp
	EntityManager.cpp
	SystemManager.cpp
	GroupRegister.cpp
	Name.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(ENTITYMGMT SHARED ${ENTITY_MGMT_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (ENTITYMGMT ${GLOG_LIBRARY})
target_link_libraries (ENTITYMGMT ${GFLAG_LIBRARY})
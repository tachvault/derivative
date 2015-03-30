# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag/gTest directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")
include_directories("${GTEST_ROOT}/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")

SET(PRIMARYASSET_EXT_FUNCTEST_SOURCES
	primaryAsset_Ext_FuncTest.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(PRIMARYASSET_EXT_FUNCTEST ${PRIMARYASSET_EXT_FUNCTEST_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST ${GLOG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST ${GFLAG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST ${GTEST_LIBRARY})
#target_link_libraries (PRIMARYASSET_EXT_FUNCTEST ${GTEST_MAIN_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST ENTITYMGMT)
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST SERVICEUTIL)
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST FINUTILITY)
target_link_libraries (PRIMARYASSET_EXT_FUNCTEST PRIMARYASSET_EXT)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
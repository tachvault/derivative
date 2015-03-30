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

SET(PRIMARYASSET_EXT_UNITTEST_SOURCES
	PRIMARYASSET_Ext_UnitTest.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(PRIMARYASSET_EXT_UNITTEST ${PRIMARYASSET_EXT_UNITTEST_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRIMARYASSET_EXT_UNITTEST ${GLOG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_UNITTEST ${GFLAG_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_UNITTEST ${GTEST_LIBRARY})
target_link_libraries (PRIMARYASSET_EXT_UNITTEST ENTITYMGMT)
target_link_libraries (PRIMARYASSET_EXT_UNITTEST SERVICEUTIL)
target_link_libraries (PRIMARYASSET_EXT_UNITTEST FINUTILITY)
target_link_libraries (PRIMARYASSET_EXT_UNITTEST PRIMARYASSET_EXT)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
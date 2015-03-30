# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag/gTest directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")
include_directories("${GTEST_ROOT}/include")
include_directories("${GMOCK_ROOT}/include")


# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")
include_directories("${PROJECT_SOURCE_DIR}/Test/UnitTest/include")

SET(ENTITY_MGMT_SOURCES
	EntityMgmt_UnitTest.cpp
	${PROJECT_SOURCE_DIR}/Test/UnitTest/include/MockStock.hpp
	${PROJECT_SOURCE_DIR}/Test/UnitTest/include/MockStockValue.hpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GMOCK_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(ENTITYMGMT_UNITTEST ${ENTITY_MGMT_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (ENTITYMGMT_UNITTEST ${GLOG_LIBRARY})
target_link_libraries (ENTITYMGMT_UNITTEST ${GFLAG_LIBRARY})
target_link_libraries (ENTITYMGMT_UNITTEST ${GTEST_LIBRARY})
target_link_libraries (ENTITYMGMT_UNITTEST ${GMOCK_LIBRARY})
target_link_libraries (ENTITYMGMT_UNITTEST ENTITYMGMT)
target_link_libraries (ENTITYMGMT_UNITTEST SERVICEUTIL)
target_link_libraries (ENTITYMGMT_UNITTEST FINUTILITY)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)

# Copy third party dynamic/shared libraries and associated files into target build directory
ADD_CUSTOM_COMMAND(TARGET ENTITYMGMT_UNITTEST
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/common/Libraries/${CMAKE_BUILD_TYPE}-Version" "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")
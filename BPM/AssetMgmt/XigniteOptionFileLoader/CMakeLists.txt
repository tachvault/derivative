# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag/gTest directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")
include_directories("${GTEST_ROOT}/include")

# add MySQL Server and connector includes
include_directories ("${MYSQL_ROOT}/include")
include_directories ("${MYSQL_CONN_ROOT}")
include_directories ("${MYSQL_CONN_ROOT}/include")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")
include_directories("${PROJECT_SOURCE_DIR}/AssetPricing/include")

SET(XIGNITEOPTIONFILELOADER_SOURCES
	XigniteOptionLoader.cpp
        OptionFile.hpp
        OptionXigniteDAO.hpp
        OptionXigniteDAO.cpp
		OptionFileMySQLDAO.hpp
		OptionFileMySQLDAO.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")

add_executable(XIGNITEOPTIONFILELOADER ${XIGNITEOPTIONFILELOADER_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (XIGNITEOPTIONFILELOADER ${GLOG_LIBRARY})
target_link_libraries (XIGNITEOPTIONFILELOADER debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (XIGNITEOPTIONFILELOADER debug ${RESTSDK_DEBUG_LIBRARY} optimized ${RESTSDK_REL_LIBRARY})
target_link_libraries (XIGNITEOPTIONFILELOADER ENTITYMGMT)
target_link_libraries (XIGNITEOPTIONFILELOADER SERVICEUTIL)
target_link_libraries (XIGNITEOPTIONFILELOADER FINUTILITY)
target_link_libraries (XIGNITEOPTIONFILELOADER PRIMARYASSET_EXT)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
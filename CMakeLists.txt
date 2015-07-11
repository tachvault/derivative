cmake_minimum_required (VERSION 2.6)

# The name of our project is "Derivative". CMakeLists files in this project can 
# refer to the root source directory of the project as ${PROJECT_SOURCE_DIR} and 
# to the root binary directory of the project as ${PROJECT_BINARY_DIR}. 
# Maps to Visual Studio solution file (Derivative.sln)
# The solution will have all targets (exe, lib, dll) 
# as Visual Studio projects (.vcproj)
project (Derivative)

# The version number.
set (PROJECT_VERSION_MAJOR 2)
set (PROJECT_VERSION_MINOR 0)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

# Setup environment level variables for folders
# Setup Blitz root directory
set (BLITZ_ROOT $ENV{BLITZ_DIR})
message ("Blitz directory : " ${BLITZ_ROOT})

# Setup MySQL root directory
set (MYSQL_ROOT $ENV{MYSQL_DIR})
message ("MySQL directory : " ${MYSQL_ROOT})

# Setup MySQL C++ connection directory
set (MYSQL_CONN_ROOT $ENV{MYSQL_CONN_DIR})
message ("MySQL connection directory : " ${MYSQL_CONN_ROOT})

# Setup Microsoft Casablanca REST SDK directory
set (RESTSDK_ROOT $ENV{CASABLANCA_ROOT})
message ("REST SDK connection directory : " ${RESTSDK_ROOT})

# Setup POCO root directory
set (POCO_ROOT $ENV{POCO_DIR})
message ("POCO directory : " ${POCO_ROOT})

# Setup Google log directory
set (GLOG_ROOT $ENV{GLOG_ROOT})
message ("Google log directory :" ${GLOG_ROOT})

# Setup Google flag directory
set (GFLAG_ROOT $ENV{GFLAG_ROOT})
message ("Google flag directory : " ${GFLAG_ROOT})

# Setup Google Test directory
set (GTEST_ROOT $ENV{GTEST_ROOT})
message ("Google Test directory : " ${GTEST_ROOT})

# Setup Google Test directory
set (GMOCK_ROOT $ENV{GMOCK_ROOT})
message ("Google Mock directory : " ${GMOCK_ROOT})

# Setup clapack math lib directory
set (CLAPACK_ROOT "${PROJECT_SOURCE_DIR}/common/clapack")
message ("clapack math lib directory : " ${CLAPACK_ROOT})

#  Let CMake find the modules and initialize for the libraries
find_package(Boost)
if(Boost_FOUND)
  message ("boost include directory : " ${Boost_INCLUDE_DIRS}) 
  message ("boost link directory : " ${Boost_LIBRARY_DIRS})
endif()
message ("Boost Library dir : " ${Boost_LIBRARY_DIRS})

FIND_LIBRARY(MYSQL_LIBRARY NAMES libmysql PATHS ${MYSQL_ROOT} PATH_SUFFIXES lib)
FIND_LIBRARY(MYSQL_CONN_LIBRARY NAMES mysqlcppconn PATHS "${MYSQL_CONN_ROOT}/lib/${CMAKE_BUILD_TYPE}" PATH_SUFFIXES lib)

FIND_LIBRARY(GLOG_LIBRARY NAMES libglog PATHS "${GLOG_ROOT}/${CMAKE_BUILD_TYPE}" PATH_SUFFIXES lib)

FIND_LIBRARY(GFLAG_DEBUG_LIBRARY NAMES libgflags-debug PATHS "${GFLAG_ROOT}/Debug" PATH_SUFFIXES lib)
FIND_LIBRARY(GFLAG_REL_LIBRARY NAMES libgflags PATHS "${GFLAG_ROOT}/Release" PATH_SUFFIXES lib)

FIND_LIBRARY(RESTSDK_DEBUG_LIBRARY NAMES cpprest120d_2_6 PATHS "${RESTSDK_ROOT}/Binaries/Win32/Debug" PATH_SUFFIXES lib)
FIND_LIBRARY(RESTSDK_REL_LIBRARY NAMES cpprest120_2_6 PATHS "${RESTSDK_ROOT}/Binaries/Win32/Release" PATH_SUFFIXES lib)

FIND_LIBRARY(POCOFOUNDATION_REL_LIBRARY NAMES PocoFoundation PATHS "${POCO_ROOT}/lib" PATH_SUFFIXES lib)
FIND_LIBRARY(POCOFOUNDATION_DEBUG_LIBRARY NAMES PocoFoundationd PATHS "${POCO_ROOT}/lib" PATH_SUFFIXES lib)
FIND_LIBRARY(POCONET_REL_LIBRARY NAMES PocoNet PATHS "${POCO_ROOT}/lib" PATH_SUFFIXES lib)
FIND_LIBRARY(POCONET_DEBUG_LIBRARY NAMES PocoNetd PATHS "${POCO_ROOT}/lib" PATH_SUFFIXES lib)

FIND_LIBRARY(CLAPACK_LIBRARY NAMES clapack PATHS ${CLAPACK_ROOT} PATH_SUFFIXES lib)
FIND_LIBRARY(BLAS_LIBRARY NAMES blas PATHS ${CLAPACK_ROOT} PATH_SUFFIXES lib)
FIND_LIBRARY(LIBF77_LIBRARY NAMES libF77 PATHS ${CLAPACK_ROOT} PATH_SUFFIXES lib)
FIND_LIBRARY(LIBI77_LIBRARY NAMES libI77 PATHS ${CLAPACK_ROOT} PATH_SUFFIXES lib)

# Google Test library
FIND_LIBRARY(GTEST_DEBUG_LIBRARY NAMES gtestd PATHS "${GTEST_ROOT}/msvc/gtest-md/Debug" PATH_SUFFIXES lib)
FIND_LIBRARY(GTEST_REL_LIBRARY NAMES gtest PATHS "${GTEST_ROOT}/msvc/gtest-md/Release" PATH_SUFFIXES lib)

# Google Mock library
FIND_LIBRARY(GMOCK_LIBRARY NAMES gmock PATHS "${GMOCK_ROOT}/msvc/2010/${CMAKE_BUILD_TYPE}" PATH_SUFFIXES lib)

# Set the executable and library output directories
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build")

# Recurse into the "Derivative" directory. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 
add_subdirectory (Service) 
add_subdirectory (BPM)
add_subdirectory (SOA)
add_subdirectory (Test)

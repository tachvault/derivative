# Install script for directory: C:/work/derivative/Test/FuncTest

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Derivative")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/work/derivative/Test/FuncTest/FinUtility_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/MySQLDataAccess_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/YahooDataAccess_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/PrimaryAsset_ext_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/Binomial_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/FD_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/GCempirical_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/HJM_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/MBinary_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/MonteCarlo_FuncTest/cmake_install.cmake")
  include("C:/work/derivative/Test/FuncTest/Facades_FuncTest/cmake_install.cmake")

endif()

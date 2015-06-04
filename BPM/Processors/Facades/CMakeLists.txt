# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# add glog/gflag/gTest directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")
include_directories("${GTEST_ROOT}/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/SOA/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include")

SET(FACADES_SOURCES
	EquityOption.hpp
        EquityOption.cpp
        EquityVanillaOption.hpp
        EquityVanillaOption.cpp
        FuturesVanillaOption.hpp
        ExchangeRateVanillaOption.cpp
        ExchangeRateVanillaOption.hpp
        FuturesVanillaOption.cpp
        EquityOptionSpread.hpp
        EquityOptionSpread.cpp
        FuturesOptionSpread.hpp
        FuturesOptionSpread.cpp
        ExchangeRateOptionSpread.hpp
        ExchangeRateOptionSpread.cpp
        EquityBarrierOption.hpp
        EquityBarrierOption.cpp
        EquityAverageOption.hpp
        EquityAverageOption.cpp
        EquityLookBackOption.hpp
        EquityLookBackOption.cpp
        EquityChooserOption.hpp
        EquityChooserOption.cpp
        EquityMargrabeOption.hpp
        EquityMargrabeOption.cpp             
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GTEST_ROOT}/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(FACADES SHARED ${FACADES_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (FACADES ${GLOG_LIBRARY})
target_link_libraries (FACADES ${GFLAG_LIBRARY})
target_link_libraries (FACADES ${RESTSDK_LIBRARY})
target_link_libraries (FACADES ENTITYMGMT)
target_link_libraries (FACADES SERVICEUTIL)
target_link_libraries (FACADES MESSAGEDISPATCHER)
target_link_libraries (FACADES FINUTILITY)
target_link_libraries (FACADES PRIMARYASSET_EXT)
target_link_libraries (FACADES DERIVATIVEASSET)
target_link_libraries (FACADES PRICINGENGINE)

# Set the preprocessor definition for _VARIADIC_MAX=10
add_definitions(-D_VARIADIC_MAX=10)
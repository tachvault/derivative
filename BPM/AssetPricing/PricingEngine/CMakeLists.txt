# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetMgmt/include")
include_directories("${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include")

SET(PRICINGENGINE_SOURCE
    ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/Binomial.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/FiniteDifference.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussMarkovWorld.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussMarkovWorldAdapter.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussianEconomy.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussianEconomyAdapter.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussianHJM.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/LongstaffSchwartz.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MBinary.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCPayoff.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCMapping.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCControlVariateMapping.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCGatherer.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCGeneric.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MExotics.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/Payoff.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/TSBinomial.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/TSInstruments.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GeometricBrownianMotion.hpp	
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GaussMarkovTermStructure.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/VolatilitySurface.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/EquityVolatilitySurface.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/FuturesVolatilitySurface.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/CIRprocess.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/HestonAsset.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/EquityAssetPricer.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/FuturesAssetPricer.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/ExchangeRateAssetPricer.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/GARCH.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/EquityGARCH.hpp
	${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/ExchangeRateGARCH.hpp
        ${PROJECT_SOURCE_DIR}/BPM/AssetPricing/include/MCAssetPricer.hpp
        Binomial.cpp
	FiniteDifference.cpp
	GaussMarkovWorld.cpp
	GaussMarkovWorldAdapter.cpp
	GaussianEconomy.cpp
	GaussianEconomyAdapter.cpp
	GaussianHJM.cpp
	LongstaffSchwartz.cpp
	MBinary.cpp
	MBinaryGaussianHJM.cpp
	MCPayoff.cpp
	MCGatherer.cpp
	MExotics.cpp
	Payoff.cpp
	TSBinomial.cpp
	TSInstruments.cpp
	TSPayoff.cpp
	GeometricBrownianMotion.cpp
	GaussMarkovTermStructure.cpp
        VolatilitySurface.cpp
        GARCH.cpp
        EquityGARCH.cpp
        ExchangeRateGARCH.cpp
	EquityVolatilitySurface.cpp
	FuturesVolatilitySurface.cpp
	HestonAsset.cpp
        EquityAssetPricer.cpp
        FuturesAssetPricer.cpp
        ExchangeRateAssetPricer.cpp
        MCAssetPricer.cpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(PRICINGENGINE SHARED ${PRICINGENGINE_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRICINGENGINE ${GLOG_LIBRARY})
target_link_libraries (PRICINGENGINE debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (PRICINGENGINE ENTITYMGMT)
target_link_libraries (PRICINGENGINE FINUTILITY)
target_link_libraries (PRICINGENGINE SERVICEUTIL)
target_link_libraries (PRICINGENGINE PRIMARYASSET_EXT)
target_link_libraries (PRICINGENGINE DERIVATIVEASSET)

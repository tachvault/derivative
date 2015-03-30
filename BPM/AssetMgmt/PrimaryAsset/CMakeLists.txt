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

SET(PRIMARYASSET_SOURCE
	DailyExchangeRateValue.cpp
	DailyExchangeRateValue.hpp
	DailyStockValue.cpp
	DailyStockValue.hpp
	ExchangeRate.cpp
	ExchangeRate.hpp
	ExchangeRateValue.cpp
	ExchangeRateValue.hpp
	IR.hpp
	IR.cpp
	IRValue.cpp
	IRValue.hpp
	LIBOR.cpp
	LIBOR.hpp
	LIBORValue.cpp
	LIBORValue.hpp
	Stock.cpp
	Stock.hpp
	StockValue.cpp
	StockValue.hpp
	Futures.cpp
	Futures.hpp
	FuturesValue.cpp
	FuturesValue.hpp
	ZeroCouponBond.hpp
	ZeroCouponBond.cpp
	ZeroCouponBondValue.hpp
	ZeroCouponBondValue.cpp
	FixedRateBond.hpp
	FixedRateBond.cpp
	FixedRateBondValue.hpp
	FixedRateBondValue.cpp
	DailyEquityOptionValue.cpp
	DailyEquityOptionValue.hpp	
	DailyFuturesOptionValue.cpp
	DailyFuturesOptionValue.hpp	
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IAsset.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IAssetValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IPrimitiveSecurity.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyExchangeRateValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyExchangeRateValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyStockValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IExchangeRate.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IExchangeRateValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IIR.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IIRValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IIBOR.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IIBORValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IStock.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IStockValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IFutures.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IFuturesValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyStockValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IBond.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IBondValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IZeroCouponBond.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IZeroCouponBondValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IFixedRateBond.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IFixedRateBondValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyEquityOptionValue.hpp
	${PROJECT_SOURCE_DIR}/BPM/assetmgmt/include/IDailyFuturesOptionValue.hpp
)

# Link shared library paths
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${GLOG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories(${GFLAG_ROOT}/${CMAKE_BUILD_TYPE})
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(PRIMARYASSET SHARED ${PRIMARYASSET_SOURCE})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (PRIMARYASSET ${GLOG_LIBRARY})
target_link_libraries (PRIMARYASSET ${GFLAG_LIBRARY})
target_link_libraries (PRIMARYASSET ASSET_PROPERTIES)
target_link_libraries (PRIMARYASSET ENTITYMGMT)
target_link_libraries (PRIMARYASSET FINUTILITY)
target_link_libraries (PRIMARYASSET SERVICEUTIL)

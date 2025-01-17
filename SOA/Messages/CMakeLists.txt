# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add Blitz includes
include_directories(${BLITZ_ROOT})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# Add RESTSDK include_directories
include_directories("${RESTSDK_ROOT}/Release/include")

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")
include_directories("${PROJECT_SOURCE_DIR}/SOA/include")

#define exports for this library
add_definitions(-DMESSAGES_EXPORTS)

SET(MESSAGES_SOURCES
    	${PROJECT_SOURCE_DIR}/service/include/IMessage.hpp
	${PROJECT_SOURCE_DIR}/service/include/IVisitor.hpp
	${PROJECT_SOURCE_DIR}/SOA/include/VanillaOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityVanillaOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesVanillaOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateVanillaOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityVanillaOptJSONVisitor.hpp
	${PROJECT_SOURCE_DIR}/SOA/include/FuturesVanillaOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateVanillaOptJSONVisitor.hpp
	${PROJECT_SOURCE_DIR}/SOA/include/OptionSpreadMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityOptionSpreadMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityOptionSpreadJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesOptionSpreadMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesOptionSpreadJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateOptionSpreadMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateOptionSpreadJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityBarrierOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityBarrierOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityAverageOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityAverageOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityLookBackOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityLookBackOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityChooserOptJSONVisitor.hpp                                          
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityChooserOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityMargrabeOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/EquityMargrabeOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesBarrierOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesBarrierOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesAverageOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/FuturesAverageOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateBarrierOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateBarrierOptMessage.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateAverageOptJSONVisitor.hpp
        ${PROJECT_SOURCE_DIR}/SOA/include/ExchangeRateAverageOptMessage.hpp
        VanillaOptMessage.cpp
	EquityVanillaOptMessage.cpp
	FuturesVanillaOptMessage.cpp
        ExchangeRateVanillaOptMessage.cpp        
        EquityVanillaOptJSONVisitor.cpp
	FuturesVanillaOptJSONVisitor.cpp
        ExchangeRateVanillaOptJSONVisitor.cpp 
        OptionSpreadMessage.cpp
        EquityOptionSpreadMessage.cpp
        EquityOptionSpreadJSONVisitor.cpp
        FuturesOptionSpreadMessage.cpp
        FuturesOptionSpreadJSONVisitor.cpp
        ExchangeRateOptionSpreadMessage.cpp
        ExchangeRateOptionSpreadJSONVisitor.cpp
        EquityBarrierOptJSONVisitor.cpp
        EquityBarrierOptMessage.cpp
        EquityAverageOptJSONVisitor.cpp
        EquityAverageOptMessage.cpp
        EquityLookBackOptJSONVisitor.cpp
        EquityLookBackOptMessage.cpp
        EquityChooserOptJSONVisitor.cpp
        EquityChooserOptMessage.cpp
        EquityMargrabeOptJSONVisitor.cpp
        EquityMargrabeOptMessage.cpp
        FuturesBarrierOptJSONVisitor.cpp
        FuturesBarrierOptMessage.cpp
        FuturesAverageOptJSONVisitor.cpp
        FuturesAverageOptMessage.cpp
        ExchangeRateBarrierOptJSONVisitor.cpp
        ExchangeRateBarrierOptMessage.cpp
        ExchangeRateAverageOptJSONVisitor.cpp
        ExchangeRateAverageOptMessage.cpp                      
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${RESTSDK_ROOT}/Binaries/Win32/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(MESSAGES SHARED ${MESSAGES_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (MESSAGES ${GLOG_LIBRARY})
target_link_libraries (MESSAGES debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (MESSAGES ENTITYMGMT)
target_link_libraries (MESSAGES debug ${RESTSDK_DEBUG_LIBRARY} optimized ${RESTSDK_REL_LIBRARY})
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
        EchangeRateOptionSpreadJSONVisitor.cpp              
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
target_link_libraries (MESSAGES ${GFLAG_LIBRARY})
target_link_libraries (MESSAGES ENTITYMGMT)
target_link_libraries (MESSAGES ${RESTSDK_LIBRARY})
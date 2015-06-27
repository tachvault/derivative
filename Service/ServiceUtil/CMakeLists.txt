# add Boost includes
include_directories(${Boost_INCLUDE_DIRS})

# add glog/gflag directories
include_directories("${GLOG_ROOT}/src/windows")
include_directories("${GFLAG_ROOT}/src/windows")

# add Blitz includes
include_directories(${BLITZ_ROOT})

# derivatives include_directories
include_directories("${PROJECT_SOURCE_DIR}/Service/include")

#define DLL EXPORTS for this DLL
add_definitions(-DSERVICEUTIL_EXPORTS)

SET(SERVICE_UTIL_SOURCES
    ${PROJECT_SOURCE_DIR}/Service/include/CSV2Array.hpp
	${PROJECT_SOURCE_DIR}/Service/include/cubature.h
	${PROJECT_SOURCE_DIR}/Service/include/cumnorm1.h
	${PROJECT_SOURCE_DIR}/Service/include/cumnorm2.h
	${PROJECT_SOURCE_DIR}/Service/include/cumnorm3.h
	${PROJECT_SOURCE_DIR}/Service/include/GaussianQuadrature.hpp
	${PROJECT_SOURCE_DIR}/Service/include/GramCharlier.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Linesearch.hpp
	${PROJECT_SOURCE_DIR}/Service/include/LogisticMap.hpp
	${PROJECT_SOURCE_DIR}/Service/include/MSWarnings.hpp
	${PROJECT_SOURCE_DIR}/Service/include/MultivariateNormal.hpp
	${PROJECT_SOURCE_DIR}/Service/include/NewtonRaphson.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Polynomial.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Powell.hpp
	${PROJECT_SOURCE_DIR}/Service/include/QFArrayUtil.hpp
	${PROJECT_SOURCE_DIR}/Service/include/QFQuasiRandom.hpp
	${PROJECT_SOURCE_DIR}/Service/include/QFRandom.hpp
	${PROJECT_SOURCE_DIR}/Service/include/QFUtil.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Regression.hpp
	${PROJECT_SOURCE_DIR}/Service/include/Rootsearch.hpp
	${PROJECT_SOURCE_DIR}/Service/include/StringForm.hpp
	${PROJECT_SOURCE_DIR}/Service/include/ThreadSafeQueue.hpp
	${PROJECT_SOURCE_DIR}/Service/include/utilities.h
	${PROJECT_SOURCE_DIR}/Service/include/SystemUtil.hpp
	${PROJECT_SOURCE_DIR}/Service/include/RootFinder.h
        ${PROJECT_SOURCE_DIR}/Service/include/PseudoRandom.hpp
        ${PROJECT_SOURCE_DIR}/Service/include/PseudoRandomArray.hpp
	CSV2Array.cpp
	cubature.c
	cumnorm1.cpp
	cumnorm2.cpp
	cumnorm3.cpp
	EntityMgrUtil.cpp
	GaussianQuadrature.cpp
	GramCharlier.cpp
	InterfaceCLAPACK.cpp
	InterfaceCubature.cpp
	MultivariateNormal.cpp
	Polynomial.cpp
	QFArrayUtil.cpp
	QFQuasiRandom.cpp
	QFUtil.cpp
	Regression.cpp
	StringForm.cpp
	utilities.cpp
	SystemUtil.cpp
	RootFinder.cpp
)

# Link shared library paths (Note: This should be added before Add_library !!!)
# In VC++, Properties->Linker->General->Additional Library Directories
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${GLOG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${GFLAG_ROOT}/${CMAKE_BUILD_TYPE}")
link_directories("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")

ADD_LIBRARY(SERVICEUTIL SHARED ${SERVICE_UTIL_SOURCES})

# Specify libraries or flags to use when linking a given target.
# In VC++, it is # Properties->Linker->Input->Additional Dependencies
target_link_libraries (SERVICEUTIL ${GLOG_LIBRARY})
target_link_libraries (SERVICEUTIL debug ${GFLAG_DEBUG_LIBRARY} optimized ${GFLAG_REL_LIBRARY})
target_link_libraries (SERVICEUTIL ${CLAPACK_LIBRARY})
target_link_libraries (SERVICEUTIL ${BLAS_LIBRARY})
target_link_libraries (SERVICEUTIL ${LIBF77_LIBRARY})
target_link_libraries (SERVICEUTIL ${LIBI77_LIBRARY})
target_link_libraries (SERVICEUTIL ENTITYMGMT)
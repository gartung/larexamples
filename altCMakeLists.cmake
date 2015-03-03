cmake_minimum_required(VERSION 2.8.12)

if(POLICY CMP0025)
	cmake_policy(SET CMP0025 OLD)
endif()

if(POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif()

project(larexamples)


set(larexamples_VERSION "04.00.01")
set(larexamples_VERSION_MAJOR 04)
set(larexamples_VERSION_MINOR 00)
set(larexamples_VERSION_PATCH 01)

set(larexamples_SOVERSION "1.0.0")


set(larexamples_DEBUG_POSTFIX "d")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules ${CMAKE_MODULE_PATH})

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

set(BASE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/BuildProducts")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BASE_OUTPUT_DIRECTORY}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BASE_OUTPUT_DIRECTORY}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BASE_OUTPUT_DIRECTORY}/${CMAKE_INSTALL_LIBDIR}")

find_package(nutools 1.07.00 REQUIRED)

find_package(art 1.13.1 REQUIRED)

include(artTools)

find_package(FNALCore 0.1.0 REQUIRED)

find_package( larcore 1.0.0 )
find_package( lardata 1.0.0 )
find_package( larevt 1.0.0 )
find_package( larsim 1.0.0 )

find_package( Boost 1.53.0 
     REQUIRED
      serialization
      date_time
     )

find_package(GCCXML 0.9.0 REQUIRED)

find_package(CLHEP 2.2.0.3 REQUIRED)

find_package(SQLite3 3.8.5 REQUIRED)

find_package(ROOT 5.34.20 REQUIRED
     Core
     Cint
     Cintex
     Hist
     Matrix
     Reflex
     RIO
     Thread
     Tree
)

if(NOT ROOT_python_FOUND)
     message(FATAL_ERROR "art requires ROOT with Python support")
endif()

find_package(TBB 4.1.0 REQUIRED)

#find_package( PostgreSQL 9.1.5 )

set(PQ ${PostgreSQL_LIBRARIES} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FNALCore_CXX_FLAGS} -O3 -g -DNDEBUG -fno-omit-frame-pointer -pedantic -Wno-unused-local-typedefs")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})
include_directories(${art_INCLUDE_DIRS})
include_directories(${FNALCore_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${larcore_INCLUDE_DIRS})
include_directories(${lardata_INCLUDE_DIRS})
include_directories(${larevt_INCLUDE_DIRS})
include_directories(${larsim_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${CLHEP_INCLUDE_DIRS})
include_directories(${Geant4_INCLUDE_DIRS})
include_directories( ${nutools_INCLUDE_DIRS} )
include_directories( ${PostgreSQL_INCLUDE_DIRS})
include_directories( ${GEANT4_FQ_DIR}/include )

# source
add_subdirectory(AnalysisExample)

# ups - table and config files
add_subdirectory(ups)

configure_package_config_file(
  Modules/larexamplesConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/larexamplesConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/larexamples-${larexamples_VERSION}
  PATH_VARS
    CMAKE_INSTALL_INCLUDEDIR
    CMAKE_INSTALL_LIBDIR
  )

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/larexamplesConfigVersion.cmake
  VERSION ${larexamples_VERSION}
  COMPATIBILITY AnyNewerVersion
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/larexamplesConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/larexamplesConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/larexamples-${larexamples_VERSION}
  COMPONENT Development
  )

install(EXPORT larexamplesLibraries
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/larexamples-${larexamples_VERSION}
  NAMESPACE larsoft::
  COMPONENT Development
  )



include(ArtCPack)

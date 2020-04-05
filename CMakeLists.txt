# ======================================================================
#  larexamples main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/larexamples/ups/setup_for_development <-d|-p>
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================


# use cmake 3.11 or later
cmake_minimum_required (VERSION 3.11)

project(larexamples VERSION 08.00.00 LANGUAGES CXX)

find_package( cetmodules )
set(fcl_dir job)
include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS CAUTIOUS
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs
)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_package( ROOT REQUIRED COMPONENTS Core Physics Hist MathMore RIO GenVector Geom Minuit Minuit2 XMLIO Gdml TMVA EG)
set(HAVE_ROOT6 BOOL:True)
set(ROOTCLING ROOT::rootcling)
find_package( larsim REQUIRED )

# macros for dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

# source
add_subdirectory(larexamples)
add_subdirectory(test)
add_subdirectory(cmakeConfig)
# ups - table and config files
#add_subdirectory(ups)

# packaging utility
#include(UseCPack)



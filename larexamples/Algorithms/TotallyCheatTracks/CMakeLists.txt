add_subdirectory(CheatTrackData)

art_make(
  LIB_LIBRARIES
    ${ROOT_CORE}
    ${ROOT_PHYSICS}
    lardataobj_RecoBase
  MODULE_LIBRARIES
    ${MF_MESSAGELOGGER}
    ${ROOT_CORE}
    larexamples_Algorithms_TotallyCheatTracks
    nusimdata_SimulationBase
  )

install_headers()
install_fhicl()

# install all sources, plus CMakeLists.txt and all configuration files
file(GLOB TESTFHICLFILES
     LIST_DIRECTORIES false
     *.fcl
     )
install_source(EXTRAS ${TESTFHICLFILES})
unset(TESTFHICLFILES)

install_source()

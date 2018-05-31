add_subdirectory(CheatTrackData)

art_make(
  LIB_LIBRARIES
    lardataobj_RecoBase
    nusimdata_SimulationBase
    cetlib_except
    ${MESSAGE_FACILITY}
    ${FHICLCPP}
    ${ROOT_PHYSICS}
    ${ROOT_CORE}
  MODULE_LIBRARIES
    larexamples_Algorithms_TotallyCheatTracks
    lardataobj_RecoBase
    nusimdata_SimulationBase
    art_Framework_Core
    art_Framework_Principal
    canvas
    ${MF_MESSAGELOGGER}
    ${FHICLCPP}
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


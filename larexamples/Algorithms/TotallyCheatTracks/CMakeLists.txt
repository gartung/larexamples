add_subdirectory(CheatTrackData)

art_make(
  LIB_LIBRARIES
    lardataobj_RecoBase
    nusimdata_SimulationBase
    cetlib_except
    ${MESSAGE_FACILITY}
    ${FHICLCPP}
    ${ROOT_Physics_LIBRARY}
    ${ROOT_Core_LIBRARY}
  MODULE_LIBRARIES
    larexamples_Algorithms_TotallyCheatTracks
    lardataobj_RecoBase
    nusimdata_SimulationBase
    art_Framework_Core
    art_Framework_Principal
    art_Persistency_Provenance
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

art_make(
  LIB_LIBRARIES
    larcorealg_Geometry
    lardataobj_RecoBase
    cetlib cetlib_except
    ${MESSAGE_FACILITY}
    ${FHICLCPP}
    ${ROOT_CORE}
  MODULE_LIBRARIES
    larexamples_Algorithms_RemoveIsolatedSpacePoints
    lardataobj_RecoBase
    larcore_Geometry_Geometry_service
    art_Framework_Core
    art_Framework_Services_Registry
    art_Framework_Principal
    art_Utilities canvas
    ${MF_MESSAGELOGGER}
    ${MF_UTILITY}
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


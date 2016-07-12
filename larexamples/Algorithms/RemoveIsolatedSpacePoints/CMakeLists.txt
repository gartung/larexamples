art_make(
  LIB_LIBRARIES
    larcore_Geometry
    lardata_RecoBase
    ${CETLIB}
    ${MESSAGE_FACILITY}
    ${FHICLCPP}
    ${ROOT_CORE}
  MODULE_LIBRARIES
    larexamples_Algorithms_RemoveIsolatedSpacePoints
    lardata_RecoBase
    larcore_Geometry_Geometry_service
    art_Framework_Core
    art_Framework_Services_Registry
    art_Framework_Principal
    art_Utilities
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
install_source(EXTRAS README CMakeLists.txt ${TESTFHICLFILES})
unset(TESTFHICLFILES)


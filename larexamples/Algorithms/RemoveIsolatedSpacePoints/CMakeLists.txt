art_make(
  LIB_LIBRARIES
    larcore_Geometry
    lardataobj_RecoBase
    ${CETLIB}
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
    art_Utilities canvas_Utilities
    ${MF_MESSAGELOGGER}
    ${MF_UTILITY}
    ${FHICLCPP}
  )


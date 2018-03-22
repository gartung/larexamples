
art_make(
  MODULE_LIBRARIES
    art_Framework_Core
    art_Framework_Principal
    canvas
    ${MF_MESSAGELOGGER}
    ${MF_UTILITY}
    ${FHICLCPP}
  )

install_source()
install_fhicl()

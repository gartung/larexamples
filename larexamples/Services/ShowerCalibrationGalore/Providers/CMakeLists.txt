art_make(
  LIB_LIBRARIES
    lardataobj_RecoBase
    ${FHICLCPP}
    cetlib cetlib_except
    ${ROOT_Hist_LIBRARY}
    ${ROOT_MathMore_LIBRARY}
    ${ROOT_RIO_LIBRARY}
    ${ROOT_Core_LIBRARY}
  )

install_fhicl()
install_headers()

# install all sources, plus CMakeLists.txt and all configuration files
file(GLOB TESTFHICLFILES
     LIST_DIRECTORIES false
     *.fcl
     )
install_source(EXTRAS CMakeLists.txt ${TESTFHICLFILES})
unset(TESTFHICLFILES)


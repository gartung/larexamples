art_make(
  LIB_LIBRARIES
    lardataobj_RecoBase
    ${FHICLCPP}
    cetlib cetlib_except
    ${ROOT_HIST}
    ${ROOT_MATHMORE}
    ${ROOT_RIO}
    ${ROOT_CORE}
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


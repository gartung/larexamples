art_make(
  LIB_LIBRARIES
    ${ROOT_CORE}
    ${ROOT_HIST}
    ${ROOT_MATHMORE}
    ${ROOT_RIO}
    cetlib
    cetlib_except
    lardataobj_RecoBase
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

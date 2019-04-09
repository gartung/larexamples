# this will create both the library and the dictionary
art_make(
  LIB_LIBRARIES
    lardataobj_RecoBase
    ${ROOT_EG_LIBRARY} # TDatabasePDG
  DICT_LIBRARIES
    nusimdata_SimulationBase
  )

install_source()
install_headers()

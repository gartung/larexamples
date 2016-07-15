art_make(
  SERVICE_LIBRARIES
    larexamples_Services_ShowerCalibrationGalore_Providers
    lardataobj_RecoBase
    art_Framework_Services_Registry
    art_Utilities canvas_Utilities
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


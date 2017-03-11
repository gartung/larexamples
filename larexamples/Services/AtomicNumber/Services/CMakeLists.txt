art_make(
  SERVICE_LIBRARIES
    larexamples_Services_AtomicNumber_Providers
    ${ART_FRAMEWORK_SERVICES_REGISTRY}    
    cetlib cetlib_except
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

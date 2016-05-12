art_make(
  SERVICE_LIBRARIES
    larexamples_Services_ShowerCalibrationGalore_Providers
    lardata_RecoBase
    art_Framework_Services_Registry
    art_Utilities
    ${ROOT_CORE}
  )

install_fhicl()
install_headers()
install_source()


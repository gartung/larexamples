
cet_test(CreateTestShowerCalibrationFromPID_test
  SOURCES CreateTestShowerCalibrationFromPID.cxx CreateTestShowerCalibrationFromPID_test.cc
  LIBRARIES
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_CORE}
    ${ROOT_HIST}
  )

cet_test(ShowerCalibrationGaloreFromPID_test
  SOURCES CreateTestShowerCalibrationFromPID.cxx ShowerCalibrationGaloreFromPID_test.cc
  LIBRARIES
    larexamples_Services_ShowerCalibrationGalore_Providers
    larcore_TestUtils
    MF_MessageLogger
    MF_Utilities
    fhiclcpp
    cetlib cetlib_except
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_CORE}
    ${ROOT_HIST}
  )

cet_test(ShowerCalibrationGaloreScale_test
  LIBRARIES
    larexamples_Services_ShowerCalibrationGalore_Providers
    larcore_TestUtils
    MF_MessageLogger
    MF_Utilities
    fhiclcpp
    cetlib cetlib_except
    ${ROOT_BASIC_LIB_LIST}
    ${ROOT_CORE}
    ${ROOT_HIST}
  )


# install all sources, plus CMakeLists.txt and all configuration files
file(GLOB TESTFHICLFILES
     LIST_DIRECTORIES false
     *.fcl
     )
install_source(EXTRAS CMakeLists.txt ${TESTFHICLFILES})
unset(TESTFHICLFILES)

simple_plugin(ShowerCalibrationGaloreTest "module"
  lardata_RecoBase
  art_Framework_Core
  art_Framework_Services_Registry
  art_Utilities
  mf_MessageLogger
  mf_Utilities
  fhiclcpp
  cetlib 
)


cet_test(ShowerCalibrationGaloreScaleTest HANDBUILT
   DATAFILES ./test_shower_calibration_scale.fcl
   TEST_EXEC lar
   TEST_ARGS --rethrow-all --config ./test_shower_calibration_scale.fcl
   )

cet_test(ShowerCalibrationGaloreFromPIDTest HANDBUILT
   DATAFILES ./test_shower_calibration_from_pid.fcl
   TEST_EXEC lar
   TEST_ARGS --rethrow-all --config ./test_shower_calibration_from_pid.fcl
   )

install_source()

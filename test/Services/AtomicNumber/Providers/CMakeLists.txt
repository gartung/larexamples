# simple test with hard-coded configurations
cet_test(AtomicNumber_test
   LIBRARIES
      larexamples_Services_AtomicNumber_Providers
      ${MF_MESSAGELOGGER}
      ${MF_UTILITIES}
      ${FHICLCPP}
      ${CETLIB}
)

# test with external configuration (happens to use Boost)
cet_test(BoostedAtomicNumber_test
   DATAFILES ./test_krypton_atomic_number.fcl
   TEST_ARGS ./test_krypton_atomic_number.fcl
   LIBRARIES
      larexamples_Services_AtomicNumber_Providers
      ${MF_MESSAGELOGGER}
      ${MF_UTILITIES}
      ${FHICLCPP}
      ${CETLIB}
   USE_BOOST_UNIT
   )

# tests just the configuration
cet_test(ValidateAtomicNumberConfiguration_test
   LIBRARIES
      larexamples_Services_AtomicNumber_Providers
      ${MF_MESSAGELOGGER}
      ${MF_UTILITIES}
      ${FHICLCPP}
      ${CETLIB}
   DATAFILES ./test_krypton_atomic_number.fcl
   TEST_ARGS ./test_krypton_atomic_number.fcl
   )
   
   
simple_plugin(AtomicNumberTest "module"
   ${ART_FRAMEWORK_CORE}
   ${ART_FRAMEWORK_CORE}
   ${ART_FRAMEWORK_SERVICES_REGISTRY}
   ${ART_UTILITIES}
   ${MF_MESSAGELOGGER}
   ${MF_UTILITIES}
   ${FHICLCPP}
   ${CETLIB}
)


cet_test(AtomicNumberService_test HANDBUILT
   DATAFILES ./test_krypton_atomic_number.fcl
   TEST_EXEC lar
   TEST_ARGS --rethrow-all --config ./test_krypton_atomic_number.fcl
   )

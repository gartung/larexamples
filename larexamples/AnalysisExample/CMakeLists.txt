
simple_plugin(AnalysisExample "module"
			larcore_Geometry_Geometry_service
                        larcore_Geometry
                        nusimdata_SimulationBase
                        ${ART_FRAMEWORK_CORE}
			${ART_FRAMEWORK_PRINCIPAL}
			${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
 			art_Persistency_Common canvas
			art_Persistency_Provenance canvas
			art_Utilities canvas
			${MF_MESSAGELOGGER}
			${MF_UTILITIES}
			cetlib cetlib_except
			${ROOT_BASIC_LIB_LIST}
              
)

install_headers()
install_fhicl()
install_source( EXTRAS ADDITIONAL_NOTES.txt README.txt )
install_scripts()

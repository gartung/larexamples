simple_plugin(RecoProxyUsageExample "module"
			lardata_RecoBaseProxy
			lardataobj_RecoBase
                        ${ART_FRAMEWORK_CORE}
			${ART_FRAMEWORK_PRINCIPAL}
			${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_Core_LIBRARY}
                        ${ART_ROOT_IO_TFILESERVICE_SERVICE}
 			art_Persistency_Common
			art_Persistency_Provenance
			art_Utilities
                        canvas
			${MF_MESSAGELOGGER}
			
			cetlib cetlib_except
			${ROOT_LIBRARIES}
			${ROOT_GenVector_LIBRARY}
)

install_headers()
install_fhicl()
install_scripts()
install_source()


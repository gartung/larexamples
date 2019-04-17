simple_plugin(RecoProxyUsageExample "module"
			lardata_RecoBaseProxy
			lardataobj_RecoBase
                        ${ART_FRAMEWORK_CORE}
			${ART_FRAMEWORK_PRINCIPAL}
			${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
                        ${ART_ROOT_IO_TFILESERVICE_SERVICE}
 			art_Persistency_Common
			art_Persistency_Provenance
			art_Utilities
                        canvas
			${MF_MESSAGELOGGER}
			
			cetlib cetlib_except
			${ROOT_BASIC_LIB_LIST}
			${ROOT_GENVECTOR}
)

install_headers()
install_fhicl()
install_scripts()
install_source()


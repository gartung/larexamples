include(artTools)

art_add_module(AnalysisExample_module AnalysisExample_module.cc)

install(TARGETS
     AnalysisExample_module
     EXPORT larexamplesLibraries
     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
     COMPONENT Runtime
     )	

install(FILES AnalysisExample.fcl DESTINATION job COMPONENT Runtime)

install(FILES rename.sh DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)


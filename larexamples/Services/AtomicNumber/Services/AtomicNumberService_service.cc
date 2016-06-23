/**
 * @file    AtomicNumberService_service.cc
 * @brief   Returns the atomic number of the active material in the TPC
 * @author  Gianluca Petrillo (petrillo@fnal.gov)
 * @date    April 13, 2016
 * @see     AtomicNumberService.h
 * 
 * Implementation file.
 */


// LArSoft libraries
#include "larexamples/Services/AtomicNumber/Services/AtomicNumberService.h"

#if 0
// -----------------------------------------------------------------------------
lar::example::AtomicNumberService::AtomicNumberService
   (Parameters const& config, art::ActivityRegistry&)
   : prov(std::make_unique<provider_type>(config()))
{}

#endif // 0

// -----------------------------------------------------------------------------
DEFINE_ART_SERVICE(lar::example::AtomicNumberService)

// -----------------------------------------------------------------------------

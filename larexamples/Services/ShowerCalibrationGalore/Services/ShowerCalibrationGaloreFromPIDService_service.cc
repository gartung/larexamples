/**
 * @file   ShowerCalibrationGaloreFromPIDService_service.cc
 * @brief  Shower energy calibration according to particle type      
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   April 28, 2016
 * @see    ShowerCalibrationGaloreFromPID.h
 * 
 * 
 */

// our header
#include "larexamples/Services/ShowerCalibrationGalore/Services/ShowerCalibrationGaloreFromPIDService.h"


DEFINE_ART_SERVICE_INTERFACE_IMPL(
  lar::example::ShowerCalibrationGaloreFromPIDService,
  lar::example::ShowerCalibrationGaloreService
  )

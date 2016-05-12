/**
 * @file   ShowerCalibrationGaloreScaleService.h
 * @brief  A shower calibration service providing a single calibration factor
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   April 28, 2016
 * @see    ShowerCalibrationGaloreScale.h
 * 
 * 
 */


#ifndef LAREXAMPLES_SERVICES_SHOWERCALIBRATIONGALORE_SERVICES_SHOWERCALIBRATIONGALORESCALESERVICE_H
#define LAREXAMPLES_SERVICES_SHOWERCALIBRATIONGALORE_SERVICES_SHOWERCALIBRATIONGALORESCALESERVICE_H 1


/// LArSoft libraries
#include "larexamples/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGaloreScale.h"
#include "larexamples/Services/ShowerCalibrationGalore/Services/ShowerCalibrationGaloreService.h"


namespace lar {
   namespace example {
      
      /**
       * @brief A shower calibration art service returning a single factor
       * 
       * See the ShowerCalibrationGaloreScale provider documentation for
       * configuration instructions and implementation details.
       * 
       * See the ShowerCalibrationGalore provider class documentation for an
       * explanation of the interface.
       * 
       * Configuration parameters
       * -------------------------
       * 
       * In addition to the service provider:
       * 
       * * *service_provider* must be set to
       *   `"ShowerCalibrationGaloreScaleService"` in order to tell _art_ to
       *   load this implementation.
       * 
       */
      using ShowerCalibrationGaloreScaleService
        = lar::ServiceProviderImplementationWrapper
          <ShowerCalibrationGaloreScale, ShowerCalibrationGaloreService>;
         
   } // namespace example
} // namespace lar


DECLARE_ART_SERVICE_INTERFACE_IMPL(
  lar::example::ShowerCalibrationGaloreScaleService,
  lar::example::ShowerCalibrationGaloreService,
  LEGACY
  )


#endif // LAREXAMPLES_SERVICES_SHOWERCALIBRATIONGALORE_SERVICES_SHOWERCALIBRATIONGALORESCALESERVICE_H


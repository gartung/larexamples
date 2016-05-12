/**
 * @file   ShowerCalibrationGaloreFromPID_test.cc
 * @brief  Tests ShowerCalibrationGaloreFromPID service provider
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   April 28, 2016
 * @see    ShowerCalibrationGaloreFromPID.h
 * 
 * Command line arguments:
 * 
 * CreateTestCalibrationFromPID  [OutputPath]
 * 
 * OutputPath is a full ROOT directory path made of a UNIX path and a ROOT
 * directory path. For example, "data/calibrations.root:Showers/ByType" will
 * create a directory "data" and a "calibrations.root" ROOT file in it (or
 * update it if exists), create a structure of two nested ROOT directories,
 * "Showers/ByType", and write all the calibration graphs in there.
 * 
 */


// LArSoft libraries
#include "larexamples/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGaloreFromPID.h"
#include "larexamples/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGalore.h"
#include "larexamples/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGaloreFromPIDTestHelpers.h"
#include "larcore/TestUtils/unit_test_base.h"
#include "test/Services/ShowerCalibrationGalore/Providers/CreateTestShowerCalibrationFromPID.h"
#include "test/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGaloreTests.h"

// C/C++ standard libraries
#include <string>
#include <iostream>


//------------------------------------------------------------------------------
int main() {
   
   //
   // prepare the test environment
   //
   
   // create a test calibration file on the spot
   std::string const CalibrationFullPath = "TestCalibration.root:Showers";
   lar::example::tests::CreateTestShowerCalibrationFromPID
     (CalibrationFullPath);
   
   // provide a test name and a push a configuration for
   // "ShowerCalibrationGaloreService" ("service_provider" is inconsequential)
   testing::BasicEnvironmentConfiguration config
     ("ShowerCalibrationGaloreFromPID_test");
   config.AddDefaultServiceConfiguration
     ("ShowerCalibrationGaloreService", R"(
     service_provider: "ShowerCalibrationGaloreFromPIDService"
     CalibrationFile: ")" + CalibrationFullPath + R"("
     )");
   
   // set up a basic testing environment with that configuration
   auto TesterEnv = testing::CreateTesterEnvironment(config);
   
   // set up a service provider
   // (ShowerCalibrationGaloreFromPID explicitly supports this one-step setup)
   TesterEnv.SimpleProviderSetup<lar::example::ShowerCalibrationGaloreFromPID>();
   
   //
   // computation of expected values
   //
   unsigned int nErrors = 0; // error count
   
   // get the provider we just set up (but accessing it by the interface)ß  
   auto const* Calibration
     = TesterEnv.Provider<lar::example::ShowerCalibrationGalore>();
   
   //
   // run the test
   //
   nErrors += lar::example::tests::ShowerCalibrationTableTest(
     std::cout, Calibration, 0.0, 2.5, 0.1,
     { 11, 13, -11, -13, 211, 111, 2112, 2212, 22 }
   );
   
   std::cout << Calibration->report() << std::endl;

   return nErrors;
} // main()

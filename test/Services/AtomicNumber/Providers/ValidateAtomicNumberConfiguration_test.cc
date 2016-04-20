/**
 * @file   ValidateAtomicNumberConfiguration_test.cc
 * @brief  Validates the AtomicNumber service provider configuration
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   April 18, 2016
 * @see    AtomicNumber.h
 * 
 * This test expects a single configuration file to be specified as first
 * argument.
 * 
 */


// LArSoft libraries
#include "larexamples/Services/AtomicNumber/Providers/AtomicNumber.h"
#include "larcore/TestUtils/unit_test_base.h" // testing namespace

// support libraries
#include "fhiclcpp/types/Name.h"

// C/C++ standard libraries
#include <iostream> // std::cout
#include <string>
#include <stdexcept> // std::runtime_error


//------------------------------------------------------------------------------
int main(int argc, char** argv) {
   
   //
   // configuration of the test
   // 

   // provide a test name;
   // the path to the configuration file will be taken from the first parameter
   testing::BasicEnvironmentConfiguration config
      (argc, argv, "ValidateAtomicNumberConfiguration_test");
   
   // set up a basic testing environment with that configuration
   auto TesterEnv = testing::CreateTesterEnvironment(config);
   
   // create a configuration table; here the name is irrelevant
   lar::example::AtomicNumber::parameters_type providerConfig
      (fhicl::Name("AtomicNumberService"));
   
   // print the configuration
   mf::LogVerbatim("ValidateAtomicNumberConfiguration") << std::string(80, '-')
      << "\nAllowed configuration for AtomicNumber provider:";
   providerConfig.print_allowed_configuration(std::cout);
   
   //
   // test of the configuration
   //
   
   mf::LogVerbatim("ValidateAtomicNumberConfiguration") << std::string(80, '-')
      << "\nValidating configuration from '"
      << config.ServiceParameterSetPath("AtomicNumberService")
      << "' in '" << config.ConfigurationPath() << "':";
   // this will trigger validation
   // and throw fhicl::detail::validationException on error
   providerConfig.validate_ParameterSet
      (TesterEnv.ServiceParameters("AtomicNumberService"), { "service_type" });
   
   //
   // done!
   //
   return 0;
} // main()

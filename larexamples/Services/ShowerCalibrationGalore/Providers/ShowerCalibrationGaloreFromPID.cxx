/**
 * @file   ShowerCalibrationGaloreFromPID.cxx
 * @brief  Shower energy calibration according to particle type
 * @author Gianluca Petrillo (petrillo@fnal.gov)
 * @date   April 28, 2016
 * @see    ShowerCalibrationGaloreFromPID.h
 * 
 * Implementation file. Nothing needed here.
 */


/// LArSoft libraries
#include "larexamples/Services/ShowerCalibrationGalore/Providers/ShowerCalibrationGaloreFromPID.h"


/// framework and utility libraries
#include "cetlib/exception.h"
#include "cetlib/search_path.h"

// ROOT libraries
#include "TSpline.h" // TSpline3
#include "TGraphErrors.h"
#include "TFile.h"
#include "TDirectory.h"


// C/C++ standard libraries
#include <algorithm> // std::lower_bound(), std::sort(), std::min(), std::max()
#include <memory> // std::make_unique()
#include <utility> // std::pair<>
#include <tuple> // std::tie()



//------------------------------------------------------------------------------
float lar::example::ShowerCalibrationGaloreFromPID::correctionFactor
  (recob::Shower const& shower, PDGID_t id /* = unknownID */) const
{
   CalibrationInfo_t const& corr = selectCorrection(id);
   
   double const E = shower.Energy().at(shower.best_plane());
   
   return (float) corr.evalFactor(E);
} // lar::example::ShowerCalibrationGaloreFromPID::correctionFactor()


//------------------------------------------------------------------------------
lar::example::ShowerCalibrationGalore::Correction_t
lar::example::ShowerCalibrationGaloreFromPID::correction
  (recob::Shower const& shower, PDGID_t id /* = unknownID */) const
{
   CalibrationInfo_t const& corr = selectCorrection(id);
   
   double const E = shower.Energy().at(shower.best_plane());
   
   return { (float) corr.evalFactor(E), (float) corr.evalError(E) };
} // lar::example::ShowerCalibrationGaloreFromPID::correction()


//------------------------------------------------------------------------------
void lar::example::ShowerCalibrationGaloreFromPID::readCalibration
  (std::string path)
{
   
   //
   // open the input file
   //
   TDirectory* CalibDir = nullptr;
   try {
      CalibDir = OpenROOTdirectory(path);
   }
   catch (cet::exception const& e) {
      // wrap the exception with context information
      throw cet::exception
        ("ShowerCalibrationGaloreFromPID", "readCalibration()", e)
        << "Reading calibration from: '" << path << "'";
   }
   if (!CalibDir) { // this would be actually a bug
      throw cet::exception
        ("ShowerCalibrationGaloreFromPID", "readCalibration()")
        << "Null directory reading calibration from: '" << path << "'";
   }
   
   // make sure that when this is over we are done with the ROOT file
   std::unique_ptr<TFile> InputFile(CalibDir->GetFile());
   
   //
   // read each calibration object and associate it with its particle category
   //
   
   Calibration_pi0    = readParticleCalibration(CalibDir, "Pi0",    111);
   
   Calibration_photon = readParticleCalibration(CalibDir, "Photon",  22);
   
   Calibration_electron
     = readParticleCalibration(CalibDir, "Electron", { -11, 11 });
   
   Calibration_muon   = readParticleCalibration(CalibDir, "Muon", { -13, 13 });
   
   Calibration_other  = readParticleCalibration(CalibDir, "Default", unknownID);
   
   //
   // release resources
   //
   
   // TFile freed by its unique pointer
   
} // lar::example::ShowerCalibrationGaloreFromPID::readCalibration()


//------------------------------------------------------------------------------
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t const&
lar::example::ShowerCalibrationGaloreFromPID::selectCorrection(PDGID_t id) const
{
   switch (id) {
      case  111: // pi0
         return Calibration_pi0;
      case   22: // photon
         return Calibration_photon;
      case  -11: // electron
      case  +11: // positron
         return Calibration_electron;
      case  -13: // muon
      case  +13: // antimuon
         return Calibration_muon;
      case unknownID:
      default:
         return Calibration_other;
   } // switch
} // lar::example::ShowerCalibrationGaloreFromPID::selectCorrection()


//------------------------------------------------------------------------------
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t
lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration
  (TDirectory* SourceDir, std::string GraphName) const
{
   CalibrationInfo_t info;
   
   // apply list is left empty
   
   //
   // retrieve the object
   //
   auto graph = details::readROOTobject<TGraphErrors>(SourceDir, GraphName);
   
   verifyOrder(graph.get());
   
   size_t const N = (size_t) graph->GetN();
   if (N == 0) {
      throw cet::exception("ShowerCalibrationGaloreFromPID")
        << "No point in graph " << SourceDir->GetPath() << "/" << GraphName
        << "\n";
   }
   
   // include the "error" on x in the full range
   info.minE = graph->GetX()[0];
   info.maxE = graph->GetX()[N - 1];
   
   // a cubic spline with initial and final derivatives ("b1" and "e1")
   // set to zero
   info.factor = std::make_unique<TSpline3>
     (("Calib_" + GraphName).c_str(), graph.get(), "b1 e1", 0., 0.);
   
   // compute the error in the same way; kind of an approximation here
   info.error = std::make_unique<TSpline3>(
     ("CalibError_" + GraphName).c_str(), graph->GetX(), graph->GetEY(), N,
     "b1 e1", 0., 0.
     );
   
   return info;
} // lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration()


//------------------------------------------------------------------------------
void lar::example::ShowerCalibrationGaloreFromPID::verifyOrder
   (TGraph const* graph)
{
   
   if (!graph) {
      throw cet::exception("ShowerCalibrationGaloreFromPID")
         << "VerifyOrder(): invalid graph specified\n";
   }
   
   size_t const N = graph->GetN();
   if (N < 2) return;
   
   Double_t const* x = graph->GetX();
   
   for (size_t i = 1; i < N; ++i) {
      if (x[i-1] > x[i]) {
         throw cet::exception("ShowerCalibrationGaloreFromPID")
         << "VerifyOrder(): points in graph '" << graph->GetName()
         << "' are not sorted in abscissa\n";
      }
   } // while
} // lar::example::ShowerCalibrationGaloreFromPID::verifyOrder()

   
//------------------------------------------------------------------------------
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t
lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration
  (TDirectory* SourceDir, std::string GraphName, PDGID_t id) const
{ 
   CalibrationInfo_t info = readParticleCalibration(SourceDir, GraphName);
   info.applyTo(id);
   return info;
} // lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration(ID)


lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t
lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration(
  TDirectory* SourceDir, std::string GraphName,
  std::initializer_list<PDGID_t> ids
) const {
   CalibrationInfo_t info = readParticleCalibration(SourceDir, GraphName);
   info.applyTo(ids);
   return info;
} // lar::example::ShowerCalibrationGaloreFromPID::readParticleCalibration(IDs)


//------------------------------------------------------------------------------
TDirectory* lar::example::ShowerCalibrationGaloreFromPID::OpenROOTdirectory
  (std::string path)
{
   //
   // split the data file path
   //
   std::string filePath, ROOTdirPath;
   std::tie(filePath, ROOTdirPath) = splitROOTpath(path);
   
   //
   // find the ROOT file in the search path
   //
   std::string fullFilePath;
   cet::search_path sp("FW_SEARCH_PATH");
   // if we can't find the file in FW_SEARCH_PATH, we try in current directory
   if (!sp.find_file(filePath, fullFilePath)) fullFilePath = filePath;
   
   //
   // open the ROOT file (created new)
   //
   auto inputFile = std::make_unique<TFile>(fullFilePath.c_str(), "READ");
   if (!(inputFile->IsOpen())) {
      throw cet::exception("ShowerCalibrationGaloreFromPID")
         << "ShowerCalibrationGaloreFromPID::OpenROOTdirectory() can't read '"
         << fullFilePath << "' (from '" << filePath << "' specification)\n";
   }
   
   //
   // get the ROOT directory
   //
   TDirectory* dir = ROOTdirPath.empty()?
     inputFile.get(): inputFile->GetDirectory(ROOTdirPath.c_str());
   if (!dir) {
      throw cet::exception("ShowerCalibrationGaloreFromPID")
         << "ShowerCalibrationGaloreFromPID::OpenROOTdirectory() can't find '"
         << ROOTdirPath << "' in ROOT file '" << inputFile->GetPath() << "'\n";
   }
   
   //
   // return the directory
   //
   inputFile.release(); // do not delete the file any more
   return dir;
} // lar::example::ShowerCalibrationGaloreFromPID::OpenROOTdirectory()


//------------------------------------------------------------------------------
// lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t
//
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t&
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t::applyTo
   (PDGID_t id)
{
   auto it = std::lower_bound(appliesTo.begin(), appliesTo.end(), id);
   if ((it == appliesTo.end()) || (*it != id))
      appliesTo.insert(it, id);
   return *this;
} // ShowerCalibrationGaloreFromPID::CalibrationInfo_t::applyTo(ID)


lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t&
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t::applyTo
   (std::initializer_list<PDGID_t> ids)
{
   appliesTo.insert(appliesTo.begin(), ids.begin(), ids.end());
   std::sort(appliesTo.begin(), appliesTo.end());
   return *this;
} // ShowerCalibrationGaloreFromPID::CalibrationInfo_t::applyTo(IDs)


//--- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ---
double
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t::evalFactor
   (double E) const
{
   double const boundE = std::min(maxE, std::max(minE, E));
   return factor->Eval(boundE);
}

double
lar::example::ShowerCalibrationGaloreFromPID::CalibrationInfo_t::evalError
   (double E) const
{
   double const boundE = std::min(maxE, std::max(minE, E));
   return error->Eval(boundE);
}


//------------------------------------------------------------------------------

std::pair<std::string, std::string>
lar::example::splitROOTpath(std::string path) {
   const std::string suffix = ".root";
   
   // find the ROOT file name
   std::string::size_type iSuffix = std::string::npos;
   do {
      iSuffix = path.rfind(suffix, iSuffix);
      if (iSuffix == std::string::npos) return {}; // failure: no suffix
      
      // if it's not "suffix:" or "suffix/" or at end of string, it's not it
      auto iAfter = iSuffix + suffix.length();
      if ((iAfter < path.length())
         && (path[iAfter] != '/')
         && (path[iAfter] != ':')
         )
      {
         // oops... this suffix is invalid; keep searching
         if (iSuffix == 0) return {}; // failure: no suffix
         --iSuffix;
         continue;
      }
      
      // we found a proper suffix
      std::pair<std::string, std::string> result;
      
      result.first = path.substr(0U, iAfter); // file path
      if (iAfter < path.length())
         result.second = path.substr(iAfter + 1, path.length()); // ROOT dir
      return result;
      
   } while (true);
} // lar::example::splitROOTpath()


//------------------------------------------------------------------------------

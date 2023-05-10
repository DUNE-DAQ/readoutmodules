/**
 * @file ndReadoutCreator.cpp ReadoutCreator implementation for Far Detector
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include <logging/Logging.hpp>
#include <nlohmann/json.hpp>
#include <cetlib/BasicPluginFactory.h>

#include "readoutmodules/ReadoutCreator.hpp"
#include "readoutmodules/ReadoutModulesIssues.hpp"

#include "readoutlibs/ReadoutIssues.hpp"
#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/concepts/ReadoutConcept.hpp"
#include "readoutlibs/models/ReadoutModel.hpp"
#include "readoutlibs/models/SkipListLatencyBufferModel.hpp"

#include "ndreadoutlibs/NDReadoutPACMANTypeAdapter.hpp"
#include "ndreadoutlibs/NDReadoutMPDTypeAdapter.hpp"
#include "ndreadoutlibs/pacman/PACMANFrameProcessor.hpp"
#include "ndreadoutlibs/pacman/PACMANListRequestHandler.hpp"
#include "ndreadoutlibs/mpd/MPDFrameProcessor.hpp"
#include "ndreadoutlibs/mpd/MPDListRequestHandler.hpp"

#include <memory>
#include <string>

using json = nlohmann::json;

namespace dunedaq {

DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::NDReadoutPACMANTypeAdapter, "PACMANFrame")
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::NDReadoutMPDTypeAdapter, "MPDFrame")

namespace readoutmodules {

class ndReadoutCreator : public ReadoutCreator
{
public:
  explicit ndReadoutCreator(std::string spec) : ReadoutCreator(spec) 
  { } 

  std::unique_ptr<readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker)
  {
    namespace rol = dunedaq::readoutlibs;
    namespace ndl = dunedaq::ndreadoutlibs;
    namespace ndt = dunedaq::ndreadoutlibs::types;
  
    // Acquire input connection and its DataType
    auto ci = appfwk::connection_index(args, {"raw_input"});
    auto datatypes = dunedaq::iomanager::IOManager::get()->get_datatypes(ci["raw_input"]);
    if (datatypes.size() != 1) {
      ers::error(dunedaq::readoutlibs::GenericConfigurationError(ERS_HERE,
        "Multiple raw_input queues specified! Expected only a single instance!"));
    }
    std::string raw_dt{ *datatypes.begin() };
    TLOG() << "Choosing specializations for ReadoutModel with raw_input"
           << " [uid:" << ci["raw_input"] << " , data_type:" << raw_dt << ']';
  
    // IF ND LAr PACMAN
    if (raw_dt.find("PACMANFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pacman";
      auto readout_model =
        std::make_unique<rol::ReadoutModel<ndt::NDReadoutPACMANTypeAdapter,
                                           ndl::PACMANListRequestHandler,
                                           rol::SkipListLatencyBufferModel<ndt::NDReadoutPACMANTypeAdapter>,
                                           ndl::PACMANFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF ND LAr MPD
    if (raw_dt.find("MPDFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a mpd";
      auto readout_model =
      std::make_unique<rol::ReadoutModel<ndt::NDReadoutMPDTypeAdapter,
                                         ndl::MPDListRequestHandler,
                                         rol::SkipListLatencyBufferModel<ndt::NDReadoutMPDTypeAdapter>,
                                         ndl::MPDFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }

    return nullptr;
  }

};

} // namespace readoutmodules
} // namespace dunedaq

extern "C" {
    std::shared_ptr<dunedaq::readoutmodules::ReadoutCreator> make(std::string spec) {
        return std::shared_ptr<dunedaq::readoutmodules::ReadoutCreator>(new dunedaq::readoutmodules::ndReadoutCreator(spec));
    }
}

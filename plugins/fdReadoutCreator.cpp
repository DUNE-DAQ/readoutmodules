/**
 * @file fdReadoutCreator.cpp ReadoutCreator implementation for Far Detector
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
#include "readoutlibs/models/BinarySearchQueueModel.hpp"
#include "readoutlibs/models/DefaultRequestHandlerModel.hpp"
#include "readoutlibs/models/EmptyFragmentRequestHandlerModel.hpp"
#include "readoutlibs/models/FixedRateQueueModel.hpp"
#include "readoutlibs/models/ReadoutModel.hpp"
#include "readoutlibs/models/ZeroCopyRecordingRequestHandlerModel.hpp"
#include "readoutlibs/models/DefaultSkipListRequestHandler.hpp"
#include "readoutlibs/models/SkipListLatencyBufferModel.hpp"

#include "fdreadoutlibs/ProtoWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBEthTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/SSPFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TDEFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TriggerPrimitiveTypeAdapter.hpp"
#include "fdreadoutlibs/DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter.hpp"

#include "fdreadoutlibs/daphne/DAPHNEFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEStreamFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEListRequestHandler.hpp"
#include "fdreadoutlibs/ssp/SSPFrameProcessor.hpp"
#include "fdreadoutlibs/wib2/RAWWIBTriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib/SWWIBTriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib/WIBFrameProcessor.hpp"
#include "fdreadoutlibs/wib2/WIB2FrameProcessor.hpp"
#include "fdreadoutlibs/wibeth/WIBEthFrameProcessor.hpp"
#include "fdreadoutlibs/tde/TDEFrameProcessor.hpp"


#include <memory>
#include <string>

using json = nlohmann::json;

namespace dunedaq {

DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter, "WIBFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIB2Frame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter, "WIBEthFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNEStreamSuperChunkTypeAdapter, "PDSStreamFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::SSPFrameTypeAdapter, "SSPFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEFrameTypeAdapter, "TDEFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TriggerPrimitiveTypeAdapter, "TriggerPrimitive")

namespace readoutmodules {

class fdReadoutCreator : public ReadoutCreator
{
public:
  explicit fdReadoutCreator(std::string spec) : ReadoutCreator(spec) 
  { } 

  std::unique_ptr<readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker)
  {
    namespace rol = dunedaq::readoutlibs;
    namespace fdl = dunedaq::fdreadoutlibs;
    namespace fdt = dunedaq::fdreadoutlibs::types;
  
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
  
    // Chose readout specializations based on DataType
    // IF WIB
    if (raw_dt.find("WIBFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for ProtoDUNE-WIB";
      auto readout_model = std::make_unique<rol::ReadoutModel<
        fdt::ProtoWIBSuperChunkTypeAdapter,
        rol::ZeroCopyRecordingRequestHandlerModel<fdt::ProtoWIBSuperChunkTypeAdapter,
                                                  rol::FixedRateQueueModel<fdt::ProtoWIBSuperChunkTypeAdapter>>,
        rol::FixedRateQueueModel<fdt::ProtoWIBSuperChunkTypeAdapter>,
        fdl::WIBFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF WIB2
    if (raw_dt.find("WIB2Frame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a DUNE-WIB";
      auto readout_model = std::make_unique<
        rol::ReadoutModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                          rol::DefaultRequestHandlerModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                                                          rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>>,
                          rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>,
                          fdl::WIB2FrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF WIBEth
    if (raw_dt.find("WIBEthFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for an Ethernet DUNE-WIB";
      auto readout_model = std::make_unique<
        rol::ReadoutModel<fdt::DUNEWIBEthTypeAdapter,
                          rol::DefaultRequestHandlerModel<fdt::DUNEWIBEthTypeAdapter,
                                                          rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>>,
                          rol::FixedRateQueueModel<fdt::DUNEWIBEthTypeAdapter>,
                          fdl::WIBEthFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }

    // IF PDS Frame using skiplist
    if (raw_dt.find("PDSFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE using SkipList LB";
      auto readout_model =
        std::make_unique<rol::ReadoutModel<fdt::DAPHNESuperChunkTypeAdapter,
                                           fdl::DAPHNEListRequestHandler,
                                           rol::SkipListLatencyBufferModel<fdt::DAPHNESuperChunkTypeAdapter>,
                                           fdl::DAPHNEFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF PDS Stream Frame using SPSC LB
    if (raw_dt.find("PDSStreamFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a PDS DAPHNE stream mode using BinarySearchQueue LB";
      auto readout_model = std::make_unique<
        rol::ReadoutModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                          rol::DefaultRequestHandlerModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                                                          rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>>,
                          rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>,
                          fdl::DAPHNEStreamFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF SSP
    if (raw_dt.find("SSPFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a SSPs using BinarySearchQueue LB";
      auto readout_model = std::make_unique<rol::ReadoutModel<
        fdt::SSPFrameTypeAdapter,
        rol::DefaultRequestHandlerModel<fdt::SSPFrameTypeAdapter, rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>>,
        rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>,
        fdl::SSPFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // If TDE
    if (raw_dt.find("TDEFrame") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for TDE";
      auto readout_model = std::make_unique<
        rol::ReadoutModel<fdt::TDEFrameTypeAdapter,
                          rol::DefaultSkipListRequestHandler<fdt::TDEFrameTypeAdapter>,
                          rol::SkipListLatencyBufferModel<fdt::TDEFrameTypeAdapter>,
                          fdl::TDEFrameProcessor>>(run_marker);
      readout_model->init(args);
      return readout_model;
    }
  
    // IF TriggerPrimitive (SW-TPG)
    if (raw_dt.find("TriggerPrimitive") != std::string::npos) {
      TLOG(TLVL_WORK_STEPS) << "Creating readout for TriggerPrimitive";
      auto readout_model = std::make_unique<rol::ReadoutModel<
        fdt::TriggerPrimitiveTypeAdapter,
        rol::DefaultSkipListRequestHandler<fdt::TriggerPrimitiveTypeAdapter>,
        rol::SkipListLatencyBufferModel<fdt::TriggerPrimitiveTypeAdapter>,
        fdl::SWWIBTriggerPrimitiveProcessor>>(run_marker);
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
        return std::shared_ptr<dunedaq::readoutmodules::ReadoutCreator>(new dunedaq::readoutmodules::fdReadoutCreator(spec));
    }
}

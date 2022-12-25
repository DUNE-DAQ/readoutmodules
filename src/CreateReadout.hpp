/**
 * @file CreateRedout.hpp Specific readout creator
 * Thanks for Brett and Phil for the idea
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_SRC_CREATEREADOUT_HPP_
#define READOUTMODULES_SRC_CREATEREADOUT_HPP_

#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"

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
#include "fdreadoutlibs/DAPHNESuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/DAPHNEStreamSuperChunkTypeAdapter.hpp"
#include "fdreadoutlibs/SSPFrameTypeAdapter.hpp"
#include "fdreadoutlibs/TDEAMCFrameTypeAdapter.hpp"
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
#include "fdreadoutlibs/tde/TDEFrameProcessor.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"
#include "ndreadoutlibs/pacman/PACMANFrameProcessor.hpp"
#include "ndreadoutlibs/pacman/PACMANListRequestHandler.hpp"
#include "ndreadoutlibs/mpd/MPDFrameProcessor.hpp"
#include "ndreadoutlibs/mpd/MPDListRequestHandler.hpp"

#include <memory>
#include <string>
#include <utility>

using dunedaq::readoutlibs::logging::TLVL_WORK_STEPS;

namespace dunedaq {

DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::ProtoWIBSuperChunkTypeAdapter, "WIBFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DUNEWIBSuperChunkTypeAdapter, "WIBFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::DAPHNESuperChunkTypeAdapter, "PDSFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TDEAMCFrameTypeAdapter, "TDEData")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::TriggerPrimitiveTypeAdapter, "TriggerPrimitive")
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT, "PACMAN")
DUNE_DAQ_TYPESTRING(dunedaq::ndreadoutlibs::types::MPD_MESSAGE_STRUCT, "MPD")

namespace readoutmodules {

std::unique_ptr<readoutlibs::ReadoutConcept>
createReadout(const nlohmann::json& args, std::atomic<bool>& run_marker)
{

  namespace rol = dunedaq::readoutlibs;
  namespace fdl = dunedaq::fdreadoutlibs;
  namespace fdt = dunedaq::fdreadoutlibs::types;
  namespace ndt = dunedaq::ndreadoutlibs::types;

  auto queues = args.get<appfwk::app::ModInit>().conn_refs;
  for (const auto& qi : queues) {
    if (qi.name == "raw_input") {
      auto& inst = qi.uid;

      // IF WIB
      if (inst.find("wib") != std::string::npos && inst.find("wib2") == std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib";
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
      if (inst.find("wib2") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib2";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                            rol::DefaultRequestHandlerModel<fdt::DUNEWIBSuperChunkTypeAdapter,
                                                            rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>>,
                            rol::FixedRateQueueModel<fdt::DUNEWIBSuperChunkTypeAdapter>,
                            fdl::WIB2FrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF DAPHNE queue
      if (inst.find("pds_queue") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using Searchable Queue";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::DAPHNESuperChunkTypeAdapter,
                            rol::DefaultRequestHandlerModel<fdt::DAPHNESuperChunkTypeAdapter,
                                                            rol::BinarySearchQueueModel<fdt::DAPHNESuperChunkTypeAdapter>>,
                            rol::BinarySearchQueueModel<fdt::DAPHNESuperChunkTypeAdapter>,
                            fdl::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF PDS skiplist
      if (inst.find("pds_list") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using SkipList LB";
        auto readout_model =
          std::make_unique<rol::ReadoutModel<fdt::DAPHNESuperChunkTypeAdapter,
                                             fdl::DAPHNEListRequestHandler,
                                             rol::SkipListLatencyBufferModel<fdt::DAPHNESuperChunkTypeAdapter>,
                                             fdl::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF PDS Stream skiplist
      if (inst.find("pds_stream") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds stream mode using BinarySearchQueue";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                            rol::DefaultRequestHandlerModel<fdt::DAPHNEStreamSuperChunkTypeAdapter,
                                                            rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>>,
                            rol::BinarySearchQueueModel<fdt::DAPHNEStreamSuperChunkTypeAdapter>,
                            fdl::DAPHNEStreamFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      if (inst.find("sw_tp") != std::string::npos) {
        TLOG(TLVL_WORK_STEPS) << "Creating readout for sw tp";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::TriggerPrimitiveTypeAdapter,
          rol::DefaultSkipListRequestHandler<fdt::TriggerPrimitiveTypeAdapter>,
          rol::SkipListLatencyBufferModel<fdt::TriggerPrimitiveTypeAdapter>,
          fdl::SWWIBTriggerPrimitiveProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF SSP
      if (inst.find("ssp") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a SSPs using Searchable Queue";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::SSPFrameTypeAdapter,
          rol::DefaultRequestHandlerModel<fdt::SSPFrameTypeAdapter, rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>>,
          rol::BinarySearchQueueModel<fdt::SSPFrameTypeAdapter>,
          fdl::SSPFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      if (inst.find("raw_tp") != std::string::npos) {
        TLOG(TLVL_WORK_STEPS) << "Creating readout for raw tp";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter,
          rol::DefaultSkipListRequestHandler<fdt::DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter>,
          rol::SkipListLatencyBufferModel<fdt::DUNEWIBFirmwareTriggerPrimitiveSuperChunkTypeAdapter>,
          fdl::RAWWIBTriggerPrimitiveProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF ND LAr PACMAN
      if (inst.find("pacman") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pacman";
        auto readout_model =
          std::make_unique<rol::ReadoutModel<ndt::PACMAN_MESSAGE_STRUCT,
                                             ndreadoutlibs::PACMANListRequestHandler,
                                             rol::SkipListLatencyBufferModel<ndt::PACMAN_MESSAGE_STRUCT>,
                                             ndreadoutlibs::PACMANFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF ND LAr MPD
      if (inst.find("mpd") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a mpd";
        auto readout_model =
          std::make_unique<rol::ReadoutModel<ndt::MPD_MESSAGE_STRUCT,
                                             ndreadoutlibs::MPDListRequestHandler,
                                             rol::SkipListLatencyBufferModel<ndt::MPD_MESSAGE_STRUCT>,
                                             ndreadoutlibs::MPDFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // If TDE
      if (inst.find("tde") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for TDE";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::TDEAMCFrameTypeAdapter
,
                            rol::DefaultRequestHandlerModel<fdt::TDEAMCFrameTypeAdapter
,
                                                            rol::FixedRateQueueModel<fdt::TDEAMCFrameTypeAdapter
>>,
                            rol::FixedRateQueueModel<fdt::TDEAMCFrameTypeAdapter
>,
                            fdl::TDEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF variadic
      if (inst.find("varsize") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a variable size FE";
      }
    }
  }

  return nullptr;
}

} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_SRC_CREATEREADOUT_HPP_

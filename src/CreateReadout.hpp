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

#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "fdreadoutlibs/daphne/DAPHNEFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEListRequestHandler.hpp"
#include "fdreadoutlibs/ssp/SSPFrameProcessor.hpp"
#include "fdreadoutlibs/wib/RAWWIBTriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib/SWWIBTriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib/WIBFrameProcessor.hpp"
#include "fdreadoutlibs/wib2/WIB2FrameProcessor.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"
#include "ndreadoutlibs/pacman/PACMANFrameProcessor.hpp"
#include "ndreadoutlibs/pacman/PACMANListRequestHandler.hpp"

#include <memory>
#include <string>
#include <utility>

using dunedaq::readoutlibs::logging::TLVL_WORK_STEPS;

namespace dunedaq {
namespace readoutmodules {

std::unique_ptr<readoutlibs::ReadoutConcept>
createReadout(const nlohmann::json& args, std::atomic<bool>& run_marker)
{

  namespace rol = dunedaq::readoutlibs;
  namespace fdl = dunedaq::fdreadoutlibs;
  namespace fdt = dunedaq::fdreadoutlibs::types;
  namespace ndt = dunedaq::ndreadoutlibs::types;

  auto queues = args.get<appfwk::app::ModInit>().qinfos;
  for (const auto& qi : queues) {
    if (qi.name == "raw_input") {
      auto& inst = qi.inst;

      // IF WIB
      if (inst.find("wib") != std::string::npos && inst.find("wib2") == std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::WIB_SUPERCHUNK_STRUCT,
          rol::ZeroCopyRecordingRequestHandlerModel<fdt::WIB_SUPERCHUNK_STRUCT,
                                                    rol::FixedRateQueueModel<fdt::WIB_SUPERCHUNK_STRUCT>>,
          rol::FixedRateQueueModel<fdt::WIB_SUPERCHUNK_STRUCT>,
          fdl::WIBFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF WIB2
      if (inst.find("wib2") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib2";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::WIB2_SUPERCHUNK_STRUCT,
                            rol::DefaultRequestHandlerModel<fdt::WIB2_SUPERCHUNK_STRUCT,
                                                            rol::FixedRateQueueModel<fdt::WIB2_SUPERCHUNK_STRUCT>>,
                            rol::FixedRateQueueModel<fdt::WIB2_SUPERCHUNK_STRUCT>,
                            fdl::WIB2FrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF DAPHNE queue
      if (inst.find("pds_queue") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using Searchable Queue";
        auto readout_model = std::make_unique<
          rol::ReadoutModel<fdt::DAPHNE_SUPERCHUNK_STRUCT,
                            rol::DefaultRequestHandlerModel<fdt::DAPHNE_SUPERCHUNK_STRUCT,
                                                            rol::BinarySearchQueueModel<fdt::DAPHNE_SUPERCHUNK_STRUCT>>,
                            rol::BinarySearchQueueModel<fdt::DAPHNE_SUPERCHUNK_STRUCT>,
                            fdl::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF PDS skiplist
      if (inst.find("pds_list") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using SkipList LB";
        auto readout_model =
          std::make_unique<rol::ReadoutModel<fdt::DAPHNE_SUPERCHUNK_STRUCT,
                                             fdl::DAPHNEListRequestHandler,
                                             rol::SkipListLatencyBufferModel<fdt::DAPHNE_SUPERCHUNK_STRUCT>,
                                             fdl::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      if (inst.find("sw_tp") != std::string::npos) {
        TLOG(TLVL_WORK_STEPS) << "Creating readout for sw tp";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::SW_WIB_TRIGGERPRIMITIVE_STRUCT,
          rol::EmptyFragmentRequestHandlerModel<fdt::SW_WIB_TRIGGERPRIMITIVE_STRUCT,
                                                rol::BinarySearchQueueModel<fdt::SW_WIB_TRIGGERPRIMITIVE_STRUCT>>,
          rol::BinarySearchQueueModel<fdt::SW_WIB_TRIGGERPRIMITIVE_STRUCT>,
          fdl::SWWIBTriggerPrimitiveProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF SSP
      if (inst.find("ssp") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a SSPs using Searchable Queue";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::SSP_FRAME_STRUCT,
          rol::DefaultRequestHandlerModel<fdt::SSP_FRAME_STRUCT, rol::BinarySearchQueueModel<fdt::SSP_FRAME_STRUCT>>,
          rol::BinarySearchQueueModel<fdt::SSP_FRAME_STRUCT>,
          fdl::SSPFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      if (inst.find("raw_tp") != std::string::npos) {
        TLOG(TLVL_WORK_STEPS) << "Creating readout for raw tp";
        auto readout_model = std::make_unique<rol::ReadoutModel<
          fdt::RAW_WIB_TRIGGERPRIMITIVE_STRUCT,
          rol::DefaultRequestHandlerModel<fdt::RAW_WIB_TRIGGERPRIMITIVE_STRUCT,
                                          rol::BinarySearchQueueModel<fdt::RAW_WIB_TRIGGERPRIMITIVE_STRUCT>>,
          rol::BinarySearchQueueModel<fdt::RAW_WIB_TRIGGERPRIMITIVE_STRUCT>,
          fdl::RAWWIBTriggerPrimitiveProcessor>>(run_marker);
        readout_model->init(args);
        return std::move(readout_model);
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

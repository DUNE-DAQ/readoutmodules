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

#include "readout/ReadoutLogging.hpp"
#include "readout/ReadoutIssues.hpp"
#include "readout/concepts/ReadoutConcept.hpp"
#include "readout/models/ReadoutModel.hpp"
#include "readout/models/BinarySearchQueueModel.hpp"
#include "readout/models/DefaultRequestHandlerModel.hpp"
#include "readout/models/FixedRateQueueModel.hpp"
#include "readout/models/EmptyFragmentRequestHandlerModel.hpp"

#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "fdreadoutlibs/wib/WIBFrameProcessor.hpp"
#include "fdreadoutlibs/wib/WIBTriggerPrimitiveProcessor.hpp"
#include "fdreadoutlibs/wib2/WIB2FrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEFrameProcessor.hpp"
#include "fdreadoutlibs/daphne/DAPHNEListRequestHandler.hpp"
#include "fdreadoutlibs/ssp/SSPFrameProcessor.hpp"

#include <memory>
#include <string>
#include <utility>

using dunedaq::readout::logging::TLVL_WORK_STEPS;

namespace dunedaq {
namespace readoutmodules {

std::unique_ptr<readout::ReadoutConcept>
createReadout(const nlohmann::json& args, std::atomic<bool>& run_marker)
{
  auto queues = args.get<appfwk::app::ModInit>().qinfos;
  for (const auto& qi : queues) {
    if (qi.name == "raw_input") {
      auto& inst = qi.inst;

      // IF WIB
      if (inst.find("wib") != std::string::npos && inst.find("wib2") == std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib";
        auto readout_model = std::make_unique<readout::ReadoutModel<
          fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT,
          readout::DefaultRequestHandlerModel<fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT, readout::FixedRateQueueModel<fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT>>,
          readout::FixedRateQueueModel<fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT>,
          fdreadoutlibs::WIBFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF WIB2
      if (inst.find("wib2") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a wib2";
        auto readout_model = std::make_unique<readout::ReadoutModel<
          fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT,
          readout::DefaultRequestHandlerModel<fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT, readout::FixedRateQueueModel<fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT>>,
          readout::FixedRateQueueModel<fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT>,
          fdreadoutlibs::WIB2FrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF DAPHNE queue
      if (inst.find("pds_queue") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using Searchable Queue";
        auto readout_model = std::make_unique<
          readout::ReadoutModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT,
                       readout::DefaultRequestHandlerModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT,
                                                  readout::BinarySearchQueueModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT>>,
                       readout::BinarySearchQueueModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT>,
                       fdreadoutlibs::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF PDS skiplist
      if (inst.find("pds_list") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a pds using SkipList LB";
        auto readout_model = std::make_unique<readout::ReadoutModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT,
                                                           fdreadoutlibs::DAPHNEListRequestHandler,
                                                           readout::SkipListLatencyBufferModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT>,
                                                           fdreadoutlibs::DAPHNEFrameProcessor>>(run_marker);
        readout_model->init(args);
        return readout_model;
      }

      // IF SSP
      if (inst.find("ssp") != std::string::npos) {
        TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating readout for a SSPs using Searchable Queue";
        auto readout_model = std::make_unique<readout::ReadoutModel<
          fdreadoutlibs::types::SSP_FRAME_STRUCT,
          readout::DefaultRequestHandlerModel<fdreadoutlibs::types::SSP_FRAME_STRUCT, readout::BinarySearchQueueModel<fdreadoutlibs::types::SSP_FRAME_STRUCT>>,
          readout::BinarySearchQueueModel<fdreadoutlibs::types::SSP_FRAME_STRUCT>,
          fdreadoutlibs::SSPFrameProcessor>>(run_marker);
        readout_model->init(args);
        return std::move(readout_model);
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

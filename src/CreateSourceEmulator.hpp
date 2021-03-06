/**
 * @file CreateSourceEmulator.hpp Specific source emulator creator.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_
#define READOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_

#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/cmd/Structs.hpp"

#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/models/SourceEmulatorModel.hpp"
#include "fdreadoutlibs/tde/TDECrateSourceEmulatorModel.hpp"

#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "fdreadoutlibs/wib/TPEmulatorModel.hpp"

#include <memory>
#include <string>
#include <utility>

using dunedaq::readoutlibs::logging::TLVL_WORK_STEPS;

namespace dunedaq {
namespace readoutmodules {

std::unique_ptr<readoutlibs::SourceEmulatorConcept>
createSourceEmulator(const iomanager::connection::ConnectionRef qi, std::atomic<bool>& run_marker)
{
  //! Values suitable to emulation

  static constexpr int daphne_time_tick_diff = 16;
  static constexpr double daphne_dropout_rate = 0.9;
  static constexpr double daphne_rate_khz = 200.0;

  static constexpr int wib_time_tick_diff = 25;
  static constexpr double wib_dropout_rate = 0.0;
  static constexpr double wib_rate_khz = 166.0;

  static constexpr int wib2_time_tick_diff = 32;
  static constexpr double wib2_dropout_rate = 0.0;
  static constexpr double wib2_rate_khz = 166.0;

  static constexpr int tde_time_tick_diff = 1000;
  static constexpr double tde_dropout_rate = 0.0;
  static constexpr double tde_rate_khz = 0.43668;

  static constexpr double emu_frame_error_rate = 0.0;

  auto& inst = qi.uid;

  // IF WIB2
  if (inst.find("wib2") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wib2 link";

    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT>>(
        qi.name, run_marker, wib2_time_tick_diff, wib2_dropout_rate, emu_frame_error_rate, wib2_rate_khz);
    return source_emu_model;
  }

  // IF WIB
  if (inst.find("wib") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake wib link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT>>(
        qi.name, run_marker, wib_time_tick_diff, wib_dropout_rate, emu_frame_error_rate, wib_rate_khz);
    return source_emu_model;
  }

  // IF PDS
  if (inst.find("pds") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake pds link";
    auto source_emu_model =
      std::make_unique<readoutlibs::SourceEmulatorModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT>>(
        qi.name, run_marker, daphne_time_tick_diff, daphne_dropout_rate, emu_frame_error_rate, daphne_rate_khz);
    return source_emu_model;
  }

  // TP link
  if (inst.find("tp") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake tp link";
    auto source_emu_model = std::make_unique<fdreadoutlibs::TPEmulatorModel>(run_marker, 66.0);
    return source_emu_model;
  }

  // IF TDE
  if (inst.find("tde") != std::string::npos) {
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating fake tde link";
    auto source_emu_model =
      std::make_unique<fdreadoutlibs::TDECrateSourceEmulatorModel<fdreadoutlibs::types::TDE_AMC_STRUCT>>(
        qi.name, run_marker, tde_time_tick_diff, tde_dropout_rate, emu_frame_error_rate, tde_rate_khz);
    return source_emu_model;
  }

  return nullptr;
}

} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_SRC_CREATESOURCEEMULATOR_HPP_

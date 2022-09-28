/**
 * @file DataRecorder.cpp DataRecorder implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
//#include "readout/NDReadoutTypes.hpp"
#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "ndreadoutlibs/NDReadoutTypes.hpp"
#include "readoutlibs/ReadoutLogging.hpp"
#include "readoutlibs/models/RecorderModel.hpp"
#include "readoutlibs/recorderconfig/Nljs.hpp"
#include "readoutlibs/recorderconfig/Structs.hpp"
#include "readoutlibs/recorderinfo/InfoNljs.hpp"

#include "DataRecorder.hpp"
#include "appfwk/DAQModuleHelper.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"
#include "readoutlibs/ReadoutIssues.hpp"
#include <string>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace readoutmodules {

DataRecorder::DataRecorder(const std::string& name)
  : DAQModule(name)
{
  register_command("conf", &DataRecorder::do_conf);
  register_command("scrap", &DataRecorder::do_scrap);
  register_command("start", &DataRecorder::do_start);
  register_command("stop_trigger_sources", &DataRecorder::do_stop);
}

void
DataRecorder::init(const data_t& args)
{
  try {
    auto qi = appfwk::connection_index(args, { "raw_recording" });
    auto inst = qi["raw_recording"].uid;

    // IF WIB2
    if (inst.find("wib2") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for wib2";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::WIB2_SUPERCHUNK_STRUCT>(get_name()));
      recorder->init(args);
      return;
    }

    // IF WIB
    if (inst.find("wib") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for wib";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::WIB_SUPERCHUNK_STRUCT>(get_name()));
      recorder->init(args);
      return;
    }

    // IF PDS
    if (inst.find("pds") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for pds";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::DAPHNE_SUPERCHUNK_STRUCT>(get_name()));
      recorder->init(args);
      return;
    }

    // IF PACMAN
    if (inst.find("pacman") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for pacman";
      recorder.reset(new readoutlibs::RecorderModel<ndreadoutlibs::types::PACMAN_MESSAGE_STRUCT>(get_name()));
      recorder->init(args);
      return;
    }

    // IF MPD
    if (inst.find("mpd") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for mpd";
      recorder.reset(new readoutlibs::RecorderModel<ndreadoutlibs::types::MPD_MESSAGE_STRUCT>(get_name()));
      recorder->init(args);
      return;
    }

    // IF TDE
    if (inst.find("tde") != std::string::npos) {
      TLOG_DEBUG(TLVL_WORK_STEPS) << "Creating recorder for tde";
      recorder.reset(new readoutlibs::RecorderModel<fdreadoutlibs::types::TDE_AMC_STRUCT >(get_name()));
      recorder->init(args);
      return;
    }

    throw readoutlibs::DataRecorderConfigurationError(ERS_HERE, "Could not create DataRecorder of type " + inst);

  } catch (const ers::Issue& excpt) {
    throw readoutlibs::DataRecorderResourceQueueError(ERS_HERE, "Could not initialize queue", "raw_recording", "");
  }
}

void
DataRecorder::get_info(opmonlib::InfoCollector& ci, int level)
{
  recorder->get_info(ci, level);
}

void
DataRecorder::do_conf(const data_t& args)
{
  recorder->do_conf(args);
}

void
DataRecorder::do_scrap(const data_t& args)
{
  recorder->do_scrap(args);
}

void
DataRecorder::do_start(const data_t& args)
{
  recorder->do_start(args);
}

void
DataRecorder::do_stop(const data_t& args)
{
  recorder->do_stop(args);
}

} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::DataRecorder)

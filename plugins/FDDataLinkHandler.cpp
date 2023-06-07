/**
 * @file FDDataLinkHandler.cpp FDDataLinkHandler class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "readoutlibs/ReadoutLogging.hpp"

#include "FDDataLinkHandler.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"
#include "rcif/cmd/Nljs.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace readoutmodules {

FDDataLinkHandler::FDDataLinkHandler(const std::string& name)
  : DAQModule(name)
  , DataLinkHandlerBase(name)
{ 
  //inherited_dlh::m_readout_creator = make_readout_creator("fd");

  inherited_mod::register_command("conf", &inherited_dlh::do_conf);
  inherited_mod::register_command("scrap", &inherited_dlh::do_scrap);
  inherited_mod::register_command("start", &inherited_dlh::do_start);
  inherited_mod::register_command("stop_trigger_sources", &inherited_dlh::do_stop);
  inherited_mod::register_command("record", &inherited_dlh::do_record);
}

void
FDDataLinkHandler::init(const data_t& args)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  inherited_dlh::init(args);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
FDDataLinkHandler::get_info(opmonlib::InfoCollector& ci, int level)
{
  inherited_dlh::get_info(ci, level);
}

std::unique_ptr<readoutlibs::ReadoutConcept>
FDDataLinkHandlercreate_readout(const nlohmann::json& args, std::atomic<bool>& run_marker)
{
  return nullptr;
}

/*
void
FDDataLinkHandler::do_conf(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_conf() method";
  inherited_dlh::do_conf(args);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_conf() method";
}

void
FDDataLinkHandler::do_scrap(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_scrap() method";
  inherited_dlh::do_scrap(args);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_scrap() method";
}
void
FDDataLinkHandler::do_start(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";
  inherited_dlh::do_start(args);
  rcif::cmd::StartParams start_params = args.get<rcif::cmd::StartParams>();
  m_run_number = start_params.run;
  TLOG() << get_name() << " successfully started for run number " << m_run_number;

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
FDDataLinkHandler::do_stop(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";
  inherited_dlh::stop(args);
  TLOG() << get_name() << " successfully stopped for run number " << m_run_number;
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
FDDataLinkHandler::do_record(const data_t& args)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_issue_recording() method";
  inherited_dlh::record(args);
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_issue_recording() method";
}
*/

} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::FDDataLinkHandler)

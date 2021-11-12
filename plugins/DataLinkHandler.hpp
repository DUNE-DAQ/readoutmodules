/**
 * @file DataLinkHandler.hpp Generic readout module
 * that creates a DataLinkHandlerConcept implementation.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_PLUGINS_DATALINKHANDLER_HPP_
#define READOUTMODULES_PLUGINS_DATALINKHANDLER_HPP_

#include "appfwk/DAQModule.hpp"

#include "readout/concepts/ReadoutConcept.hpp"
#include "readoutmodules/datalinkhandler/Structs.hpp"

#include "CreateReadout.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace readoutmodules {

class DataLinkHandler : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief DataLinkHandler Constructor
   * @param name Instance name for this DataLinkHandler instance
   */
  explicit DataLinkHandler(const std::string& name);

  DataLinkHandler(const DataLinkHandler&) = delete;            ///< DataLinkHandler is not copy-constructible
  DataLinkHandler& operator=(const DataLinkHandler&) = delete; ///< DataLinkHandler is not copy-assignable
  DataLinkHandler(DataLinkHandler&&) = delete;                 ///< DataLinkHandler is not move-constructible
  DataLinkHandler& operator=(DataLinkHandler&&) = delete;      ///< DataLinkHandler is not move-assignable

  void init(const data_t& args) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  // Commands
  void do_conf(const data_t& /*args*/);
  void do_scrap(const data_t& /*args*/);
  void do_start(const data_t& /*args*/);
  void do_stop(const data_t& /*args*/);
  void do_record(const data_t& /*args*/);

  // Configuration
  bool m_configured;
  using module_conf_t = datalinkhandler::Conf;
  daqdataformats::run_number_t m_run_number;

  // Internal
  std::unique_ptr<readout::ReadoutConcept> m_readout_impl;

  // Threading
  std::atomic<bool> m_run_marker;
};

} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_PLUGINS_DATALINKHANDLER_HPP_

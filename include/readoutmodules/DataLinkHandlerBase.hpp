/**
 * @file DataLinkHandlerBase.hpp Implements standard 
 * module functionalities, requires the setup of a readout
 * creator function. This class is meant to be inherited
 * to specify which readout specialization to load.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_INCLUDE_READOUTMODULES_DATALINKHANDLERBASE_HPP_
#define READOUTMODULES_INCLUDE_READOUTMODULES_DATALINKHANDLERBASE_HPP_

#include "readoutmodules/ReadoutModulesIssues.hpp"
#include "readoutlibs/concepts/ReadoutConcept.hpp"
#include "readoutlibs/ReadoutLogging.hpp"
#include "daqdataformats/Types.hpp"
#include "logging/Logging.hpp"

#include "nlohmann/json.hpp"
#include "rcif/cmd/Nljs.hpp"

#include "appfwk/ModuleConfiguration.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace readoutmodules {

class DataLinkHandlerBase
{
public:
  /**
   * @brief DataLinkHandlerBase Constructor
   * @param name Instance name for this DataLinkHandlerBase instance
   */
  explicit DataLinkHandlerBase(const std::string& name);
  virtual ~DataLinkHandlerBase() {}

  DataLinkHandlerBase(const DataLinkHandlerBase&) = delete;            ///< DataLinkHandlerBase is not copy-constructible
  DataLinkHandlerBase& operator=(const DataLinkHandlerBase&) = delete; ///< DataLinkHandlerBase is not copy-assignable
  DataLinkHandlerBase(DataLinkHandlerBase&&) = delete;                 ///< DataLinkHandlerBase is not move-constructible
  DataLinkHandlerBase& operator=(DataLinkHandlerBase&&) = delete;      ///< DataLinkHandlerBase is not move-assignable

  void init(std::shared_ptr<ModuleConfiguration> cfg);
  void get_info(opmonlib::InfoCollector& ci, int level);

  virtual std::unique_ptr<dunedaq::readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker) = 0;

  // Commands
  void do_conf(const nlohmann::json& /*args*/);
  void do_scrap(const nlohmann::json& /*args*/);
  void do_start(const nlohmann::json& /*args*/);
  void do_stop(const nlohmann::json& /*args*/);
  void do_record(const nlohmann::json& /*args*/);

  std::string get_dlh_name() { return m_name; }

private:

  // Configuration
  bool m_configured;
  daqdataformats::run_number_t m_run_number;

  // Name
  std::string m_name;

  // Internal
  std::unique_ptr<readoutlibs::ReadoutConcept> m_readout_impl;

  // Threading
  std::atomic<bool> m_run_marker;
};

} // namespace readoutmodules
} // namespace dunedaq

// Declarations
#include "detail/DataLinkHandlerBase.hxx"

#endif // READOUTMODULES_INCLUDE_READOUTMODULES_DATALINKHANDLERBASE_HPP_

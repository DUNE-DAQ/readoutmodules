/**
 * @file NDDataLinkHandler.hpp FarDetector Generic readout
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_PLUGINS_NDDATALINKHANDLER_HPP_
#define READOUTMODULES_PLUGINS_NDDATALINKHANDLER_HPP_

#include "appfwk/DAQModule.hpp"

#include "readoutmodules/DataLinkHandlerBase.hpp"

#include <string>

namespace dunedaq {
namespace readoutmodules {

class NDDataLinkHandler : public dunedaq::appfwk::DAQModule,
                          private dunedaq::readoutmodules::DataLinkHandlerBase
{
public:
  using inherited_dlh = dunedaq::readoutmodules::DataLinkHandlerBase;
  using inherited_mod = dunedaq::appfwk::DAQModule;
  /**
   * @brief NDDataLinkHandler Constructor
   * @param name Instance name for this NDDataLinkHandler instance
   */
  explicit NDDataLinkHandler(const std::string& name);

  NDDataLinkHandler(const NDDataLinkHandler&) = delete;            ///< NDDataLinkHandler is not copy-constructible
  NDDataLinkHandler& operator=(const NDDataLinkHandler&) = delete; ///< NDDataLinkHandler is not copy-assignable
  NDDataLinkHandler(NDDataLinkHandler&&) = delete;                 ///< NDDataLinkHandler is not move-constructible
  NDDataLinkHandler& operator=(NDDataLinkHandler&&) = delete;      ///< NDDataLinkHandler is not move-assignable

  void init(const data_t& args) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

  std::unique_ptr<readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker) override;

};

} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_PLUGINS_NDDATALINKHANDLER_HPP_

/**
 * @file FragmentSender.hpp Module to send fragments using the NetworkManager
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef READOUTMODULES_PLUGINS_FRAGMENTSENDER_HPP_
#define READOUTMODULES_PLUGINS_FRAGMENTSENDER_HPP_

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "utilities/WorkerThread.hpp"
#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "readoutlibs/concepts/RecorderConcept.hpp"
#include "readoutlibs/utils/ReusableThread.hpp"

#include <atomic>
#include <string>

namespace dunedaq {
namespace readoutmodules {

class FragmentSender : public dunedaq::appfwk::DAQModule
{
public:
  explicit FragmentSender(const std::string& name);

  FragmentSender(const FragmentSender&) = delete;
  FragmentSender& operator=(const FragmentSender&) = delete;
  FragmentSender(FragmentSender&&) = delete;
  FragmentSender& operator=(FragmentSender&&) = delete;

  void init(const nlohmann::json& obj) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  // Commands
  void do_conf(const nlohmann::json& obj);
  void do_scrap(const nlohmann::json& obj);
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);

  void do_work();

  // Queue
  using source_t = dunedaq::appfwk::DAQSource<std::pair<std::unique_ptr<daqdataformats::Fragment>, std::string>>;
  std::unique_ptr<source_t> m_input_queue;

  // Threading
  readoutlibs::ReusableThread m_work_thread;
  std::atomic<bool> m_run_marker;

  // Stats
  std::atomic<int> m_packets_processed{ 0 };

};
} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_PLUGINS_FRAGMENTSENDER_HPP_

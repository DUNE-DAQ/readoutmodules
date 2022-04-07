/**
 * @file CPUPinner.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef READOUTMODULES_PLUGINS_CPUPINNER_HPP_
#define READOUTMODULES_PLUGINS_CPUPINNER_HPP_

#include "appfwk/DAQModule.hpp"

#include "utilities/WorkerThread.hpp"

#include "readoutmodules/cpupinner/Nljs.hpp"

namespace dunedaq {
namespace readoutmodules {
class CPUPinner : public dunedaq::appfwk::DAQModule
{
public:
  explicit CPUPinner(const std::string& name);

  CPUPinner(const CPUPinner&) = delete;
  CPUPinner& operator=(const CPUPinner&) = delete;
  CPUPinner(CPUPinner&&) = delete;
  CPUPinner& operator=(CPUPinner&&) = delete;

  void init(const nlohmann::json& iniobj) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  void do_conf(const nlohmann::json& config);
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);
  void do_scrap(const nlohmann::json& obj);
  void do_work(std::atomic<bool>&);

  // pin the thread with thread ID `tid` to the set of CPUs in `cpus`
  void pin_thread(int tid, std::vector<int> cpus) const;

  // Get the thread ID for the thread in the current process that has
  // name `name`. Returns -1 if no such thread found
  int tid_for_name(std::string name) const;

  dunedaq::utilities::WorkerThread m_thread;
  cpupinner::Conf m_conf;
};
} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_PLUGINS_CPUPINNER_HPP_

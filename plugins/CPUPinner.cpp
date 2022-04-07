/**
 * @file CPUPinner.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2021.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CPUPinner.hpp"

#include "logging/Logging.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>

namespace dunedaq {
namespace readoutmodules {
CPUPinner::CPUPinner(const std::string& name)
  : DAQModule(name)
  , m_thread(std::bind(&CPUPinner::do_work, this, std::placeholders::_1))
{

  register_command("conf", &CPUPinner::do_conf);
  register_command("start", &CPUPinner::do_start);
  register_command("stop", &CPUPinner::do_stop);
  register_command("scrap", &CPUPinner::do_scrap);
}

void
CPUPinner::init(const nlohmann::json&)
{}

void
CPUPinner::get_info(opmonlib::InfoCollector& /* ci */, int /*level*/)
{}

void
CPUPinner::do_conf(const nlohmann::json& conf)
{
  m_conf = conf.get<cpupinner::Conf>();
  TLOG_DEBUG(2) << get_name() + " configured.";
}

void
CPUPinner::do_start(const nlohmann::json& /* args */)
{
  m_thread.start_working_thread("pinner");
  TLOG_DEBUG(2) << get_name() + " successfully started.";
}

void
CPUPinner::do_stop(const nlohmann::json&)
{
  m_thread.stop_working_thread();
  TLOG_DEBUG(2) << get_name() + " successfully stopped.";
}

void
CPUPinner::do_scrap(const nlohmann::json&)
{}

void
CPUPinner::do_work(std::atomic<bool>& running_flag)
{
  size_t n_threads_to_pin = m_conf.thread_confs.size();
  size_t n_threads_pinned = 0;
  while (running_flag.load()) {
    // Loop over the requested threads to pin. If we find a thread
    // with the given name, pin it and remove it from the list so we don't try it again next time
    for (auto it = m_conf.thread_confs.begin(); it != m_conf.thread_confs.end(); ) {
      int tid = tid_for_name(it->name);
      if (tid >= 0) {
        std::stringstream ss;
        for(auto cpu: it->cpu_set) {
          ss << cpu << ", ";
        }
        TLOG_DEBUG(1) << "Pinning thread name " << it->name << ", tid " << tid << " to CPUs " << ss.str();
        pin_thread(tid, it->cpu_set);
        it = m_conf.thread_confs.erase(it);
        ++n_threads_pinned;
      } else {
        ++it;
      }
    }
    // No more threads to pin, so no need to keep looping
    if (m_conf.thread_confs.empty()) {
      TLOG() << "No more threads to pin. Exiting loop";
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
  } // while (running_flag.load())

  TLOG() << get_name() << ": do_work() done. Pinned " << n_threads_pinned << " out of " << n_threads_to_pin << " requested";
}

// pin the thread with thread ID `tid` to the set of CPUs in `cpus`
void
CPUPinner::pin_thread(int tid, std::vector<int> cpus) const
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  for (int cpu : cpus) {
    CPU_SET(cpu, &cpuset);
  }
  sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
}

// Get the thread ID for the thread in the current process that has
// name `name`. Returns -1 if no such thread found
int
CPUPinner::tid_for_name(std::string name) const
{
  // /proc/self/task contains a directory for every thread in the
  // current process (including the main thread). The directory name
  // is the thread ID. The thread name is stored in the `comm` file
  // in the thread's directory
  for (auto const& dir_entry : std::filesystem::directory_iterator{ "/proc/self/task" }) {
    std::filesystem::path name_file = dir_entry.path() / "comm";
    std::ifstream fin(name_file);
    std::string thread_name;
    fin >> thread_name;
    if (thread_name == name) {
      return std::stoi(dir_entry.path().filename());
    }
  }
  return -1;
}

} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::CPUPinner)

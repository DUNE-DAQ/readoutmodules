/**
 * @file FragmentSender.cpp FragmentSender implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#include "readoutlibs/ReadoutLogging.hpp"

#include "daqdataformats/Fragment.hpp"

#include "FragmentSender.hpp"
#include "appfwk/DAQModuleHelper.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "logging/Logging.hpp"
#include "readoutlibs/ReadoutIssues.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "networkmanager/NetworkManager.hpp"
#include "dfmessages/Fragment_serialization.hpp"

#include <string>

using namespace dunedaq::readoutlibs::logging;

namespace dunedaq {
namespace readoutmodules {

FragmentSender::FragmentSender(const std::string& name)
  : DAQModule(name),
    m_work_thread(0)
{
  register_command("conf", &FragmentSender::do_conf);
  register_command("scrap", &FragmentSender::do_scrap);
  register_command("start", &FragmentSender::do_start);
  register_command("stop", &FragmentSender::do_stop);
}

void
FragmentSender::do_conf(const data_t& /*args*/)
{
}

void
FragmentSender::do_scrap(const data_t& /*args*/)
{
}

void
FragmentSender::init(const data_t& args)
{
  try {
    auto qi = appfwk::queue_index(args, { "input_queue" });
    m_input_queue.reset(new source_t(qi["input_queue"].inst));
  } catch (const ers::Issue& excpt) {
    throw readoutlibs::GenericResourceQueueError(ERS_HERE, "input_queue", get_name(), excpt);
  }
}

void
FragmentSender::get_info(opmonlib::InfoCollector& /*ci*/, int /* level */)
{
  //dummyconsumerinfo::Info info;
  //info.packets_processed = m_packets_processed;

  //ci.add(info);
}

void
FragmentSender::do_start(const data_t& /* args */)
{
  m_packets_processed = 0;
  m_run_marker.store(true);
  m_work_thread.set_work(&FragmentSender::do_work, this);
}

void
FragmentSender::do_stop(const data_t& /* args */)
{
  m_run_marker.store(false);
  while (!m_work_thread.get_readiness()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void
FragmentSender::do_work()
{
  std::pair<std::unique_ptr<daqdataformats::Fragment>, std::string> element;
  while (m_run_marker) {
    try {
      m_input_queue->pop(element, std::chrono::milliseconds(100));
      auto serialised_frag =
          dunedaq::serialization::serialize(std::move(element.first), dunedaq::serialization::kMsgPack);
      networkmanager::NetworkManager::get().send_to(element.second,
              static_cast<const void*>(serialised_frag.data()), serialised_frag.size(), std::chrono::milliseconds(1000)); 
      m_packets_processed++;
    } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
      continue;
    } catch (ers::Issue& e) {
      
    }
  }
}


} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::FragmentSender)

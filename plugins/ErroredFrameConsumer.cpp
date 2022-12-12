/**
 * @file ErroredFrameConsumer.cpp Module that consumes frames with errors from a queue
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licencing/copyright details are in the COPYING file that you should have
 * recieved with this code.
 * */

#include "DummyConsumer.cpp"
#include "DummyConsumer.hpp"
#include "detdataformats/wib/WIBFrame.hpp"
#include "logging/Logging.hpp"
#include "readoutlibs/ReadoutLogging.hpp"

#include <bitset>

#include <memory>
#include <string>

namespace dunedaq {

    DUNE_DAQ_TYPESTRING(detdataformats::wib::WIBFrame, "WIBFrame")

namespace readoutmodules {

class ErroredFrameConsumer : public DummyConsumer<detdataformats::wib::WIBFrame>
{
public:
  explicit ErroredFrameConsumer(const std::string name)
    : DummyConsumer<detdataformats::wib::WIBFrame>(name)
  {}

  void packet_callback(detdataformats::wib::WIBFrame& packet) override
  {
    if (packet.get_wib_header()->wib_errors) {
      m_error_count += std::bitset<16>(packet.get_wib_header()->wib_errors).count();
      //      TLOG() << "bitset: " << std::bitset<16>(packet.get_wib_header()->wib_errors)
      //          << " error count: " << m_error_count;
    }
  }

private:
  int m_error_count;
};
} // namespace readoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::ErroredFrameConsumer)

/**
 * @file FakeCardReaderBase.hpp Generate payloads from input file
 * Generates user payloads at a given rate, from raw binary data files.
 * This implementation is purely software based, no I/O devices and tools
 * are needed to use this module.
 *
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_INCLUDE_READOUTMODULES_FAKECARDREADERBASE_HPP_
#define READOUTMODULES_INCLUDE_READOUTMODULES_FAKECARDREADERBASE_HPP_

// package
#include "readoutlibs/concepts/SourceEmulatorConcept.hpp"
#include "readoutlibs/sourceemulatorconfig/Structs.hpp"
#include "readoutlibs/sourceemulatorconfig/Nljs.hpp"
#include "readoutlibs/utils/ReusableThread.hpp"
#include "readoutlibs/utils/FileSourceBuffer.hpp"

#include "nlohmann/json.hpp"
#include "rcif/cmd/Nljs.hpp"

// std
#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace readoutmodules {

class FakeCardReaderBase
{
public:
  /**
   * @brief FakeCardReaderBase Constructor
   * @param name Instance name for this FakeCardReaderBase instance
   */
  explicit FakeCardReaderBase(const std::string& name);
  virtual ~FakeCardReaderBase() {}

  FakeCardReaderBase(const FakeCardReaderBase&) = delete;            ///< FakeCardReaderBase is not copy-constructible
  FakeCardReaderBase& operator=(const FakeCardReaderBase&) = delete; ///< FakeCardReaderBase is not copy-assignable
  FakeCardReaderBase(FakeCardReaderBase&&) = delete;                 ///< FakeCardReaderBase is not move-constructible
  FakeCardReaderBase& operator=(FakeCardReaderBase&&) = delete;      ///< FakeCardReaderBase is not move-assignable

  void init(const nlohmann::json&);
  void get_info(opmonlib::InfoCollector& ci, int level);

  // To be implemented by final module
  virtual std::unique_ptr<readoutlibs::SourceEmulatorConcept>
  create_source_emulator(const appfwk::app::ConnectionReference qi, std::atomic<bool>& run_marker) = 0;

  // Commands
  void do_conf(const nlohmann::json& /*args*/);
  void do_scrap(const nlohmann::json& /*args*/);
  void do_start(const nlohmann::json& /*args*/);
  void do_stop(const nlohmann::json& /*args*/);

  std::string get_fcr_name() { return m_name; }

private:
  // Configuration
  bool m_configured;
  std::string m_name;
  using module_conf_t = readoutlibs::sourceemulatorconfig::Conf;
  module_conf_t m_cfg;

  std::map<std::string, std::unique_ptr<readoutlibs::SourceEmulatorConcept>> m_source_emus;

  // Internals
  std::unique_ptr<readoutlibs::FileSourceBuffer> m_source_buffer;

  // Threading
  std::atomic<bool> m_run_marker;
};

} // namespace readoutmodules
} // namespace dunedaq

// Declarations
#include "detail/FakeCardReaderBase.hxx"

#endif // READOUTMODULES_INCLUDE_READOUTMODULES_FAKECARDREADERBASE_HPP_

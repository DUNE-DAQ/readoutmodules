/**
 * @file DataRecorder.hpp Module to record data
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef READOUTMODULES_PLUGINS_DATARECORDER_HPP_
#define READOUTMODULES_PLUGINS_DATARECORDER_HPP_

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "utilities/WorkerThread.hpp"
#include "fdreadoutlibs/FDReadoutTypes.hpp"
#include "readoutlibs/concepts/RecorderConcept.hpp"
#include "readoutlibs/recorderconfig/Structs.hpp"
#include "readoutlibs/utils/BufferedFileWriter.hpp"
#include "readoutlibs/utils/ReusableThread.hpp"

#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace dunedaq {
namespace readoutmodules {

class DataRecorder : public dunedaq::appfwk::DAQModule
{
public:
  explicit DataRecorder(const std::string& name);

  DataRecorder(const DataRecorder&) = delete;
  DataRecorder& operator=(const DataRecorder&) = delete;
  DataRecorder(DataRecorder&&) = delete;
  DataRecorder& operator=(DataRecorder&&) = delete;

  void init(const nlohmann::json& obj) override;
  void get_info(opmonlib::InfoCollector& ci, int level) override;

private:
  // Commands
  void do_conf(const nlohmann::json& obj);
  void do_scrap(const nlohmann::json& obj);
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);

  std::unique_ptr<readoutlibs::RecorderConcept> recorder;
};
} // namespace readoutmodules
} // namespace dunedaq

#endif // READOUTMODULES_PLUGINS_DATARECORDER_HPP_

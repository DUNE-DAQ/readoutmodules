/**
 * @file ReadoutCreator.hpp ReadoutCreator base definitions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_INCLUDE_READOUTMODULES_READOUTCREATOR_HPP_
#define READOUTMODULES_INCLUDE_READOUTMODULES_READOUTCREATOR_HPP_

#include "readoutlibs/concepts/ReadoutConcept.hpp"

#include "ReadoutModulesIssues.hpp"

#include <cetlib/BasicPluginFactory.h>
#include <cetlib/compiler_macros.h>

#include <atomic>
#include <memory>
#include <string>

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                                                                    \
  extern "C"                                                                                                           \
  {
#endif

/**
 * @brief Declare the function that will be called by the plugin loader
 * @param klass Class to be defined as a DUNE Readout Creator
 */
#define DEFINE_DUNE_READOUT_CREATOR(klass)                                                                            \
  EXTERN_C_FUNC_DECLARE_START                                                                                          \
  std::unique_ptr<dunedaq::readoutmodules::ReadoutCreator> make()                                                             \
  {                                                                                                                    \
    return std::unique_ptr<dunedaq::readoutmodules::ReadoutCreator>(new klass());                                             \
  }                                                                                                                    \
  } 

namespace dunedaq::readoutmodules {

/**
 * @brief Readout Creator
 */
class ReadoutCreator
{
public:
  explicit ReadoutCreator(std::string /*spec*/) {}
  virtual ~ReadoutCreator();
  ReadoutCreator(const ReadoutCreator&) = 
    delete; ///< ReadoutCreator is not copy-constructible
  ReadoutCreator& operator=(const ReadoutCreator&) =
    delete; ///< ReadoutCreator is not copy-assignable
  ReadoutCreator(ReadoutCreator&&) =
    delete; ///< ReadoutCreator is not move-constructible
  ReadoutCreator& operator=(ReadoutCreator&&) =
    delete; ///< ReadoutCreator is not move-assignable

  //! Readout creation function. Implementation specific.
  virtual std::unique_ptr<dunedaq::readoutlibs::ReadoutConcept>
  create_readout(const nlohmann::json& args, std::atomic<bool>& run_marker) = 0;

};

std::shared_ptr<ReadoutCreator>
make_command_facility(std::string const& spec)
{
  std::string plugin_name = spec + "ReadoutCreator";
  static cet::BasicPluginFactory bpf("duneReadoutCreator", "make");
  std::shared_ptr<ReadoutCreator> rc_ptr;
  try {
    rc_ptr = bpf.makePlugin<std::shared_ptr<ReadoutCreator>>(plugin_name, spec);
  } catch (const cet::exception &cexpt) {
    throw ReadoutCreatorCreationFailed(ERS_HERE, spec, cexpt);
  } catch (const ers::Issue &iexpt) {
    throw ReadoutCreatorCreationFailed(ERS_HERE, spec, iexpt);
  } catch (...) {  // NOLINT JCF Jan-27-2021 violates letter of the law but not the spirit
    throw ReadoutCreatorCreationFailed(ERS_HERE, spec, "Unknown error.");
  }
  return rc_ptr;
}

} // namespace dunedaq::readoutmodules

#endif // READOUTMODULES_INCLUDE_READOUTMODULES_READOUTCREATOR_HPP_

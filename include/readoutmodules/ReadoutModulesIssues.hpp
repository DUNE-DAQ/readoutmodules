/**
 * @file ReadoutIssues.hpp Readout system related ERS issues
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUTMODULES_INCLUDE_READOUT_READOUTISSUES_HPP_
#define READOUTMODULES_INCLUDE_READOUT_READOUTISSUES_HPP_

#include "daqdataformats/SourceID.hpp"

#include <ers/Issue.hpp>

#include <string>

namespace dunedaq {
ERS_DECLARE_ISSUE(readoutmodules,
                  InternalError,
                  "SourceID[" << sid << "] Internal Error: " << error,
                  ((daqdataformats::SourceID)sid)((std::string)error))

ERS_DECLARE_ISSUE(readoutmodules,
                  CommandError,
                  "SourceID[" << sid << "] Command Error: " << commanderror,
                  ((daqdataformats::SourceID)sid)((std::string)commanderror))

ERS_DECLARE_ISSUE(readoutmodules,
                  InitializationError,
                  "Readout Initialization Error: " << initerror,
                  ((std::string)initerror))

ERS_DECLARE_ISSUE(readoutmodules,
                  ConfigurationError,
                  "SourceID[" << sid << "] Readout Configuration Error: " << conferror,
                  ((daqdataformats::SourceID)sid)((std::string)conferror))

ERS_DECLARE_ISSUE(readoutmodules,
                  BufferedReaderWriterConfigurationError,
                  "Configuration Error: " << conferror,
                  ((std::string)conferror))

ERS_DECLARE_ISSUE(readoutmodules,
                  DataRecorderConfigurationError,
                  "Configuration Error: " << conferror,
                  ((std::string)conferror))

ERS_DECLARE_ISSUE(readoutmodules,
                  GenericConfigurationError,
                  "Configuration Error: " << conferror,
                  ((std::string)conferror))

ERS_DECLARE_ISSUE(readoutmodules, CannotOpenFile, "Couldn't open binary file: " << filename, ((std::string)filename))

ERS_DECLARE_ISSUE(readoutmodules,
                  BufferedReaderWriterCannotOpenFile,
                  "Couldn't open file: " << filename,
                  ((std::string)filename))

ERS_DECLARE_ISSUE_BASE(readoutmodules,
                       CannotReadFile,
                       readoutmodules::ConfigurationError,
                       " Couldn't read properly the binary file: " << filename << " Cause: " << errorstr,
                       ((daqdataformats::SourceID)sid)((std::string)filename),
                       ((std::string)errorstr))

ERS_DECLARE_ISSUE(readoutmodules, CannotWriteToFile, "Could not write to file: " << filename, ((std::string)filename))

ERS_DECLARE_ISSUE(readoutmodules,
                  PostprocessingNotKeepingUp,
                  "SourceID[" << sid << "] Postprocessing has too much backlog, thread: " << i,
                  ((daqdataformats::SourceID)sid)((size_t)i))

ERS_DECLARE_ISSUE(readoutmodules,
                  EmptySourceBuffer,
                  "SourceID[" << sid << "] Source Buffer is empty, check file: " << filename,
                  ((daqdataformats::SourceID)sid)((std::string)filename))

ERS_DECLARE_ISSUE(readoutmodules,
                  CannotReadFromQueue,
                  "SourceID[" << sid << "] Failed attempt to read from the queue: " << queuename,
                  ((daqdataformats::SourceID)sid)((std::string)queuename))

ERS_DECLARE_ISSUE(readoutmodules,
                  CannotWriteToQueue,
                  "SourceID[" << sid << "] Failed attempt to write to the queue: " << queuename
                              << ". Data will be lost!",
                  ((daqdataformats::SourceID)sid)((std::string)queuename))

ERS_DECLARE_ISSUE(readoutmodules,
                  TrmWithEmptyFragment,
                  "SourceID[" << sid << "] Trigger Matching result with empty fragment: " << trmdetails,
                  ((daqdataformats::SourceID)sid)((std::string)trmdetails))

ERS_DECLARE_ISSUE(readoutmodules,
                  RequestOnEmptyBuffer,
                  "SourceID[" << sid << "] Request on empty buffer: " << trmdetails,
                  ((daqdataformats::SourceID)sid)((std::string)trmdetails))

ERS_DECLARE_ISSUE_BASE(readoutmodules,
                       FailedReadoutInitialization,
                       readoutmodules::InitializationError,
                       " Couldnt initialize Readout with current Init arguments " << initparams << ' ',
                       ((std::string)name),
                       ((std::string)initparams))

ERS_DECLARE_ISSUE(readoutmodules,
                  FailedFakeCardInitialization,
                  "Could not initialize fake card " << name,
                  ((std::string)name))

ERS_DECLARE_ISSUE_BASE(readoutmodules,
                       NoImplementationAvailableError,
                       readoutmodules::ConfigurationError,
                       " No " << impl << " implementation available for raw type: " << rawt << ' ',
                       ((daqdataformats::SourceID)sid)((std::string)impl),
                       ((std::string)rawt))

ERS_DECLARE_ISSUE(readoutmodules,
                  ResourceQueueError,
                  " The " << queueType << " queue was not successfully created for " << moduleName,
                  ((std::string)queueType)((std::string)moduleName))

ERS_DECLARE_ISSUE_BASE(readoutmodules,
                       DataRecorderResourceQueueError,
                       readoutmodules::DataRecorderConfigurationError,
                       " The " << queueType << " queue was not successfully created. ",
                       ((std::string)name),
                       ((std::string)queueType))

ERS_DECLARE_ISSUE(readoutmodules,
                  GenericResourceQueueError,
                  "The " << queueType << " queue was not successfully created for " << moduleName,
                  ((std::string)queueType)((std::string)moduleName))

ERS_DECLARE_ISSUE(readoutmodules,
                  ConfigurationNote,
                  "ConfigurationNote: " << text,
                  ((std::string)name)((std::string)text))

ERS_DECLARE_ISSUE(readoutmodules,
                  ConfigurationProblem,
                  "SourceID[" << sid << "] Configuration problem: " << text,
                  ((daqdataformats::SourceID)sid)((std::string)text))

ERS_DECLARE_ISSUE(readoutmodules,
                  RequestTimedOut,
                  "SourceID[" << sid << "] Request timed out",
                  ((daqdataformats::SourceID)sid))

ERS_DECLARE_ISSUE(readoutmodules,
                  EndOfRunEmptyFragment,
                  "SourceID[" << sid << "] Empty fragment at the end of the run",
                  ((daqdataformats::SourceID)sid))

} // namespace dunedaq

#endif // READOUTMODULES_INCLUDE_READOUT_READOUTISSUES_HPP_

/**
 * @file TimeSyncConsumer.cpp Module that consumes TimeSync's from a queue
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DummyConsumer.hpp"
#include "dfmessages/TimeSync.hpp"

DEFINE_DUNE_DAQ_MODULE(dunedaq::readoutmodules::DummyConsumer<dunedaq::dfmessages::TimeSync>)


namespace dunedaq {
namespace readoutmodules {

FakeCardReaderBase::FakeCardReaderBase(const std::string& name)
  : m_configured(false)
  , m_name(name)
  , m_run_marker{ false }
{
/*
  register_command("conf", &FakeCardReaderBase::do_conf);
  register_command("scrap", &FakeCardReaderBase::do_scrap);
  register_command("start", &FakeCardReaderBase::do_start);
  register_command("drain_dataflow", &FakeCardReaderBase::do_stop);
*/
}

void
FakeCardReaderBase::init(std::shared_ptr<appfwk::ModuleConfiguration> cfg)
{
  m_cfg = cfg;
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering init() method";
  //auto ini = args.get<appfwk::app::ModInit>();
  auto ini = cfg->module<appdal::DataReader>(m_name);
  if (ini != nullptr && ini->get_configuration()->get_emulation_mode()) {

    for (auto qi : ini->get_outputs()) {
      
      try {
        if (m_source_emus.find(qi->UID()) != m_source_emus.end()) {
          TLOG() << get_fcr_name() << "Same queue instance used twice";
          throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_fcr_name(), "");
        }
        m_source_emus[qi->UID()] = create_source_emulator(qi->UID(), m_run_marker);
        if (m_source_emus[qi->UID()].get() == nullptr) {
          TLOG() << get_fcr_name() << "Source emulator could not be created";
          throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_fcr_name(), "");
        }
        // m_source_emus[qi->UID()]->init(cfg);
        m_source_emus[qi->UID()]->set_sender(qi->UID());
      } catch (const ers::Issue& excpt) {
        throw readoutlibs::ResourceQueueError(ERS_HERE, qi->UID(), get_fcr_name(), excpt);
      }
    }
  }
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Exiting init() method";
}

void
FakeCardReaderBase::get_info(opmonlib::InfoCollector& ci, int level)
{

  for (auto& [name, emu] : m_source_emus) {
    emu->get_info(ci, level);
  }
}

void
FakeCardReaderBase::do_conf(const nlohmann::json& /*args*/)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering do_conf() method";

  if (m_configured) {
    TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_WORK_STEPS) << "This module is already configured!";
  } else {
    auto cfg = m_cfg->module<appdal::DataReader>(get_fcr_name());

    std::map<uint32_t, const coredal::DROStreamConf*> streams;
    for (const auto & readout_if : cfg->get_interfaces()) {
      for (const auto& data_stream : readout_if->get_contains()) {
        auto dro_stream = data_stream->cast<coredal::DROStreamConf>();
        if (dro_stream != nullptr) {
          streams[dro_stream->get_source_id()] = dro_stream;
        } 
      }
    }

    for (const auto& qi : cfg->get_outputs()) {
      auto q_with_id = qi->cast<coredal::QueueWithId>();
      if (q_with_id == nullptr) {
        throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_fcr_name(), "Queue is not of type QueueWithId");
      }  
      if (m_source_emus.find(q_with_id->UID()) == m_source_emus.end()) {
       TLOG() << "Cannot find queue: " <<  q_with_id->UID() << std::endl;
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Cannot find queue: " + q_with_id->UID());
      }
      if (m_source_emus[q_with_id->UID()]->is_configured()) {
        TLOG() << "Emulator for queue name " << q_with_id->UID() << " was already configured";
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Emulator configured twice: " + q_with_id->UID());
      }
      m_source_emus[q_with_id->UID()]->conf(streams[q_with_id->get_source_id()], cfg->get_configuration()->get_emulation_conf());
    }
    for (auto& [name, emu] : m_source_emus) {
      if (!emu->is_configured()) {
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Not all links were configured");
      }
    }

    // Mark configured
    m_configured = true;
  }

  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Exiting do_conf() method";
}

void
FakeCardReaderBase::do_scrap(const nlohmann::json& args)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering do_scrap() method";

  for (auto& [name, emu] : m_source_emus) {
    emu->scrap(args);
  }

  m_configured = false;

  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Exiting do_scrap() method";
}
void
FakeCardReaderBase::do_start(const nlohmann::json& args)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering do_start() method";

  m_run_marker.store(true);

  for (auto& [name, emu] : m_source_emus) {
    emu->start(args);
  }

  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Exiting do_start() method";
}

void
FakeCardReaderBase::do_stop(const nlohmann::json& args)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering do_stop() method";

  m_run_marker = false;

  for (auto& [name, emu] : m_source_emus) {
    emu->stop(args);
  }

  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Exiting do_stop() method";
}

} // namespace readoutmodules
} // namespace dunedaq


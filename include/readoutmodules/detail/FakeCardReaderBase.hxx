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
FakeCardReaderBase::init(const nlohmann::json& args)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering init() method";
  auto ini = args.get<appfwk::app::ModInit>();
  for (const auto& qi : ini.conn_refs) {
    
    try {
      if (m_source_emus.find(qi.name) != m_source_emus.end()) {
        TLOG() << get_fcr_name() << "Same queue instance used twice";
        throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_fcr_name(), args.dump());
      }
      m_source_emus[qi.name] = create_source_emulator(qi, m_run_marker);
      if (m_source_emus[qi.name].get() == nullptr) {
        TLOG() << get_fcr_name() << "Source emulator could not be created";
        throw readoutlibs::FailedFakeCardInitialization(ERS_HERE, get_fcr_name(), args.dump());
      }
      m_source_emus[qi.name]->init(args);
      m_source_emus[qi.name]->set_sender(qi.uid);
    } catch (const ers::Issue& excpt) {
      throw readoutlibs::ResourceQueueError(ERS_HERE, qi.name, get_fcr_name(), excpt);
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
FakeCardReaderBase::do_conf(const nlohmann::json& args)
{
  TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_ENTER_EXIT_METHODS) << get_fcr_name() << ": Entering do_conf() method";

  if (m_configured) {
    TLOG_DEBUG(dunedaq::readoutlibs::logging::TLVL_WORK_STEPS) << "This module is already configured!";
  } else {
    m_cfg = args.get<readoutlibs::sourceemulatorconfig::Conf>();

    for (const auto& emu_conf : m_cfg.link_confs) {
      if (m_source_emus.find(emu_conf.queue_name) == m_source_emus.end()) {
        TLOG() << "Cannot find queue: " << emu_conf.queue_name << std::endl;
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Cannot find queue: " + emu_conf.queue_name);
      }
      if (m_source_emus[emu_conf.queue_name]->is_configured()) {
        TLOG() << "Emulator for queue name " << emu_conf.queue_name << " was already configured";
        throw readoutlibs::GenericConfigurationError(ERS_HERE, "Emulator configured twice: " + emu_conf.queue_name);
      }
      m_source_emus[emu_conf.queue_name]->conf(args, emu_conf);
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


# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io

moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('flxlibs/felixcardreader.jsonnet')
moo.otypes.load_types('dtpctrllibs/dtpcontroller.jsonnet')
moo.otypes.load_types("readoutlibs/sourceemulatorconfig.jsonnet")
moo.otypes.load_types("readoutlibs/readoutconfig.jsonnet")
moo.otypes.load_types('lbrulibs/pacmancardreader.jsonnet')
moo.otypes.load_types("readoutlibs/recorderconfig.jsonnet")

# Import new types
import dunedaq.readoutlibs.sourceemulatorconfig as sec
import dunedaq.flxlibs.felixcardreader as flxcr
import dunedaq.dtpctrllibs.dtpcontroller as dtpctrl
import dunedaq.readoutlibs.readoutconfig as rconf
import dunedaq.lbrulibs.pacmancardreader as pcr
import dunedaq.readoutlibs.recorderconfig as bfs

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
from daqconf.core.conf_utils import Endpoint, Direction, Queue
from daqconf.core.sourceid import TPInfo, SourceIDBroker, FWTPID, FWTPOUTID

from detdataformats._daq_detdataformats_py import *
from os import path

# Time to waait on pop()
QUEUE_POP_WAIT_MS = 100


def generate(
    DRO_CONFIG=None,
    CLOCK_SPEED_HZ=62500000,
    DATA_RATE_SLOWDOWN_FACTOR=1,
    DATA_FILE="./frames.bin",
    TP_DATA_FILE="./tp_frames.bin",
    HOST="localhost",
    EMULATOR_MODE=False,
    FLX_INPUT=False,
    ETH_MODE=False,
    RAW_RECORDING_ENABLED=False,
    RAW_RECORDING_OUTPUT_DIR=".",
    SOFTWARE_TPG_ENABLED=False,
    FIRMWARE_TPG_ENABLED=False,
    FAKE_TIMESTAMP=False,
    DTP_CONNECTIONS_FILE="${DTPCONTROLS_SHARE}/config/dtp_connections.xml",
    FIRMWARE_HIT_THRESHOLD=20,
    TPG_CHANNEL_MAP= "ProtoDUNESP1ChannelMap",
    LATENCY_BUFFER_SIZE=499968,
    DATA_REQUEST_TIMEOUT=1000,
    SOURCEID_BROKER : SourceIDBroker = None,
    DEBUG=False
):

    modules = []
    queues = []

    host = DRO_CONFIG.host.replace("-","_")
    RUIDX = f"{host}_{DRO_CONFIG.card}"

    link_to_tp_sid_map = {}
    fw_tp_id_map = {}
    fw_tp_out_id_map = {}

    if SOFTWARE_TPG_ENABLED:
        for link in DRO_CONFIG.links:
            link_to_tp_sid_map[link.dro_source_id] = SOURCEID_BROKER.get_next_source_id("Trigger")
            SOURCEID_BROKER.register_source_id("Trigger", link_to_tp_sid_map[link.dro_source_id], None)
    if FIRMWARE_TPG_ENABLED:
        for fwsid,fwconf in SOURCEID_BROKER.get_all_source_ids("Detector_Readout").items():
            if isinstance(fwconf, FWTPID) and fwconf.host == DRO_CONFIG.host and fwconf.card == DRO_CONFIG.card:
                if DEBUG: print(f"SSB fwsid: {fwsid}")
                fw_tp_id_map[fwconf] = fwsid
                link_to_tp_sid_map[fwconf] = SOURCEID_BROKER.get_next_source_id("Trigger")
                SOURCEID_BROKER.register_source_id("Trigger", link_to_tp_sid_map[fwconf], None)
            if isinstance(fwconf, FWTPOUTID) and fwconf.host == DRO_CONFIG.host and fwconf.card == DRO_CONFIG.card:
                if DEBUG: print(f"SSB fw tp out id: {fwconf}")
                fw_tp_out_id_map[fwconf] = fwsid

    if DEBUG: print(f"SSB fwsid_map: {fw_tp_id_map}")
    if DEBUG: print(f"SSB fw_tp_out source ID map: {fw_tp_out_id_map}")

    # Hack on strings to be used for connection instances: will be solved when data_type is properly used.

    FWTP_TICK_LENGTH=32
    FRONTEND_TYPE = DetID.subdetector_to_string(DetID.Subdetector(DRO_CONFIG.links[0].det_id))
    if ((FRONTEND_TYPE== "HD_TPC" or FRONTEND_TYPE== "VD_Bottom_TPC") and CLOCK_SPEED_HZ== 50000000):
        FRONTEND_TYPE = "wib"
        FWTP_TICK_LENGTH=25
    elif ((FRONTEND_TYPE== "HD_TPC" or FRONTEND_TYPE== "VD_Bottom_TPC") and CLOCK_SPEED_HZ== 62500000 and ETH_MODE==False):
        FRONTEND_TYPE = "wib2"
    elif ((FRONTEND_TYPE== "HD_TPC" or FRONTEND_TYPE== "VD_Bottom_TPC") and CLOCK_SPEED_HZ== 62500000 and ETH_MODE==True):
        FRONTEND_TYPE = "wibeth"
    elif FRONTEND_TYPE== "HD_PDS" or FRONTEND_TYPE== "VD_Cathode_PDS" or FRONTEND_TYPE=="VD_Membrane_PDS":
        FRONTEND_TYPE = "pds_list"
    elif FRONTEND_TYPE== "VD_Top_TPC":
        FRONTEND_TYPE = "tde"
    elif FRONTEND_TYPE== "NDLAr_TPC":
        FRONTEND_TYPE = "pacman"
    elif FRONTEND_TYPE== "NDLAr_PDS":
        FRONTEND_TYPE = "mpd"
    

    if DEBUG: print(f'FRONTENT_TYPE={FRONTEND_TYPE}')

    if FLX_INPUT:
        link_0 = []
        link_1 = []
        sid_0 = []
        sid_1 = []
        for link in DRO_CONFIG.links:
            if link.dro_slr == 0:
                link_0.append(link.dro_link)
                sid_0.append(link.dro_source_id)
            if link.dro_slr == 1:
                link_1.append(link.dro_link)
                sid_1.append(link.dro_source_id)
        for idx in sid_0:
            queues += [Queue(f'flxcard_0.output_{idx}',f"datahandler_{idx}.raw_input",f'{FRONTEND_TYPE}_link_{idx}', 100000 )]
        for idx in sid_1:
            queues += [Queue(f'flxcard_1.output_{idx}',f"datahandler_{idx}.raw_input",f'{FRONTEND_TYPE}_link_{idx}', 100000 )]
        if FIRMWARE_TPG_ENABLED:
            link_0.append(5)
            fw_tp_sid = fw_tp_id_map[FWTPID(DRO_CONFIG.host, DRO_CONFIG.card, 0)]
            queues += [Queue(f'flxcard_0.output_{fw_tp_sid}',f"tp_datahandler_{fw_tp_sid}.raw_input",f'raw_tp_link_{fw_tp_sid}', 100000 )]
            if len(link_1) > 0:
                link_1.append(5)
                fw_tp_sid = fw_tp_id_map[FWTPID(DRO_CONFIG.host, DRO_CONFIG.card, 1)]
                queues += [Queue(f'flxcard_1.output_{fw_tp_sid}',f"tp_datahandler_{fw_tp_sid}.raw_input",f'raw_tp_link_{fw_tp_sid}', 100000 )]

        modules += [DAQModule(name = 'flxcard_0',
                              plugin = 'FelixCardReader',
                              conf = flxcr.Conf(card_id = DRO_CONFIG.links[0].dro_card,
                                                logical_unit = 0,
                                                dma_id = 0,
                                                chunk_trailer_size = 32,
                                                dma_block_size_kb = 4,
                                                dma_memory_size_gb = 4,
                                                numa_id = 0,
                                                links_enabled = link_0))]
            
        if len(link_1) > 0:
            modules += [DAQModule(name = "flxcard_1",
                                  plugin = "FelixCardReader",
                                  conf = flxcr.Conf(card_id = DRO_CONFIG.links[0].dro_card,
                                                    logical_unit = 1,
                                                    dma_id = 0,
                                                    chunk_trailer_size = 32,
                                                    dma_block_size_kb = 4,
                                                    dma_memory_size_gb = 4,
                                                    numa_id = 0,
                                                    links_enabled = link_1))]
        # DTPController - only required if FW TPs enabled
        if FIRMWARE_TPG_ENABLED:
            if len(link_0) > 0:
                modules += [DAQModule(
                            name = 'dtpctrl_0',
                            plugin = 'DTPController',
                            conf = dtpctrl.Conf(connections_file=path.expandvars(DTP_CONNECTIONS_FILE),
                                                device="flx-0-p2-hf",
                                                uhal_log_level="notice",
                                                source="ext",
                                                pattern="",
                                                threshold=FIRMWARE_HIT_THRESHOLD,
                                                masks=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]) )]
            if len(link_1) > 0:
                modules += [DAQModule(
                            name = 'dtpctrl_1',
                            plugin = 'DTPController',
                            conf = dtpctrl.Conf(connections_file=path.expandvars(DTP_CONNECTIONS_FILE),
                                                device="flx-1-p2-hf",
                                                uhal_log_level="notice",
                                                source="ext",
                                                pattern="",
                                                threshold=FIRMWARE_HIT_THRESHOLD,
                                                masks=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]) )]

    else:
        fake_source = "fake_source"
        card_reader = "FakeCardReader"
        conf = sec.Conf(link_confs = [sec.LinkConfiguration(source_id=link.dro_source_id,
                                                            slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                                                            queue_name=f"output_{link.dro_source_id}",
                                                            data_filename = DATA_FILE,
                                                            emu_frame_error_rate=0) for link in DRO_CONFIG.links
                                     ] + [
                                      sec.LinkConfiguration(source_id=tp_link,
                                                            slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                                                            queue_name=f"output_raw_tp_{tp_link}",
                                                            tp_data_filename=TP_DATA_FILE,
                                                            emu_frame_error_rate=0) for tp_link in link_to_tp_sid_map.values()
                                     ],
                        # input_limit=10485100, # default
                        queue_timeout_ms = QUEUE_POP_WAIT_MS,
                        set_t0_to=0)
            
        if FRONTEND_TYPE=='pacman':
            fake_source = "pacman_source"
            card_reader = "PacmanCardReader"
            conf = pcr.Conf(link_confs = [pcr.LinkConfiguration(Source_ID=link.dro_source_id)
                                          for link in DRO_CONFIG.links],
                            zmq_receiver_timeout = 10000)
        modules += [DAQModule(name = fake_source,
                              plugin = card_reader,
                              conf = conf)]
        queues += [Queue(f"{fake_source}.output_{link.dro_source_id}",f"datahandler_{link.dro_source_id}.raw_input",f'{FRONTEND_TYPE}_link_{link.dro_source_id}', 100000) for link in DRO_CONFIG.links]
        queues += [Queue(f"{fake_source}.output_raw_tp_{tp_link}",f"tp_datahandler_{tp_link}.raw_input",f'tp_link_{tp_link}', 100000) for tp_link in link_to_tp_sid_map.values()]

    errored_consumer_needed = False
    if SOFTWARE_TPG_ENABLED:
        queues += [Queue(f"datahandler_{link.dro_source_id}.tp_out",f"tp_datahandler_{link_to_tp_sid_map[link.dro_source_id]}.raw_input",f"sw_tp_link_{link.dro_source_id}",100000 )]                

    for link in DRO_CONFIG.links:
        #? why only create errored frames for wib, should this also be created for wib2 or other FE's?
        if FRONTEND_TYPE == 'wib':
            errored_consumer_needed = True
            queues += [Queue(f"datahandler_{link.dro_source_id}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q")]

        tpset_topic = "None"
        modules += [DAQModule(name = f"datahandler_{link.dro_source_id}",
                          plugin = "DataLinkHandler", 
                          conf = rconf.Conf(
                                  readoutmodelconf= rconf.ReadoutModelConf(
                                      source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                      fake_trigger_flag=1,
                                      source_id =  link.dro_source_id,
                                      timesync_connection_name = f"timesync_{RUIDX}",
                                      timesync_topic_name = "Timesync",
                                  ),
                                  latencybufferconf= rconf.LatencyBufferConf(
                                      latency_buffer_alignment_size = 4096,
                                      latency_buffer_size = LATENCY_BUFFER_SIZE,
                                      source_id =  link.dro_source_id,
                                  ),
                                  rawdataprocessorconf= rconf.RawDataProcessorConf(
                                      source_id =  link.dro_source_id,
                                      enable_software_tpg = SOFTWARE_TPG_ENABLED,
                                      channel_map_name = TPG_CHANNEL_MAP,
                                      emulator_mode = EMULATOR_MODE,
                                      fwtp_tick_length = FWTP_TICK_LENGTH,
                                      fwtp_fake_timestamp = FAKE_TIMESTAMP,
                                      error_counter_threshold=100,
                                      error_reset_freq=10000,
                                      tpset_topic=tpset_topic,
                                      tpset_sourceid=link_to_tp_sid_map[link.dro_source_id] if SOFTWARE_TPG_ENABLED else 0
                                  ),
                                  requesthandlerconf= rconf.RequestHandlerConf(
                                      latency_buffer_size = LATENCY_BUFFER_SIZE,
                                      pop_limit_pct = 0.8,
                                      pop_size_pct = 0.1,
                                      source_id = link.dro_source_id,
                                      det_id = DRO_CONFIG.links[0].det_id,
                                      output_file = path.join(RAW_RECORDING_OUTPUT_DIR, f"output_{RUIDX}_{link.dro_source_id}.out"),
                                      stream_buffer_size = 8388608,
                                      request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                      enable_raw_recording = RAW_RECORDING_ENABLED,
                                  )), extra_commands={"record": rconf.RecordingParams(duration=10)})]

    if SOFTWARE_TPG_ENABLED:
        for link in DRO_CONFIG.links:
            modules += [DAQModule(name = f"tp_datahandler_{link_to_tp_sid_map[link.dro_source_id]}",
                               plugin = "DataLinkHandler",
                               conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS,
                                                                                           fake_trigger_flag=1,
                                                                                           source_id = link_to_tp_sid_map[link.dro_source_id]),
                                                 latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                            source_id =  link_to_tp_sid_map[link.dro_source_id]),
                                                 rawdataprocessorconf = rconf.RawDataProcessorConf(source_id =  link_to_tp_sid_map[link.dro_source_id],
                                                                                                   enable_software_tpg = False,
                                                                                                   fwtp_tick_length = FWTP_TICK_LENGTH,
                                                                                                   fwtp_fake_timestamp = FAKE_TIMESTAMP,
                                                                                                   channel_map_name=TPG_CHANNEL_MAP),
                                                 requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                              pop_limit_pct = 0.8,
                                                                                              pop_size_pct = 0.1,
                                                                                              source_id = link_to_tp_sid_map[link.dro_source_id],
                                                                                              det_id = 1,
                                          output_file = path.join(RAW_RECORDING_OUTPUT_DIR, f"output_raw_tp_{RUIDX}_{link.dro_source_id}.out"),
                                                                                              stream_buffer_size = 100 if FRONTEND_TYPE=='pacman' else 8388608,
                                                                                              request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                                              enable_raw_recording = RAW_RECORDING_ENABLED)))]
    if FIRMWARE_TPG_ENABLED:
        assert(len(fw_tp_out_id_map) <= 2)
        assert(len(fw_tp_id_map) <= 2)
        for tp, tp_out in zip(fw_tp_id_map.values(), fw_tp_out_id_map.values()):
            errored_consumer_needed = True
            queues += [Queue(f"tp_datahandler_{tp}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q")]
            queues += [Queue(f"tp_datahandler_{tp}.tp_out",f"tp_out_datahandler_{tp_out}.raw_input",f"sw_tp_link_{tp_out}",100000 )]                
            modules += [DAQModule(name = f"tp_out_datahandler_{tp_out}",
                               plugin = "DataLinkHandler",
                               conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS,
                                                                                           fake_trigger_flag=1,
                                                                                         source_id = tp_out),
                                                 latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                            source_id = tp_out),
                                                 rawdataprocessorconf = rconf.RawDataProcessorConf(source_id =  tp_out,
                                                                                                   enable_software_tpg = False,
                                                                                                   fwtp_tick_length = FWTP_TICK_LENGTH,
                                                                                                   fwtp_fake_timestamp = FAKE_TIMESTAMP,
                                                                                                   channel_map_name=TPG_CHANNEL_MAP),
                                                 requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                              pop_limit_pct = 0.8,
                                                                                              pop_size_pct = 0.1,
                                                                                              source_id = tp_out,
                                                                                              det_id = 1,
                                                                                              stream_buffer_size = 100 if FRONTEND_TYPE=='pacman' else 8388608,
                                                                                              request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                                              enable_raw_recording = False)))]

            modules += [DAQModule(name = f"tp_datahandler_{tp}",
                                  plugin = "DataLinkHandler", 
                                  conf = rconf.Conf(
                                      readoutmodelconf= rconf.ReadoutModelConf(
                                          source_queue_timeout_ms= QUEUE_POP_WAIT_MS,
                                          fake_trigger_flag=1,
                                          source_id = tp,
                                          timesync_connection_name = f"timesync_{RUIDX}",
                                          timesync_topic_name = "Timesync",
                                      ),
                                      latencybufferconf= rconf.LatencyBufferConf(
                                          latency_buffer_alignment_size = 4096,
                                          latency_buffer_size = LATENCY_BUFFER_SIZE,
                                          source_id = tp,
                                      ),
                                      rawdataprocessorconf= rconf.RawDataProcessorConf(
                                          source_id = tp,
                                          enable_software_tpg = False,
                                          enable_firmware_tpg = True,
                                          channel_map_name = TPG_CHANNEL_MAP,
                                          emulator_mode = EMULATOR_MODE,
                                          fwtp_tick_length = FWTP_TICK_LENGTH,
                                          fwtp_fake_timestamp = FAKE_TIMESTAMP,
                                          error_counter_threshold=100,
                                          error_reset_freq=10000,
                                          tpset_topic="TPSets"
                                      ),
                                      requesthandlerconf= rconf.RequestHandlerConf(
                                          latency_buffer_size = LATENCY_BUFFER_SIZE,
                                          pop_limit_pct = 0.8,
                                          pop_size_pct = 0.1,
                                          source_id = tp,
                                          det_id = DRO_CONFIG.links[0].det_id,
                                          output_file = path.join(RAW_RECORDING_OUTPUT_DIR, f"output_tp_{RUIDX}_{tp}.out"),
                                          stream_buffer_size = 8388608,
                                          request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                          enable_raw_recording = RAW_RECORDING_ENABLED,
                                      )))]



    modules += [DAQModule(name="timesync_consumer", plugin="TimeSyncConsumer")]
    modules += [DAQModule(name="fragment_consumer", plugin="FragmentConsumer")]

    if errored_consumer_needed:
        modules += [DAQModule(name="errored_frame_consumer", plugin="ErroredFrameConsumer")]

    mgraph = ModuleGraph(modules, queues=queues)


    if FIRMWARE_TPG_ENABLED:
        for sid in fw_tp_id_map.values():
            mgraph.connect_modules(f"tp_datahandler_{sid}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
            mgraph.connect_modules(f"tp_datahandler_{sid}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
            mgraph.add_endpoint(f"tp_requests_{sid}", f"tp_datahandler_{sid}.request_input", Direction.IN)
            mgraph.add_endpoint(f"tp_requests_{sid}", None, Direction.OUT) # Fake request endpoint
            mgraph.add_endpoint(f"tpsets_link{sid}", f"tp_datahandler_{sid}.tpset_out",    Direction.OUT, topic=["TPSets"])
            mgraph.add_endpoint(f"tpsets_link{sid}", None,    Direction.IN, topic=["TPSets"]) # Fake TPSet endpoing
            
        for sid in fw_tp_out_id_map.values():
            mgraph.connect_modules(f"tp_out_datahandler_{sid}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
            mgraph.connect_modules(f"tp_out_datahandler_{sid}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
            mgraph.add_endpoint(f"tp_out_requests_{sid}", f"tp_out_datahandler_{sid}.request_input", Direction.IN)
            mgraph.add_endpoint(f"tp_out_requests_{sid}", None, Direction.OUT) # Fake request endpoint




    for link in DRO_CONFIG.links:
        if SOFTWARE_TPG_ENABLED:
            mgraph.connect_modules(f"tp_datahandler_{link_to_tp_sid_map[link.dro_source_id]}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
            mgraph.connect_modules(f"tp_datahandler_{link_to_tp_sid_map[link.dro_source_id]}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)

            mgraph.add_endpoint(f"tp_requests_{link_to_tp_sid_map[link.dro_source_id]}", f"tp_datahandler_{link_to_tp_sid_map[link.dro_source_id]}.request_input", Direction.IN)
            mgraph.add_endpoint(f"tp_requests_{link_to_tp_sid_map[link.dro_source_id]}", None, Direction.OUT) # Fake request endpoint
            
            mgraph.add_endpoint(f"tpsets_link{link.dro_source_id}", f"datahandler_{link.dro_source_id}.tpset_out",    Direction.OUT, topic=["TPSets"])
            mgraph.add_endpoint(f"tpsets_link{link.dro_source_id}", None,    Direction.IN, topic=["TPSets"]) # Fake TPSet endpoint
        
        mgraph.connect_modules(f"datahandler_{link.dro_source_id}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"datahandler_{link.dro_source_id}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)

        mgraph.add_endpoint(f"requests_{link.dro_source_id}", f"datahandler_{link.dro_source_id}.request_input", Direction.IN)
        mgraph.add_endpoint(f"requests_{link.dro_source_id}", None, Direction.OUT) # Fake request endpoint

    ru_app = App(modulegraph=mgraph, host=HOST, name="readout_app")
    if DEBUG:
        ru_app.export("readout_app.dot")
    return ru_app

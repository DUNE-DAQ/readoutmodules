# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io

moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types("readoutlibs/sourceemulatorconfig.jsonnet")
moo.otypes.load_types("readoutlibs/readoutconfig.jsonnet")
moo.otypes.load_types("readoutlibs/recorderconfig.jsonnet")

# Import new types
import dunedaq.readoutlibs.sourceemulatorconfig as sec
import dunedaq.readoutlibs.readoutconfig as rconf
import dunedaq.readoutlibs.recorderconfig as bfs

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
from daqconf.core.conf_utils import Endpoint, Direction, Queue
from daqconf.core.sourceid import TPInfo, SourceIDBroker

# Time to waait on pop()
QUEUE_POP_WAIT_MS = 100
# local clock speed Hz
CLOCK_SPEED_HZ = 50000000


def generate(
    DRO_CONFIG=None,
    SOURCEID_BROKER=None,
    FRONTEND_TYPE="raw_tp",
    NUMBER_OF_DATA_PRODUCERS=1,
    NUMBER_OF_TP_PRODUCERS=1,
    DATA_RATE_SLOWDOWN_FACTOR=1,
    ENABLE_SOFTWARE_TPG=False,
    CHANNEL_MAP_NAME="ProtoDUNESP1ChannelMap", 
    FWTP_STITCH_CONSTANT=2048,
    FWTP_FORMAT_VERSION=1,
    RUN_NUMBER=333,
    DATA_FILE="./frames.bin",
    TP_DATA_FILE="./tp_frames.bin",
    HOST="localhost",
    READOUT_SENDS_TP_FRAGMENTS=False,
):

    modules = []
    queues = []

    host = DRO_CONFIG.host.replace("-","_")
    RUIDX = f"{host}_{DRO_CONFIG.card}"

    modules += [DAQModule(name="fake_source", plugin="FakeCardReader", conf=sec.Conf(
                    link_confs=[
                        sec.LinkConfiguration(
                            source_id =  link.dro_source_id,
                            slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                            queue_name=f"output_{link.dro_source_id}",
                            data_filename=TP_DATA_FILE,
                            emu_frame_error_rate=0,
                        )
                        for link in DRO_CONFIG.links],
                    # input_limit=10485100, # default
                    queue_timeout_ms=QUEUE_POP_WAIT_MS
            ))]

    queues += [Queue(f"fake_source.output_{link.dro_source_id}",f"raw_tp_handler_{link.dro_source_id}.raw_input",f'{FRONTEND_TYPE}_link_{link.dro_source_id}', 100000) for link in DRO_CONFIG.links]
   
    for link in DRO_CONFIG.links: 
        modules += [DAQModule(name=f"raw_tp_handler_{link.dro_source_id}", plugin="DataLinkHandler", conf=rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
			source_id = link.dro_source_id,
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        timesync_connection_name = f"timesync_{link.dro_source_id}",
                        timesync_topic_name = "Timesync",
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        source_id = link.dro_source_id,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        source_id = link.dro_source_id,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                        channel_map_name=CHANNEL_MAP_NAME,
                        fwtp_stitch_constant=FWTP_STITCH_CONSTANT,
                        fwtp_format_version=FWTP_FORMAT_VERSION,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        source_id = link.dro_source_id,
                        output_file=f"output_raw_tp_{link.dro_source_id}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=True,
                    ),
                ))]

    modules += [DAQModule(name="timesync_consumer", plugin="TimeSyncConsumer")]
    modules += [DAQModule(name="fragment_consumer", plugin="FragmentConsumer")]

    mgraph = ModuleGraph(modules, queues=queues)

    for link in DRO_CONFIG.links:
        mgraph.add_fragment_producer(id = link.dro_source_id, subsystem = "Detector Readout",
            requests_in   = f"raw_tp_handler_{link.dro_source_id}.request_input",
            fragments_out = f"raw_tp_handler_{link.dro_source_id}.fragment_queue")
        mgraph.connect_modules(f"raw_tp_handler_{link.dro_source_id}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"raw_tp_handler_{link.dro_source_id}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
        mgraph.add_endpoint(f"rtp_requests_{link.dro_source_id}", f"raw_tp_handler_{link.dro_source_id}.request_input", Direction.IN)
        mgraph.add_endpoint(f"rtp_requests_{link.dro_source_id}", None, Direction.OUT) # Fake request endpoint
        
    ru_app = App(modulegraph=mgraph, host=HOST, name="readout_app")
    return ru_app

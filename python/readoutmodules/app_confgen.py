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

# Time to waait on pop()
QUEUE_POP_WAIT_MS = 100
# local clock speed Hz
CLOCK_SPEED_HZ = 50000000


def generate(
    FRONTEND_TYPE="wib",
    NUMBER_OF_DATA_PRODUCERS=1,
    NUMBER_OF_TP_PRODUCERS=1,
    DATA_RATE_SLOWDOWN_FACTOR=1,
    ENABLE_SOFTWARE_TPG=False,
    RUN_NUMBER=333,
    DATA_FILE="./frames.bin",
    TP_DATA_FILE="./tp_frames.bin",
    HOST="localhost"
):

    modules = []
    queues = []

    modules += [DAQModule(name="fake_source", plugin="FakeCardReader", conf=sec.Conf(
                    link_confs=[
                        sec.LinkConfiguration(
                            geoid=sec.GeoID(system="TPC", region=0, element=idx),
                            slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                            queue_name=f"output_{idx}",
                            data_filename=DATA_FILE,
                            emu_frame_error_rate=0,
                        )
                        for idx in range(NUMBER_OF_DATA_PRODUCERS)
                    ]
                    + [
                        sec.LinkConfiguration(
                            geoid=sec.GeoID(system="TPC", region=0, element=idx),
                            slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                            queue_name=f"output_raw_tp_{idx}",
                            tp_data_filename=TP_DATA_FILE,
                            emu_frame_error_rate=0,
                        )
                        for idx in range(
                            NUMBER_OF_DATA_PRODUCERS,
                            NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS,
                        )
                    ],
                    # input_limit=10485100, # default
                    queue_timeout_ms=QUEUE_POP_WAIT_MS,
                    set_t0_to=0,
                ))]

    queues += [Queue(f"fake_source.output_{idx}",f"datahandler_{idx}.raw_input",f'{FRONTEND_TYPE}_link_{idx}', 100000) for idx in range(NUMBER_OF_DATA_PRODUCERS)]
    queues += [Queue(f"fake_source.output_raw_tp_{idx}",f"raw_tp_handler_{idx}.raw_input",f'raw_tp_link_{idx}', 100000) for idx in range(NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS)]

    for idx in range(NUMBER_OF_DATA_PRODUCERS):
        if ENABLE_SOFTWARE_TPG:
            queues += [Queue(f"datahandler_{idx}.tp_out",f"sw_tp_handler_{idx}.raw_input",f"sw_tp_link_{idx}",100000 )]                
                
        if FRONTEND_TYPE == 'wib':
            queues += [Queue(f"datahandler_{idx}.errored_frames", 'errored_frame_consumer.input_queue', "errored_frames_q", 10000)]

        modules += [DAQModule(name=f"datahandler_{idx}", plugin="DataLinkHandler", conf=rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                        timesync_connection_name = f"timesync_dlh_{idx}",
                        timesync_topic_name = "Timesync",
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                        error_counter_threshold=100,
                        error_reset_freq=10000,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=True,
                    ),
                ), extra_commands={"record": rconf.RecordingParams(duration=10)})]

        if ENABLE_SOFTWARE_TPG:
            modules += [DAQModule(name=f"sw_tp_handler_{idx}", plugin="DataLinkHandler", conf=rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                        timesync_connection_name = f"timesync_tp_dlh_{idx}",
                        timesync_topic_name = "Timesync",
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=False,
                    ),
                ))]

    modules += [DAQModule(name=f"raw_tp_handler_{idx}", plugin="DataLinkHandler", conf=rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                        timesync_connection_name = f"timesync_rtp_{idx}",
                        timesync_topic_name = "Timesync",
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_raw_tp_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=True,
                    ),
                ))
            for idx in range(NUMBER_OF_DATA_PRODUCERS, NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS)]

    modules += [DAQModule(name="timesync_consumer", plugin="TimeSyncConsumer")]
    modules += [DAQModule(name="fragment_consumer", plugin="FragmentConsumer")]

    if FRONTEND_TYPE == 'wib':
        modules += [DAQModule(name="errored_frame_consumer", plugin="ErroredFrameConsumer")]

    mgraph = ModuleGraph(modules, queues=queues)
    for idx in range(NUMBER_OF_DATA_PRODUCERS):

        if ENABLE_SOFTWARE_TPG:
            mgraph.connect_modules(f"tp_datahandler_{idx}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
            mgraph.connect_modules(f"tp_datahandler_{idx}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
            mgraph.add_endpoint(f"tp_requests_{idx}", f"tp_datahandler_{idx}.request_input", Direction.IN)
            mgraph.add_endpoint(f"tp_requests_{idx}", None, Direction.OUT) # Fake request endpoint
        
        mgraph.connect_modules(f"datahandler_{idx}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"datahandler_{idx}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
        mgraph.add_endpoint(f"requests_{idx}", f"datahandler_{idx}.request_input", Direction.IN)
        mgraph.add_endpoint(f"requests_{idx}", None, Direction.OUT) # Fake request endpoint


    for idx in range(NUMBER_OF_DATA_PRODUCERS, NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS):
        mgraph.connect_modules(f"raw_tp_handler_{idx}.timesync_output", "timesync_consumer.input_queue", "timesync_q")
        mgraph.connect_modules(f"raw_tp_handler_{idx}.fragment_queue", "fragment_consumer.input_queue", "data_fragments_q", 100)
        mgraph.add_endpoint(f"rtp_requests_{idx}", f"raw_tp_handler_{idx}.request_input", Direction.IN)
        mgraph.add_endpoint(f"rtp_requests_{idx}", None, Direction.OUT) # Fake request endpoint
        
    ru_app = App(modulegraph=mgraph, host=HOST, name="readout_app")
    return ru_app

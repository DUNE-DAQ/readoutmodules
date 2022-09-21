// This is the configuration schema for readoutapp

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.readoutmodules.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {
  number: s.number  ("number", "i8", doc="a number"),

  readoutapp: s.record("readoutapp", [
    s.field('host',      daqconf.Host, default='localhost', doc='Host to run the readout app on'),
    s.field('card', self.number, default=0, doc='Card to read'),
    s.field("tpg_channel_map", daqconf.TPGChannelMap, default="ProtoDUNESP1ChannelMap", doc="Channel map for software TPG"),
    s.field("tp_data_file", daqconf.Path, default='./tp_frames.bin', doc="File to read TPs from")
    
  ]),

  readoutapp_gen: s.record('readoutapp_gen', [
    s.field('boot',     daqconf.boot,  default=daqconf.boot, doc='Boot parameters'),
    s.field('readout', daqconf.readout, default=daqconf.readout, doc='Readout paramaters'),
    s.field('readoutapp', self.readoutapp, default=self.readoutapp, doc='Readoutapp parameters'),
  ]),
};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)

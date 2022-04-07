// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Fake* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.readoutmodules.cpupinner";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local cpupinner = {
  thread_name : s.string("ThreadName",
    doc="A name of a thread"),

  cpu_id: s.number("CPUID", "i4", doc="A CPU ID"),

  cpu_set: s.sequence("CPUSet", self.cpu_id),

  thread_conf: s.record("ThreadConf", [
    s.field("name", self.thread_name),
    s.field("cpu_set", self.cpu_set),
  ]),

  thread_confs: s.sequence("ThreadConfs", self.thread_conf),
  
  cpupinnerconf: s.record("Conf", [
    s.field("thread_confs", self.thread_confs),
  ]),

};

moo.oschema.sort_select(cpupinner, ns)

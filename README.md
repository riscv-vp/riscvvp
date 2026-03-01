## RiscV Virtual platform (SystemC, Spike, Verilator).

## This is RiscV VP that has:
1. Each ISS (Spike) run in a separate SC_THREAD (loosely timed coding style).
2. RTL models into model communicate with functional models using (pins - queue) transactors.
3. Each model (RTL, functional, etc) is a separate sc_module that connects via TLM2.0 sockets.

## This repo contains 3 examples:

1. “two_iss” – contains 2 spike CPU cores, communicating with spike uncore model.
2. “tb_with_memrtl” - TLM2.0 based testbench that tests cycle accurate RTL memory model. That cycle accurate memory model contains Verilated RTL,  pin2queue and queue2pins adapters, b_transport callback.
3. “two_iss_with_memrtl” – two Spikes communicating with uncore model where cycle accurate Verilated RTL memory model in TLM2.0 wrapper is also connected.

## Running requirements:
1. SystemC installed (ver 3.0.1 tested)
2. Verilator installed (ver 5.034 tested)
3. RiscV (spike) compiled and installed (it is used as a library, i.e. only .h files and .so files needed).
4. RiscV toolchain to compile RiscV bin that can be run on Spike.


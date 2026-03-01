//============== Modifyed Verilator SystemC top module =================
//======================Original header=================================
// -*- SystemC -*-
// DESCRIPTION: Verilator Example: Top level main for invoking SystemC model
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2017 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
//======================================================================

#include <string>
#include <memory>                // For std::unique_ptr
#include <systemc.h>             // SystemC global header

#include <verilated.h>
#if VM_TRACE
#include <verilated_vcd_sc.h>
#endif
#include <sys/stat.h>  // mkdir

#include "uncore.h"
#include "spike.h"
#include "memrtl.h"

#include "vp_defines.h"

int sc_main (int argc, char* argv[])
{

  int running_threads_counter = 2;

  try {

    // Prevent unused variable warnings
    if (false && argc && argv) {}

    // Create logs/ directory in case we have traces to put under it
    Verilated::mkdir("logs");

    // Set debug level, 0 is off, 9 is highest presently used
    // May be overridden by commandArgs argument parsing
    Verilated::debug(0);

    // Randomization reset policy
    // May be overridden by commandArgs argument parsing
    Verilated::randReset(2);

#if VM_TRACE
    // Before any evaluation, need to know to calculate those signals only used for tracing
    Verilated::traceEverOn(true);
#endif

    // Pass arguments so Verilated code can see them, e.g. $value$plusargs
    // This needs to be called before you create any model
    Verilated::commandArgs(argc, argv);

    // General logfile
    ios::sync_with_stdio();


    cfg_t cfg;
    cfg.isa = "rv64imafdcv_zicsr";  // Standard RV64GC ISA
    cfg.priv = "MSU";        // Machine, Supervisor, and User privilege levels
    cfg.misaligned = false;  // Don't allow misaligned memory accesses
    cfg.endianness = endianness_little;  // Little endian
    cfg.start_pc = VP_START_PC;  // Start PC
    cfg.mem_layout.clear();
    cfg.pmpregions = 16;

    log_file_t log_fileA{"spike0_log.txt"};
    log_file_t log_fileB{"spike1_log.txt"};

    const std::unique_ptr<spike_module> spike0{new spike_module{"spike0", 0, running_threads_counter, &cfg, log_fileA}};
    const std::unique_ptr<spike_module> spike1{new spike_module{"spike1", 1, running_threads_counter, &cfg, log_fileB}};

    const std::unique_ptr<uncore_module> uncore{new uncore_module{"uncore", &cfg, "main.bin"}};
    const std::unique_ptr<memrtl_module> memrtl{new memrtl_module{"memrtl", sc_time(10, SC_NS), sc_time(17, SC_NS)}};

    spike0->initiator_socket.bind(uncore->target_socket0);
    spike1->initiator_socket.bind(uncore->target_socket1);
    uncore->initiator_socket.bind(memrtl->target_socket);

    spike0->enableDebug();
    spike1->enableDebug();
    uncore->enableDebug();
    memrtl->enableDebug();

    // You must do one evaluation before enabling waves, in order to allow
    // SystemC to interconnect everything for testing.
    sc_start(1, SC_NS);


#if VM_TRACE
    // If verilator was invoked with --trace argument,
    // and if at run time passed the +trace argument, turn on tracing
    VerilatedVcdSc* tfp = nullptr;
    const char* flag = Verilated::commandArgsPlusMatch("trace");
    if (flag && 0 == strcmp(flag, "+trace")) {
        cout << "Enabling waves into logs/vlt_dump.vcd...\n";
        tfp = new VerilatedVcdSc;
        memrtl->set_trace(tfp, 99);   // Trace 99 levels of hierarchy
        Verilated::mkdir("logs");
        tfp->open("logs/vlt_dump.vcd");
    }
#endif

   // Simulate while shared_threads_counter not zero
    while (running_threads_counter > 0) {
        // Simulate 50ns
        sc_start(50, SC_NS);
#if VM_TRACE
        // Flush the wave files each cycle so we can immediately see the output
        // Don't do this in "real" programs, do it in an abort() handler instead
        if (tfp) tfp->flush();
#endif
    }

    sc_stop();              //This calls destructors of all sc_modules

    // Close trace if opened
#if VM_TRACE
    if (tfp) {
        tfp->close();
        tfp = nullptr;
    }
#endif

    // Coverage analysis (calling write only after the test is known to pass)
#if VM_COVERAGE
    Verilated::mkdir("logs");
    VerilatedCov::write("logs/coverage.dat");
#endif

  } catch (const sc_report& e) {
        std::cerr << "Caught SystemC exception: " << e.what() << std::endl;
        return 1;
  }

  return 0;
}

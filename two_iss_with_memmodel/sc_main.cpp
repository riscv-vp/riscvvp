#include <string>
#include <memory>                // For std::unique_ptr
#include <systemc.h>             // SystemC global header
#include "uncore.h"
#include "spike.h"
#include "memmodel.h"

#include "vp_defines.h"

int sc_main (int argc, char* argv[])
{

  int running_threads_counter = 2;

  try {

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
    const std::unique_ptr<memmodel_module> memmodel{new memmodel_module{"memmodel"}};

    spike0->initiator_socket.bind(uncore->target_socket0);
    spike1->initiator_socket.bind(uncore->target_socket1);
    uncore->initiator_socket.bind(memmodel->target_socket);

    spike0->enableDebug();
    spike1->enableDebug();
    uncore->enableDebug();
    memmodel->enableDebug();

   // Simulate while shared_threads_counter not zero
    while (running_threads_counter > 0) {
        // Simulate 50ns
        sc_start(50, SC_NS);
    }

    sc_stop();              //This calls destructors of all sc_modules

  } catch (const sc_report& e) {
        std::cerr << "Caught SystemC exception: " << e.what() << std::endl;
        return 1;
  }

  return 0;
}

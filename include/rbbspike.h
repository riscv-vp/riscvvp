#ifndef RBBSPIKE_H
#define RBBSPIKE_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "riscv/simif.h"      // needed for base class
#include "riscv/cfg.h"        // needed for cfg_t* in header
#include "riscv/processor.h"  // needed for processor_t* in header
#include "riscv/log_file.h"   // for log_file_t
#include "riscv/mmu.h"
#include "riscv/devices.h"
#include "riscv/debug_module.h"
#include "riscv/jtag_dtm.h"
#include "riscv/remote_bitbang.h"
#include "dbg_component.h"    // needed for debug_component class

struct rbbspike_module: sc_module, simif_t, debug_component {

//Note that only Constructor, Destructor and TLM2.0 socket(s) are PUBLIC everything else is PRIVATE

  tlm_utils::simple_initiator_socket<rbbspike_module> initiator_socket;

  SC_CTOR(rbbspike_module, int spike_id, int& running_threads_counter, const cfg_t* cfg, log_file_t& logfile, const debug_module_config_t &dm_config, sc_time bitbang_delay, bool halted, uint16_t rbb_port) : spike_id(spike_id), running_threads_counter(running_threads_counter), initiator_socket("initiator_socket"), trans(std::make_unique<tlm::tlm_generic_payload>()), local_bus(std::make_unique<bus_t>(nullptr)), cfg(cfg),
                                                                                         proc(std::make_unique<processor_t>(cfg->isa, cfg->priv, cfg, this, spike_id, halted, logfile.get(), cout)),
                                                                                         dm(std::make_unique<debug_module_t>(this, dm_config)),
                                                                                         jtag_dtm(std::make_unique<jtag_dtm_t>(dm.get(), 0)),
                                                                                         remote_bitbang(std::make_unique<remote_bitbang_t>(rbb_port, jtag_dtm.get())), bitbang_delay(bitbang_delay), debug_component(name()) {

    harts[spike_id] = proc.get(); ///TO VERIFY CHECK BUT MY GUESS THIS IS TO ACCESS HART_IDs FOR SPIKE SIMULATOR

    proc->set_debug(true);
    proc->enable_log_commits();
    proc->set_histogram(false);

    SC_THREAD(spike_thread);
    SC_THREAD(bitbang_thread);

    local_bus->add_device(RBBSPIKE_DEBUG_START, dm.get());
  }

  ~rbbspike_module() {
    LOG_DBG(sc_time_stamp() << ", class destructor called");
  }

private:
  static constexpr int RBBSPIKE_DEBUG_START = 0x0;

  bool keep_sim = true;
  int spike_id;
  int& running_threads_counter;

  sc_time bitbang_delay;

  //We reuse the same tlm_generic_payload to pass data over initiator socket
  std::unique_ptr<tlm::tlm_generic_payload> trans;
  sc_time delay = SC_ZERO_TIME;

enum class tlm_nexthop {
    LOCAL_BUS,
    UNCORE
};

  // components
  const cfg_t* const cfg;
  std::unique_ptr<processor_t> proc; // TODO check
  std::map<size_t, processor_t*> harts;
  std::unique_ptr<bus_t> local_bus;  //This bus is needed for the hart to communicate with the debug_module locally.

  void spike_thread();
  void bitbang_thread();

  //start of pure virtual functions from simif_t
  char* addr_to_mem(reg_t paddr) override;
  bool mmio_load(reg_t paddr, size_t len, uint8_t* bytes) override;
  bool mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) override;
  void proc_reset(unsigned id) override;
  const cfg_t& get_cfg() const override;
  const std::map<size_t, processor_t*>& get_harts() const override;
  const char* get_symbol(uint64_t paddr) override;
  //end of pure virtual functions from simif_t

  //For GDB debugging
  //communication chain is: GDB <---tcp---> OpenOCD <---tcp---> remote_bitbang(spike) >--ptr--> jtag_dtm(spike) >--ptr--> debug_module(spike) >--ptr--> simif_t(spike) >--ptr--> processor_t(spike)
  std::unique_ptr<debug_module_t> dm;
  std::unique_ptr<jtag_dtm_t> jtag_dtm;
  std::unique_ptr<remote_bitbang_t> remote_bitbang;

  inline tlm_nexthop tlm_address_decode(reg_t paddr);

};

#endif

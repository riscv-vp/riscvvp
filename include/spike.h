#ifndef SPIKE_H
#define SPIKE_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "riscv/simif.h"      // needed for base class
#include "riscv/cfg.h"        // needed for cfg_t* in header
#include "riscv/processor.h"  // needed for processor_t* in header
#include "riscv/log_file.h"   // for log_file_t
#include "riscv/mmu.h"
#include "riscv/devices.h"
#include "dbg_component.h" // needed for debug_component class

struct spike_module: sc_module, simif_t, debug_component {

//Note that only Constructor, Destructor and TLM2.0 socket(s) are PUBLIC everything else is PRIVATE

  tlm_utils::simple_initiator_socket<spike_module> initiator_socket;

  SC_CTOR(spike_module, int spike_id, int& running_threads_counter, const cfg_t* cfg, log_file_t& logfile) : spike_id(spike_id), running_threads_counter(running_threads_counter), initiator_socket("initiator_socket"), trans(std::make_unique<tlm::tlm_generic_payload>()), cfg(cfg),
                                                                                         proc(std::make_unique<processor_t>(cfg->isa, cfg->priv, cfg, this, spike_id, false, logfile.get(), cout)), debug_component(name()) {
    harts[spike_id] = proc.get(); ///TO VERIFY CHECK BUT MY GUESS THIS IS TO ACCESS HART_IDs FOR SPIKE SIMULATOR

    proc->set_debug(true);
    proc->enable_log_commits();
    proc->set_histogram(false);

    SC_THREAD(spike_thread);
  }

  ~spike_module() {
    LOG_DBG(sc_time_stamp() << ", destructor called");
  }

private:
  bool keep_sim = true;
  int spike_id;
  int& running_threads_counter;

  //We reuse the same tlm_generic_payload to pass data over initiator socket
  std::unique_ptr<tlm::tlm_generic_payload> trans;
  sc_time delay = SC_ZERO_TIME;

  // components
  const cfg_t* const cfg;
  std::unique_ptr<processor_t> proc; // TODO check
  std::map<size_t, processor_t*> harts;

  void spike_thread();

  //start of pure virtual functions from simif_t
  char* addr_to_mem(reg_t paddr) override;
  bool mmio_load(reg_t paddr, size_t len, uint8_t* bytes) override;
  bool mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) override;
  void proc_reset(unsigned id) override;
  const cfg_t& get_cfg() const override;
  const std::map<size_t, processor_t*>& get_harts() const override;
  const char* get_symbol(uint64_t paddr) override;
  //end of pure virtual functions from simif_t

};

#endif

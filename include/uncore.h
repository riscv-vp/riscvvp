#ifndef UNCORE_H
#define UNCORE_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "riscv/cfg.h"        // needed for cfg_t* in header
#include "riscv/mmu.h"
#include "riscv/devices.h"
#include "dbg_component.h"

struct ucfg_t {               //uncore config struct
  reg_t default_rstvec;
  reg_t spike_ram_memory_address;
  reg_t extram_start_address;
  reg_t extram_end_address;
};


struct uncore_module: sc_module, debug_component {

//Note that only Constructor, Destructor, TLM2.0 socket(s) and callback(s) are PUBLIC everything else is PRIVATE

  tlm_utils::simple_target_socket<uncore_module> target_socket0;
  tlm_utils::simple_target_socket<uncore_module> target_socket1;
  tlm_utils::simple_initiator_socket<uncore_module> initiator_socket;

  SC_CTOR(uncore_module, const cfg_t* cfg, const ucfg_t* ucfg, const char* fw_filename) : cfg(cfg), ucfg(ucfg), target_socket0("target_socket0"), target_socket1("target_socket1"), spike_bus(std::make_unique<bus_t>(nullptr)), fw_filename(fw_filename), debug_component(name()) {
    target_socket0.register_b_transport(this, &uncore_module::b_transport);
    target_socket1.register_b_transport(this, &uncore_module::b_transport);
    setup();
  }

  ~uncore_module() {
    LOG_DBG(sc_time_stamp() << ", destructor called");
  }

  void b_transport (tlm::tlm_generic_payload& trans, sc_time& delay);

private:

enum class tlm_nexthop {
    SPIKE_BUS,
    EXTRAM,
    UNKNOWN_DEVICE
};

  const cfg_t* const cfg;
  const ucfg_t* const ucfg;
  std::unique_ptr<bus_t> spike_bus;
  const char* fw_filename;           //Firmware to be loaded into RAM from where Spike will fetch instructions

  //we use shared_ptr for devices because in Spike it is common to use shared_ptrs for devices, dont know why
  std::shared_ptr<rom_device_t> spike_boot_rom;
  std::shared_ptr<mem_t> spike_ram;

  inline tlm_nexthop tlm_address_decode(reg_t paddr);

  void setup();

  void read_bin_on_memory(const char* filename, void *memory, uint64_t offset);

};

#endif

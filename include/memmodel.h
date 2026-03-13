#ifndef MEMMODEL_H
#define MEMMODEL_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "riscv/decode.h"        //required for reg_t type
#include "dbg_component.h"

struct memmodel_module: sc_module, debug_component {

//Note that only Constructor, Destructor, TLM2.0 socket(s) and callback(s) are PUBLIC everything else is PRIVATE

  tlm_utils::simple_target_socket<memmodel_module> target_socket;
  std::map<reg_t, uint8_t> sparse_array;

  SC_CTOR(memmodel_module) : target_socket("target_socket"), sparse_array(), debug_component(name()) {
    target_socket.register_b_transport(this, &memmodel_module::b_transport);
  }

  ~memmodel_module() {
    LOG_DBG(sc_time_stamp() << ", destructor called");
  }


  void b_transport (tlm::tlm_generic_payload& trans, sc_time& delay);

private:

  const int MODEL_MAX_RAM_ADDRESS = 65535;

};

#endif

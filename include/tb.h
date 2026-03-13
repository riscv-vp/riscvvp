#ifndef TB_H
#define TB_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "riscv/decode.h"        //required for reg_t type
#include "rtlcontrol_tlm_extension.h"
#include "dbg_component.h" // needed for debug_component class

struct tb_module: sc_module, debug_component {

//Note that only Constructor, Destructor and TLM2.0 socket(s) are PUBLIC everything else is PRIVATE

  tlm_utils::simple_initiator_socket<tb_module> initiator_socket0;
  tlm_utils::simple_initiator_socket<tb_module> initiator_socket1;

  SC_CTOR(tb_module, int& running_threads_counter) : running_threads_counter(running_threads_counter), initiator_socket0("initiator_socket0"), initiator_socket1("initiator_socket1"), trans(std::make_unique<tlm::tlm_generic_payload>()), data_array0(1), data_array1(1), debug_component(name()) {
    SC_THREAD(tb_thread);
  }

  ~tb_module() {
    LOG_DBG(sc_time_stamp() << ", destructor called");
  }

private:
  bool keep_sim = true;
  int& running_threads_counter;


  //We reuse the same tlm_generic_payload to pass data over initiator socket
  std::unique_ptr<tlm::tlm_generic_payload> trans;

  sc_time delay = SC_ZERO_TIME;
  std::vector<uint8_t> data_array0, data_array1;

  void tb_thread();

  void fillVecWithRandom(std::vector<uint8_t>&, uint8_t, uint8_t);

  bool drivertl(std::vector<uint8_t>&, reg_t, bool);                          //DriveRTL without delays (wrapper)
  bool drivertl(std::vector<uint8_t>&, reg_t, bool, sc_time, sc_time);        //DriveRTL with delays (wrapper)
  bool drivertl(std::vector<uint8_t>&, reg_t, bool, bool, sc_time, sc_time);  //DriveRTL master, usually not callable directly but from a wrapper

  bool drivemodel(std::vector<uint8_t>&, reg_t, bool);                        //DriveModel

};

#endif

#ifndef MEMRTL_H
#define MEMRTL_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "riscv/decode.h"        //required for reg_t type
#include <verilated_vcd_sc.h>
#include "Vtop_x.h"
#include "dbg_component.h"

struct memrtl_module: sc_module, debug_component {

//Note that only Constructor, Destructor, TLM2.0 socket(s), callback(s), "set_trace" and "start_of_simulation" are PUBLIC everything else is PRIVATE

  tlm_utils::simple_target_socket<memrtl_module> target_socket;

  SC_CTOR(memrtl_module, sc_time clk_period, sc_time rst_timeout) : clk_ch("clk", clk_period, 0.5, SC_ZERO_TIME, true), rst_timeout(rst_timeout), req_PIPE(4), resp_PIPE(4), target_socket("target_socket"), veritopx(std::make_unique<Vtop_x>("veritopx")), debug_component(name()) {
    target_socket.register_b_transport(this, &memrtl_module::b_transport);

    clk_port(clk_ch);     // Bind the internal clock channel to the port

    // Attach Vtop's signals to this upper model
    veritopx->clk(clk_port);
    veritopx->rst_n(rstn);
    veritopx->up_valid(queue2pins_down_valid);
    veritopx->up_data(queue2pins_down_data);
    veritopx->up_ready(queue2pins_down_ready);
    veritopx->down_valid(pins2queue_up_valid);
    veritopx->down_data(pins2queue_up_data);
    veritopx->down_ready(pins2queue_up_ready);

    SC_THREAD(reset_thread);
    SC_METHOD(queue2pins_method);
    sensitive << clk_port.pos();
    SC_METHOD(pins2queue_method);
    sensitive << clk_port.pos();
  }

  ~memrtl_module() {
    std::cout << sc_time_stamp() << ": Cleanup: destructor" << std::endl;
    veritopx->final();
  }

  void start_of_simulation();

#if VM_TRACE
  //Set_trace cannot be implemented in the constructor because we must start start simulation before enabling waves
  void set_trace(VerilatedVcdSc* tfp, int levels);
#endif

  void b_transport (tlm::tlm_generic_payload& trans, sc_time& delay);

private:

  const int RTL_MAX_RAM_ADDRESS = 65535;

  sc_clock clk_ch;
  sc_in_clk clk_port;         //we use sc_in_clk port because we cannot use sensitivity "sensitive << clk_port.pos()" directly on a channel

  sc_mutex rtl_access;

  sc_fifo<std::uint32_t> req_PIPE;
  sc_fifo<std::uint32_t> resp_PIPE;

  sc_time rst_timeout;

  //Used by pins_to_queue_method
  sc_signal<std::uint32_t> REG;
  sc_signal<bool> state;
  enum BufState { BYPASS=false, SKID=true };

  //Used by queue_to_pins_method
  std::uint32_t ReadValueFromReqPIPE;

  // Using unique_ptr is similar to "Vtop* veritop = new Vtop" then deleting at end
  const std::unique_ptr<Vtop_x> veritopx;

  // Define interconnect
  sc_signal<bool> rstn;
  sc_signal<bool> queue2pins_down_valid;
  sc_signal<std::uint32_t> queue2pins_down_data;
  sc_signal<bool> queue2pins_down_ready;
  sc_signal<bool> pins2queue_up_valid;
  sc_signal<std::uint32_t> pins2queue_up_data;
  sc_signal<bool> pins2queue_up_ready;

  void reset_thread();
  void queue2pins_method();
  void pins2queue_method();

};

#endif

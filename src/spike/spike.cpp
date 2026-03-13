#include <string>
#include "systemc.h"
#include "spike.h"

void spike_module::spike_thread() {

  while (keep_sim) {
      proc->step(1);                //Call spike step
  }

  LOG_DBG(sc_time_stamp() << ", Spike finished.");
  running_threads_counter--;
}

//// start of overriden pure virtual functions from simif_t ////
char* spike_module::addr_to_mem(reg_t paddr) {
    return nullptr;
}

bool spike_module::mmio_load(reg_t paddr, size_t len, uint8_t* bytes) {
    LOG_DBG(sc_time_stamp() << ", mmio_load called for address 0x" << hex << paddr << " (len=" << dec << len << ") via tlm2");
    delay = SC_ZERO_TIME;
    trans->set_command(tlm::TLM_READ_COMMAND);
    trans->set_address(paddr);
    trans->set_data_ptr(bytes);
    trans->set_data_length(len);
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    initiator_socket->b_transport(*trans, delay);
    wait(delay);

    switch (trans->get_response_status()) {
        case tlm::TLM_OK_RESPONSE: {
            return true;
        }
        case tlm::TLM_GENERIC_ERROR_RESPONSE: {
            LOG_DBG(sc_time_stamp() << ", mmio_load TLM_GENERIC_ERROR_RESPONSE for address 0x" << hex << paddr << " (len=" << dec << len << ") via tlm2");
            keep_sim = false;
            return false;
        }
        default: {
            SC_REPORT_ERROR("Spike_Module", "Response_status ERROR.");
            return false;
        }
    }
}

bool spike_module::mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) {
    LOG_DBG(sc_time_stamp() << ", mmio_store called for address 0x" << hex << paddr << " (len=" << dec << len << ") via tlm2");
    delay = SC_ZERO_TIME;
    trans->set_command(tlm::TLM_WRITE_COMMAND);
    trans->set_address(paddr);
    trans->set_data_ptr(const_cast<unsigned char*>(bytes));
    trans->set_data_length(len);
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    initiator_socket->b_transport(*trans, delay);
    wait(delay);

    switch (trans->get_response_status()) {
        case tlm::TLM_OK_RESPONSE: {
            return true;
        }
        case tlm::TLM_GENERIC_ERROR_RESPONSE: {
            LOG_DBG(sc_time_stamp() << ", mmio_store TLM_GENERIC_ERROR_RESPONSE for address 0x" << hex << paddr << " (len=" << dec << len << ") via tlm2");
            keep_sim = false;
            return false;
        }
        default: {
            SC_REPORT_ERROR("Spike_Module", "Response_status ERROR.");
            return false;
        }
    }
}

void spike_module::proc_reset(unsigned id) {
    // This is a placeholder implementation
}

const cfg_t& spike_module::get_cfg() const {
    // Return the cfg member
    return *cfg;
}

const std::map<size_t, processor_t*>& spike_module::get_harts() const {
    return harts;
}

const char* spike_module::get_symbol(uint64_t paddr) {
    // This is a placeholder implementation
    return nullptr;
}
//// end of overriden pure virtual functions from simif_t ////

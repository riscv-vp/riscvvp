#include <string>
#include "systemc.h"
#include "rbbspike.h"

rbbspike_module::tlm_nexthop rbbspike_module::tlm_address_decode(reg_t paddr) {
    auto desc = local_bus->find_device(paddr >> PGSHIFT << PGSHIFT, PGSIZE);
    if (desc.second) {
        return tlm_nexthop::LOCAL_BUS;
    } else {
        return tlm_nexthop::UNCORE;
    }
}


void rbbspike_module::spike_thread() {

  while (keep_sim) {
      proc->step(1);                //Call spike step
      wait(2000, SC_NS);               //Replace with quantum keeper
  }

  LOG_DBG(sc_time_stamp() << ", RbbSpike thread finished.");
  running_threads_counter--;
}

void rbbspike_module::bitbang_thread() {

  while (keep_sim) {
      remote_bitbang->tick();
      wait(bitbang_delay);
  }

  LOG_DBG(sc_time_stamp() << ", BitBang thread finished.");
  running_threads_counter--;
}

//// start of overriden pure virtual functions from simif_t ////
char* rbbspike_module::addr_to_mem(reg_t paddr) {
    return nullptr;
}

bool rbbspike_module::mmio_load(reg_t paddr, size_t len, uint8_t* bytes) {
    switch (tlm_address_decode(paddr)) {
        case tlm_nexthop::LOCAL_BUS: {
            LOG_DBG(sc_time_stamp() << ", LOAD to LOCAL_BUS called for address 0x" << hex << paddr << " (len=" << dec << len << ")");
            return local_bus->load(paddr, len, bytes);
            break;
        }
        case tlm_nexthop::UNCORE: {
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
                    SC_REPORT_ERROR("RbbSpike_Module", "Response_status ERROR.");
                    return false;
                }
            }
            break;
        }
        default: {
            SC_REPORT_ERROR("RbbSPike_Module", "tlm_destination ERROR.");
            return false;
        }
    }
}

bool rbbspike_module::mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) {
    switch (tlm_address_decode(paddr)) {
        case tlm_nexthop::LOCAL_BUS: {
            LOG_DBG(sc_time_stamp() << ", STORE to LOCAL_BUS called for address 0x" << hex << paddr << " (len=" << dec << len << ")");
            return local_bus->store(paddr, len, bytes);
            break;
        }
        case tlm_nexthop::UNCORE: {
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
                    SC_REPORT_ERROR("RbbSpike_Module", "Response_status ERROR.");
                    return false;
                }
            }
            break;
        }
        default: {
            SC_REPORT_ERROR("RbbSPike_Module", "tlm_destination ERROR.");
            return false;
        }
    }
}

void rbbspike_module::proc_reset(unsigned id) {
    // This is a placeholder implementation
}

const cfg_t& rbbspike_module::get_cfg() const {
    // Return the cfg member
    return *cfg;
}

const std::map<size_t, processor_t*>& rbbspike_module::get_harts() const {
    return harts;
}

const char* rbbspike_module::get_symbol(uint64_t paddr) {
    // This is a placeholder implementation
    return nullptr;
}
//// end of overriden pure virtual functions from simif_t ////

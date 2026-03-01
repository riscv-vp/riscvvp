#include "systemc.h"
#include "memmodel.h"

void memmodel_module::b_transport (tlm::tlm_generic_payload& trans, sc_time& delay) {

        const tlm::tlm_command cmd = trans.get_command();
        const reg_t addr = trans.get_address();
        const size_t length = trans.get_data_length() / sizeof(uint8_t);
        uint8_t* data = reinterpret_cast<uint8_t*>(trans.get_data_ptr());

        if (addr + length - 1 > MODEL_MAX_RAM_ADDRESS ) {
            trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
            return;
        } else {
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        switch (cmd) {
            case tlm::TLM_READ_COMMAND: {
                LOG_DBG(sc_time_stamp() << ", TLM_LOAD called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                delay = delay + SC_ZERO_TIME;     //We dont increment delay because this is a reference MODEL that shall no consume time
                for (int i=0; i<length; i++) {
                    data[i] = sparse_array[i];
                }
                break;
            }
            case tlm::TLM_WRITE_COMMAND: {
                LOG_DBG(sc_time_stamp() << ", TLM_STORE called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                delay = delay + SC_ZERO_TIME;     //We dont increment delay because this is a reference MODEL that shall no consume time
                for (int i=0; i<length; i++) {
                    sparse_array[i] = data[i];
                }
                break;
            }
            default: {
                SC_REPORT_ERROR("Memmodel_Module", "Command ERROR.");
            }
        }
}

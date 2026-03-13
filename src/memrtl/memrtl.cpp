#include "systemc.h"
#include "memrtl.h"
#include "rtlcontrol_tlm_extension.h"

void memrtl_module::b_transport (tlm::tlm_generic_payload& trans, sc_time& delay) {

        //We shall declare internal b_transport variables inside b_transport because this way they will be "on the call stack". If we declare them as class memebers this will break reentrancy of b_transport.
        int operation = 0;

        std::uint32_t req_vec, resp_vec, last;
        sc_bv<32> bv, bv_vec;

        const tlm::tlm_command cmd = trans.get_command();
        const reg_t addr = trans.get_address();
        const size_t length = trans.get_data_length() / sizeof(uint8_t);
        uint8_t* data = reinterpret_cast<uint8_t*>(trans.get_data_ptr());

        if (addr + length - 1 > RTL_MAX_RAM_ADDRESS ) {
            trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
            return;
        } else {
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        switch (cmd) {
            case tlm::TLM_READ_COMMAND: {
                LOG_DBG(sc_time_stamp() << ", TLM_LOAD called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                operation = 0;
                delay = delay + SC_ZERO_TIME;     //We dont increment delay because RTL consumes time and Initiator thread will naturally block
                break;
            }
            case tlm::TLM_WRITE_COMMAND: {
                LOG_DBG(sc_time_stamp() << ", TLM_STORE called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                operation = 1;
                delay = delay + SC_ZERO_TIME;     //We dont increment delay because RTL consumes time and Initiator thread will naturally block
                break;
            }
            default: {
                SC_REPORT_ERROR("Uncore_Module", "Command ERROR.");
            }
        }


        rtlcontrol_tlm_extension* rtl_ctrl_ext = nullptr;
        trans.get_extension(rtl_ctrl_ext);

        //Because this "b_transport" callback is reentrant several sc_threads may compete for the same req_PIPE and resp_PIPE queues. This means one thread can put data in req_PIPE but another thread grab response for that request from resp_PIPE.
        //To avoid this we can put guard in the form of "sc_mutex" that will guarantee that another thread will not step in before the first sc_thread (that grabbed the mutex) receive the response (or respnses in case of burst) from the resp_PIPE.

        //Another way to address this problem is to use virtual channels (dedicated req_PIPE and resp_PIPE queues per sc_thread), each VC have Req and Resp queues, each bit that is send down RTL pipeline will be marked with
        //corresponding master ID that is preserved in RTL block so it will come back to the correct Resp channel. And yes proper arbiter SC_THREAD shall be used with implemented arbitration logic
        //(per burst or per beat). We also need to use TLM_GENERIC_PAYLOAD extention that carries CPU_ID and mapped to the bus ID (for the bus ID unused bits can be utilized).

        rtl_access.lock();
        LOG_DBG(sc_time_stamp() << ", rtl_access locked");

        if (!rtl_ctrl_ext) {

            LOG_DBG(sc_time_stamp() << ", no rtl_ctrl_exntension detected; process burst using BLOCKING writes and reads.");

            //Burst processing loop
            for (int i=0; i<length; i++) {
                bv.range(31, 31) = operation;                                  //SET REQ OPERATION
                if (i == (length - 1)) {                                       //SET REQ LAST
                    bv.range(30, 30) = 1;
                } else {
                    bv.range(30, 30) = 0;
                }
                bv.range(29, 24) = 0;                                          //SET REQ UNUSED
                bv.range(23, 8) = addr + i;                                    //SET REQ ADDR
                bv.range(7, 0) =  data[i];                                     //SET REQ DATA
                req_vec = bv.to_uint();

                req_PIPE.write(req_vec);
                resp_vec = resp_PIPE.read();

                bv_vec = resp_vec;
                last = bv_vec.range(30, 30).to_uint();                         //GET RESP LAST
                data[i] = bv_vec.range(7, 0).to_uint();                        //GET RESP DATA

                //Lets do checks on RESP
                if ((i == (length - 1)) && (last == 0)) {
                    LOG_DBG(sc_time_stamp() << ", ==================== !!!!!!! ERROR !!!!!!! =================== FINAL burst beat has LAST set to 0.");
                    trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
                }
                if ((i != (length - 1)) && (last == 1)) {
                    LOG_DBG(sc_time_stamp() << ", ==================== !!!!!!! ERROR !!!!!!! =================== Not_FINAL burst beat has LAST set to 1.");
                    trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
                }
            }
        } else {

            LOG_DBG(sc_time_stamp() << ", rtl_ctrl_exntension detected; REQ_delay = " << sc_time(rtl_ctrl_ext->req_delay) << ", RESP_delay = " << sc_time(rtl_ctrl_ext->resp_delay) <<  "; process burst using NON-BLOCKING writes and reads.");

            int curr_req = 0;
            int curr_resp = 0;

            bool req_data_set = false;

            sc_time nextreqtime = sc_time_stamp() + rtl_ctrl_ext->req_delay;
            sc_time nextresptime = sc_time_stamp() + rtl_ctrl_ext->resp_delay;

            //Burst processing loop
            while (curr_resp<length) {        //continue until we process all responses

                //Determine the next event time
                sc_time now = sc_time_stamp();
                sc_time next = std::min(nextreqtime, nextresptime);
                //Wait until the next scheduled time
                wait(next - now);
                //After wait we have to check what time is now
                now = sc_time_stamp();

                //If current time is larger that next request time then we process request
                if (now >= nextreqtime) {
                    LOG_DBG(sc_time_stamp() << ", ____time_to_try_to_process_REQ____");
                    if (curr_req<length) {
                        if (req_data_set == false) {
                            bv.range(31, 31) = operation;                      //SET REQ OPERATION
                            if (curr_req == (length - 1)) {                    //SET REQ LAST
                                bv.range(30, 30) = 1;
                            } else {
                                bv.range(30, 30) = 0;
                            }
                            bv.range(29, 24) = 0;                              //SET REQ UNUSED
                            bv.range(23, 8) = addr + curr_req;                 //SET REQ ADDR
                            bv.range(7, 0) =  data[curr_req];                  //SET REQ DATA
                            req_vec = bv.to_uint();
                            req_data_set = true;
                        }

                        if (req_PIPE.nb_write(req_vec)) {
                            LOG_DBG(sc_time_stamp() << ", REQ pushed to PEQ_PIPE" << "; len: " << length << "; cur_req: " << curr_req);
                            curr_req++;   //if req write successfull increment curr_req
                            req_data_set = false;
                        }
                    }
                    //we increment next nextrequest time
                    nextreqtime += rtl_ctrl_ext->req_delay;
                }

                //If current time is larger that next response time then we process response
                if (now >= nextresptime) {
                    LOG_DBG(sc_time_stamp() << ", ____time_to_try_to_process_RESP____");
                    if (resp_PIPE.nb_read(resp_vec)) {
                        LOG_DBG(sc_time_stamp() << ", RESP popped from RESP_PIPE" << "; len: " << length << "; cur_resp: " << curr_resp);
                        bv_vec = resp_vec;
                        last = bv_vec.range(30, 30).to_uint();                 //GET RESP LAST
                        data[curr_resp] = bv_vec.range(7, 0).to_uint();        //GET RESP DATA

                        //Lets do checks on RESP
                        if ((curr_resp == (length - 1)) && (last == 0)) {
                            LOG_DBG(sc_time_stamp() << ", ==================== !!!!!!! ERROR !!!!!!! =================== FINAL burst beat has LAST set to 0.");
                            trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
                        }
                        if ((curr_resp != (length - 1)) && (last == 1)) {
                            LOG_DBG(sc_time_stamp() << ", ==================== !!!!!!! ERROR !!!!!!! =================== Not_FINAL burst beat has LAST set to 1.");
                            trans.set_response_status (tlm::TLM_GENERIC_ERROR_RESPONSE);
                        }
                        curr_resp++;      //if resp read successfull increment curr_resp
                    }
                    //we increment next nextresponse time
                    nextresptime += rtl_ctrl_ext->resp_delay;
                }
            }
        }

        rtl_access.unlock();  //we return mutex once burst is processed
        LOG_DBG(sc_time_stamp() << ", rtl_access unlocked");
}


void memrtl_module::queue2pins_method() {
    if (rstn.read() == false) {
       queue2pins_down_valid.write(false);
    }
    else
    {
      //we are comparing pre-clock value here.
      //pipeline drives if ~down_valid or down_ready
      if (queue2pins_down_valid.read() == false || queue2pins_down_ready.read() == true) {
        if (req_PIPE.nb_read(ReadValueFromReqPIPE) == true) {
          queue2pins_down_valid.write(true);
          queue2pins_down_data.write(ReadValueFromReqPIPE);
        }
        else
        {
          queue2pins_down_valid.write(false);
          //we only modify value of valid and don't modify value of data signal
        }
      }
    }
}


//NOTE: We use "skid-buffer" concept here because "skid_buffer" sets "up_ready" signal based on available space in the "REG" expansion register.
//Alternative approach could be to check for available space in the "down_PIPE" which is "sc_fifo" and set "up_ready" signal based on the available space in that "down_PIPE" fifo.
void memrtl_module::pins2queue_method() {
    if (rstn.read() == false) {
       state.write(BufState::BYPASS);
       pins2queue_up_ready.write(true);
    }
    else
    {
      //we are comparing pre-clock value here.
      //Write data into expansion REGs if handshake happens but no free place in down_PIPE fifo
      if (state.read() == BufState::BYPASS) {
          if (pins2queue_up_valid.read() == true && pins2queue_up_ready.read() == true) {
            if (resp_PIPE.nb_write(pins2queue_up_data.read()) == false) {
                REG.write(pins2queue_up_data.read());
                state.write(BufState::SKID);
                pins2queue_up_ready.write(false);
            }
          }
      }
      else if (state.read() == BufState::SKID) {
          if (resp_PIPE.nb_write(REG) == true) {
             state.write(BufState::BYPASS);
             pins2queue_up_ready.write(true);
          }
      }
      else {}
    }
}

#if VM_TRACE
void memrtl_module::set_trace(VerilatedVcdSc* tfp, int levels) {
    veritop0->trace(tfp, levels);
}
#endif

void memrtl_module::start_of_simulation() {
    rstn.write(false);
    LOG_DBG(sc_time_stamp() << ", Initial (RESET_ON)");
}

void memrtl_module::reset_thread() {
    wait(rst_timeout);
    rstn.write(true);
    LOG_DBG(sc_time_stamp() << ", RESET_OFF");
}

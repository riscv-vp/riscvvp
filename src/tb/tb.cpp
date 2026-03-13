#include <string>
#include "systemc.h"
#include "tb.h"

void tb_module::tb_thread() {

  bool sim_test_ok = true;

  //Do 22256 bytes WRITE
  data_array0.resize(22256);
  fillVecWithRandom(data_array0, 0, 254);
  data_array1 = data_array0;

  drivertl(data_array0, 0, true);
  drivemodel(data_array1, 0, true);

  //Do 22256 bytes READ no delays
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(22256);
  data_array1.resize(22256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false);
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }


  //Do 22256 bytes READ 1ns/1ns delay
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(22256);
  data_array1.resize(22256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false, sc_time(1, SC_NS), sc_time(1, SC_NS));
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }


  //Do 22256 bytes READ 100ns/1ns delay
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(22256);
  data_array1.resize(22256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false, sc_time(100, SC_NS), sc_time(1, SC_NS));
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }

  //Do 22256 bytes READ 1ns/100ns delay
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(2256);
  data_array1.resize(2256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false, sc_time(1, SC_NS), sc_time(100, SC_NS));
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }

  //Do 22256 bytes READ 100ns/100ns delay
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(2256);
  data_array1.resize(2256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false, sc_time(100, SC_NS), sc_time(100, SC_NS));
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }

  //Do 22256 bytes READ 50ns/50ns delay
  data_array0.clear();
  data_array1.clear();

  data_array0.resize(2256);
  data_array1.resize(2256);

  fillVecWithRandom(data_array0, 0, 254);
  fillVecWithRandom(data_array1, 0, 254);

  drivertl(data_array0, 0, false, sc_time(50, SC_NS), sc_time(50, SC_NS));
  drivemodel(data_array1, 0, false);

  if (data_array1 == data_array0) {
      LOG_DBG(sc_time_stamp() << ", VECTORS MATCH!!!!!");
  } else {
      LOG_DBG(sc_time_stamp() << ", VECTORS DONT MATCH!!!!!");
      sim_test_ok = false;
  }



  if (sim_test_ok) {
      LOG_DBG(sc_time_stamp() << ", SIMULATION_PASS  :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)   :-)  SIMULATION_PASS");
  } else {
      LOG_DBG(sc_time_stamp() << ", :-( SIMULATION_FAIL :-(");
  };
  LOG_DBG(sc_time_stamp() << ", TB finished.");
  running_threads_counter--;

}


void tb_module::fillVecWithRandom(std::vector<uint8_t>& data_vec, uint8_t minVal, uint8_t maxVal) {
    for (auto& x : data_vec) {
        x = minVal + std::rand() % (maxVal - minVal + 1);
    }
}


bool tb_module::drivertl(std::vector<uint8_t>& data_vec, reg_t address, bool store) {
    return drivertl(data_vec, address, store, false, sc_time(SC_ZERO_TIME), sc_time(SC_ZERO_TIME));
}

bool tb_module::drivertl(std::vector<uint8_t>& data_vec, reg_t address, bool store, sc_time req_delay, sc_time resp_delay) {
    return drivertl(data_vec, address, store, true, req_delay, resp_delay);
}

bool tb_module::drivertl(std::vector<uint8_t>& data_vec, reg_t address, bool store, bool usedelay, sc_time req_delay, sc_time resp_delay) {

    rtlcontrol_tlm_extension* ext = nullptr;                             //Here is the recommended way of creating TLM2.0 extension dynamically and then destroying it after "b_transport" call
    if (usedelay) {
        if (req_delay == sc_time(SC_ZERO_TIME) || resp_delay == sc_time(SC_ZERO_TIME)) {
            SC_REPORT_ERROR("Tb_Module", "Zero delay provided.");
        }
        rtlcontrol_tlm_extension* ext = new rtlcontrol_tlm_extension;
        ext->req_delay = req_delay;
        ext->resp_delay = resp_delay;
        trans->set_extension(ext);
    }

    if (store) {
        LOG_DBG(sc_time_stamp() << ", drivertl, store called for RTL address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
        trans->set_command(tlm::TLM_WRITE_COMMAND);
    } else {
        LOG_DBG(sc_time_stamp() << ", drivertl, load called for RTL address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
        trans->set_command(tlm::TLM_READ_COMMAND);
    }

    delay = SC_ZERO_TIME;
    trans->set_address(address);
    trans->set_data_ptr(data_vec.data());
    trans->set_data_length(data_vec.size() * sizeof(data_vec[0]));    //sizeof(data_vec[0]) only works if vector is not empty. Alternatively use sizeof(unit8_t).
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    initiator_socket0->b_transport(*trans, delay);
    wait(delay);

    if (usedelay && ext != nullptr) {
        trans->clear_extension(ext);
        delete ext;
        ext = nullptr;
    }

    switch (trans->get_response_status()) {
        case tlm::TLM_OK_RESPONSE: {
            return true;
        }
        case tlm::TLM_GENERIC_ERROR_RESPONSE: {
            LOG_DBG(sc_time_stamp() << ", drivertl, TLM_GENERIC_ERROR_RESPONSE for address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
            keep_sim = false;
            return false;
        }
        default: {
            SC_REPORT_ERROR("Tb_Module", "Response_status ERROR.");
            return false;
        }
    }
}

bool tb_module::drivemodel(std::vector<uint8_t>& data_vec, reg_t address, bool store) {

    if (store) {
        LOG_DBG(sc_time_stamp() << ", drivemodel, store called for MEM address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
        trans->set_command(tlm::TLM_WRITE_COMMAND);
    } else {
        LOG_DBG(sc_time_stamp() << ", drivemodel, load called for MEM address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
        trans->set_command(tlm::TLM_READ_COMMAND);
    }

    delay = SC_ZERO_TIME;
    trans->set_address(address);
    trans->set_data_ptr(data_vec.data());
    trans->set_data_length(data_vec.size() * sizeof(data_vec[0]));    //sizeof(data_vec[0]) only works if vector is not empty. Alternatively use sizeof(unit8_t).
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    initiator_socket1->b_transport(*trans, delay);
    wait(delay);

    switch (trans->get_response_status()) {
        case tlm::TLM_OK_RESPONSE: {
            return true;
        }
        case tlm::TLM_GENERIC_ERROR_RESPONSE: {
            LOG_DBG(sc_time_stamp() << ", drivemodel, TLM_GENERIC_ERROR_RESPONSE for address 0x" << hex << address << " (len=" << dec << data_vec.size() << ") via tlm2");
            keep_sim = false;
            return false;
        }
        default: {
            SC_REPORT_ERROR("Tb_Module", "Response_status ERROR.");
            return false;
        }
    }
}

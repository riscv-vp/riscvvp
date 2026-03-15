#include "systemc.h"
#include "uncore.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/stat.h"

uncore_module::tlm_nexthop uncore_module::tlm_address_decode(reg_t paddr) {
    auto desc = spike_bus->find_device(paddr >> PGSHIFT << PGSHIFT, PGSIZE);
    if (desc.first != 0) {
        return tlm_nexthop::SPIKE_BUS;
    } else if (ucfg->extram_start_address <= paddr && paddr <= ucfg->extram_end_address) {
        return tlm_nexthop::EXTRAM;
    } else {
        return tlm_nexthop::UNKNOWN_DEVICE;
    }
}


void uncore_module::b_transport (tlm::tlm_generic_payload& trans, sc_time& delay) {

    const tlm::tlm_command cmd = trans.get_command();
    const reg_t addr = trans.get_address();
    const size_t length = trans.get_data_length() / sizeof(uint8_t);

    switch (tlm_address_decode(addr)) {
        case tlm_nexthop::SPIKE_BUS: {
            if (trans.get_command() == tlm::TLM_READ_COMMAND) {
                LOG_DBG(sc_time_stamp() << ", TLM_LOAD to SPIKE_BUS called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                delay = delay + sc_time(10, SC_NS);
                if (spike_bus->load(trans.get_address(), trans.get_data_length(), trans.get_data_ptr())) {
                    trans.set_response_status(tlm::TLM_OK_RESPONSE);
                } else {
                    trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                }
            } else if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
                LOG_DBG(sc_time_stamp() << ", TLM_STORE to SPIKE_BUS called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
                delay = delay + sc_time(50, SC_NS);
                if (spike_bus->store(trans.get_address(), trans.get_data_length(), trans.get_data_ptr())) {
                    trans.set_response_status(tlm::TLM_OK_RESPONSE);
                } else {
                    trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                }
            } else {
                SC_REPORT_ERROR("Uncore_Module", "Command ERROR.");
            }
            break;
        }
        case tlm_nexthop::EXTRAM: {
            LOG_DBG(sc_time_stamp() << ", TLM to EXTRAM is called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
            delay = delay + sc_time(7, SC_NS);                       //Lets add some interconnect delay
            trans.set_address(addr - ucfg->extram_start_address);    //We modify address of the transaction so that EXTRAM will see address with base address to be zero
            initiator_socket->b_transport(trans, delay);
            break;
        }
        case tlm_nexthop::UNKNOWN_DEVICE: {
            LOG_DBG(sc_time_stamp() << ", TLM to UNKNOWN_DEVICE is called for address 0x" << hex << addr << " (len=" << dec << length << ") via tlm2");
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            break;
        }
        default: {
            SC_REPORT_ERROR("Uncore_Module", "tlm_destination ERROR.");
        }
    }
}


void uncore_module::setup() {
  const int reset_vec_size = 8;

  assert(cfg->start_pc.has_value());            //make sure that start_pc is provided
  reg_t start_pc = cfg->start_pc.value();

  //The following code is for 64bit RiscV only as it uses "ld t0,24(t0)" not "lw t0,24(t0)"
  //We no longer have ability here to check if proc->get_xlen() == 64 or 32;
  //so we may need to check is cfg.isa is 64 bit.

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    0x0182b283u,                                // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };

  assert(cfg->endianness == endianness_little); //only little endian supported
  for (int i = 0; i < reset_vec_size; i++)
      reset_vec[i] = to_le(reset_vec[i]);

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  spike_boot_rom = std::make_shared<rom_device_t>(rom);

  spike_ram = std::make_shared<mem_t>(16384);
  read_bin_on_memory(fw_filename, spike_ram.get(), 0);

  spike_bus->add_device(ucfg->default_rstvec, spike_boot_rom.get());
  spike_bus->add_device(ucfg->spike_ram_memory_address, spike_ram.get());
}

void uncore_module::read_bin_on_memory(const char* filename, void *memory, uint64_t offset) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        SC_REPORT_ERROR("Uncore_Module", "ERROR: Binary file cannot be opened.");
    }
    struct stat s;
    if (fstat(fd, &s) < 0)
        abort();
    auto size = static_cast<size_t>(s.st_size);

    char* buf = (char*)mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(buf != MAP_FAILED);
    close(fd);

    for (unsigned i = 0; i < size; i++) {
        *(reinterpret_cast<mem_t*>(memory)->contents(i + offset)) = buf[i];
    }
    munmap(buf, size);
}

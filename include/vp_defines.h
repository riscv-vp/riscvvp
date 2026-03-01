//We try not to rely on Spike "platform.h" file. So we have most of the important spike defines specified in this file.

#ifndef VP_DEFINES_H
#define VP_DEFINES_H

#define VP_DEFAULT_RSTVEC                    0x00001000
#define VP_START_PC                          0x80000000
#define VP_SPIKE_RAM_MEMORY_ADDRESS          0x80000000
#define VP_EXTRAM_START_ADDRESS              0x81000000
#define VP_EXTRAM_END_ADDRESS                0x81FFFFFF

#endif

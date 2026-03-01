OUTPUT_ARCH("riscv")

ENTRY(_start)

EXTRAM_BASE = 0x81000000;
PROVIDE(__extram_base = EXTRAM_BASE);

UNMAPPED_BASE = 0x82000000;
PROVIDE(__unmapped_base = UNMAPPED_BASE);

SECTIONS
{
    . = 0x80000000;
    .text : ALIGN(0x1000)
    {
        *(.text)
    }

    .bss : ALIGN(0x1000)
    {
        *(.bss)
    }
}

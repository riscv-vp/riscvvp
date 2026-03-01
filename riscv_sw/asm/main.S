.section .text

.globl _start

_start:
        la t0, __extram_base    # load address of buffer into t0
        li t1, 0x12345678       # load a value into t1
        sd t1, 0(t0)            # store 64-bit value from t1 into __extram_base

        ld t2, 0(t0)            # load 64-bit value from __extram_base to t2

        la t0, __unmapped_base  # load address of buffer into t0
        li t1, 0x77777777       # load a value into t1
        sd t1, 0(t0)            # store 64-bit value from t1 into __unmapped_base to finish simulation
        1: j 1b                 # wait for termination

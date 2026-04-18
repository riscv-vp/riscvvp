#!/usr/bin/env bash

gnome-terminal -- bash -c "../build/two_iss_rbb_with_memmodel; echo \"Press Enter to close\"; read"
sleep 2
gnome-terminal -- bash -c "$RISCV_HOME/../riscv-openocd-dist/bin/openocd -f ./openocd_spike.cfg; echo \"Press Enter to close\"; read"
gnome-terminal -- bash -c "riscv64-unknown-elf-gdb; echo \"Press Enter to close\"; read"

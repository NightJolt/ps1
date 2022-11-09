#include "cpu.h"

namespace ps1 {
    constexpr register_t register_garbage_value = 0xDEADBEEF; // magic value for debugging
    constexpr register_t bios_main_func_addr = 0xBFC00000; // BIOS main function address
}


ps1::cpu_t::cpu_t() {
    for (int i = 0; i < 32; i++) {
        regs[i] = register_garbage_value; // initialize registers to garbage value
    }

    regs[0] = 0; // $zero is always zero

    pc = bios_main_func_addr; // start at BIOS main function
}
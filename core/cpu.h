#pragma once

#include "defs.h"

namespace ps1 {
    typedef uint32_t cpu_reg_t;

    // 32-bit MIPS R3000A processor.
    class cpu_t {
    public:
        cpu_t();

    private:
        cpu_reg_t regs[32]; // general purpose registers
        cpu_reg_t pc; // program counter
    };
}
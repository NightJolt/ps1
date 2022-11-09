#pragma once

#include "defs.h"

namespace ps1 {
    typedef uint32_t register_t;

    // 32-bit MIPS R3000A processor.
    class cpu_t {
    public:
        cpu_t();

    private:
        register_t regs[32]; // general purpose registers
        register_t pc; // program counter
    };
}
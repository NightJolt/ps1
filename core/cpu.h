#pragma once

#include "defs.h"

namespace ps1 {
    union cpu_instr_t {
        cpu_instr_t() = default;
        cpu_instr_t(uint32_t);

        operator uint32_t();

        uint32_t raw;

        struct {
            uint32_t funct : 6;
            uint32_t shamt : 5;
            uint32_t rd : 5;
            uint32_t rt : 5;
            uint32_t rs : 5;
            uint32_t opcode : 6;
        } a;

        struct {
            uint32_t imm : 16;
            uint32_t rt : 5;
            uint32_t rs : 5;
            uint32_t opcode : 6;
        } b;
    };

    // 32-bit MIPS R3000A processor.
    class cpu_t {
    public:
        cpu_t(bus_t*);

        void tick(); // advance by one instruction

        void execute(cpu_instr_t); // execute instruction

    private:
        uint32_t get_reg(uint32_t);
        void set_reg(uint32_t, uint32_t);

        void op_lui(cpu_instr_t);

        cpu_reg_t regs[32]; // general purpose registers
        cpu_reg_t pc; // program counter
        cpu_reg_t hi; // hi register
        cpu_reg_t lo; // hi register

        bus_t* bus; // pointer to bus
    };
}
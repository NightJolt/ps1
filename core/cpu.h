#pragma once

#include "defs.h"

namespace ps1 {
    union cpu_instr_t {
        cpu_instr_t() = default;
        cpu_instr_t(uint32_t);

        operator uint32_t();

        uint32_t raw;

        struct {
            uint32_t subfunc : 6;
            uint32_t imm5 : 5;
            uint32_t rd : 5;
            uint32_t rt : 5;
            uint32_t rs : 5;
            uint32_t opcode : 6;
        } a;

        struct {
            uint32_t imm16 : 16;
            uint32_t rt : 5;
            uint32_t rs : 5;
            uint32_t opcode : 6;
        } b;
    };

    enum struct cpu_opcode_t {
        special = 0b000000,
        lui = 0b001111,
        ori = 0b001101,
        sw = 0b101011,
        addiu = 0b001001,
    };

    enum struct cpu_subfunc_t {
        ssl = 0b000000,
    };

    // 32-bit MIPS R3000A processor.
    class cpu_t {
    public:
        cpu_t(bus_t*);

        void tick(); // advance by one instruction

        void execute(cpu_instr_t); // execute instruction
        void execute_special(cpu_instr_t); // execute special instruction
        void execute_err(cpu_instr_t); // handle invalid cpu instruction

    private:
        uint32_t get_reg(uint32_t);
        void set_reg(uint32_t, uint32_t);

        void op_lui(cpu_instr_t);
        void op_ori(cpu_instr_t);
        void op_sw(cpu_instr_t);
        void op_ssl(cpu_instr_t);
        void op_addiu(cpu_instr_t);

        umap_t <cpu_opcode_t, func_t <void(cpu_instr_t)>> opmap;
        umap_t <cpu_subfunc_t, func_t <void(cpu_instr_t)>> opspecmap;

        cpu_reg_t regs[32]; // general purpose registers
        cpu_reg_t pc; // program counter
        cpu_reg_t hi; // hi register
        cpu_reg_t lo; // hi register

        bus_t* bus; // pointer to bus
    };
}
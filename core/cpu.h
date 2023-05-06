#pragma once

#include "defs.h"

namespace ps1 {
    union cpu_instr_t {
        cpu_instr_t() {}
        cpu_instr_t(uint32_t value) : raw(value) {}

        operator uint32_t() {
            return raw;
        }

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

        struct {
            uint32_t imm26 : 26;
            uint32_t opcode : 6;
        } c;
    };

    enum struct cpu_opcode_t {
        SPECIAL = 0b000000,
        LUI = 0b001111,
        ANDI = 0b001100,
        ORI = 0b001101,
        SW = 0b101011,
        SH = 0b101001,
        SB = 0b101000,
        LW = 0b100011,
        LB = 0b100000,
        ADDIU = 0b001001,
        ADDI = 0b001000,
        J = 0b000010,
        JAL = 0b000011,
        BNE = 0b000101,
        BEQ = 0b000100,
        COP = 0b010000,
    };

    enum struct cpu_subfunc_t {
        SSL = 0b000000,
        OR = 0b100101,
        SLTU = 0b101011,
        ADDU = 0b100001,
        JR = 0b001000,
    };

    enum struct cpu_state_t {
        sleeping,
        running,
        halted
    };

    // * 32-bit MIPS R3000A processor.
    struct cpu_t {
        /*
        * instruction in delay slot
        * this instruction is going to be executed on next cpu cycle
        */
        cpu_instr_t instr_delay_slot;
        
        /*
        * fetching data from ram first stores value in load delay slot.
        * after half* cycle value is stored in target register
        */
        uint32_t load_delay_target;
        uint32_t load_delay_value;

        /*
        * need two set of registers to simulate half cycles from load delay slot
        */
        cpu_reg_t in_regs[32]; // * general purpose registers
        cpu_reg_t out_regs[32]; // * we need to torelate load delay slot. however, copying 128 byte per cycle is terrible. need better solution!
        cpu_reg_t hi; // * hi register
        cpu_reg_t lo; // * lo register

        /*
        * program counter pointer
        * instruction on this address will be fetched and stored in instruction delay slot on next cpu cycle
        */
        cpu_reg_t pc;

        /*
        * cop0 registers
        *
        * reg 12 (status register):
        * bit 16 - redirects all bus r/w to cache
        */
        cpu_reg_t c0regs[32];

        bus_t* bus;

        cpu_state_t state;

        // ! debug data
        uint32_t instr_exec_cnt; // * number of instructions executed
    };

     // * init scpu state
    void cpu_init(cpu_t*, bus_t*);

    // * advance by one instruction
    void cpu_tick(cpu_t*);

    // *put cpu in halted state
    void cpu_set_state(cpu_t*, cpu_state_t);
}
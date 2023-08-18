#pragma once

#include "defs.h"

namespace ps1 {
#pragma pack(push, 1) // ! it must always be 4 bytes
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
#pragma pack(pop)

    enum struct cpu_opcode_t {
        SPECIAL = 0b000000,
        BBBB = 0b000001,
        J = 0b000010,
        JAL = 0b000011,
        BEQ = 0b000100,
        BNE = 0b000101,
        BLEZ = 0b000110,
        BGTZ = 0b000111,
        ADDI = 0b001000,
        ADDIU = 0b001001,
        SLTI = 0b001010,
        SLTIU = 0b001011,
        ANDI = 0b001100,
        ORI = 0b001101,
        // XORI = 0b001110,
        LUI = 0b001111,
        COP = 0b010000,
        // COP1 = 0b010001,
        // COP2 = 0b010010,
        // COP3 = 0b010011,
        // * N/A gap
        LB = 0b100000,
        LH = 0b100001,
        // LWL = 0b100010,
        LW = 0b100011,
        LBU = 0b100100,
        LHU = 0b100101,
        // LWR = 0b100110,
        // * N/A gap
        SB = 0b101000,
        SH = 0b101001,
        // SWL = 0b101010,
        SW = 0b101011,
        // * N/A gap
        // SWR = 0b101110,
        // * N/A gap
        // LWC0 = 0b110000,
        // LWC1 = 0b110001,
        // LWC2 = 0b110010,
        // LWC3 = 0b110011,
        // * N/A gap
        // SWC0 = 0b111000,
        // SWC1 = 0b111001,
        // SWC2 = 0b111010,
        // SWC3 = 0b111011,
    };

    enum struct cpu_subfunc_t {
        SLL = 0b000000,
        // * N/A gap
        SRL = 0b000010,
        SRA = 0b000011,
        SLLV = 0b000100,
        // * N/A gap
        // SRLV = 0b000110,
        // SRAV = 0b000111,
        JR = 0b001000,
        JALR = 0b001001,
        // * N/A gap
        SYSCALL = 0b001100,
        BREAK = 0b001101,
        // * N/A gap
        MFHI = 0b010000,
        MTHI = 0b010001,
        MFLO = 0b010010,
        MTLO = 0b010011,
        // * N/A gap
        // MULT = 0b011000,
        // MULTU = 0b011001,
        DIV = 0b011010,
        DIVU = 0b011011,
        // * N/A gap
        ADD = 0b100000,
        ADDU = 0b100001,
        // SUB = 0b100010,
        SUBU = 0b100011,
        AND = 0b100100,
        OR = 0b100101,
        XOR = 0b100110,
        NOR = 0b100111,
        // * N/A gap
        SLT = 0b101010,
        SLTU = 0b101011,
    };

    enum struct cpu_state_t {
        sleeping,
        running,
        halted
    };

    // * 32-bit MIPS R3000A processor.
    struct cpu_t {
        /*
        * fetching data from ram first stores value in load delay slot.
        * after half* cycle value is stored in target register
        */
        uint32_t load_delay_target;
        uint32_t load_delay_value;

        /*
        * need two set of registers to simulate half cycles from load delay slot
        *
        * reg 0 zero
        * reg 1 assembler temporary
        * reg 2-3 return values
        * reg 4-7 arguments
        * reg 8-15 temporary
        * reg 16-23 saved
        * reg 24-25 temporary
        * reg 26-27 kernel
        * reg 28 global pointer
        * reg 29 stack pointer
        * reg 30 frame pointer
        * reg 31 return address
        * 
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
        * currently executing instruction
        * needed for exception handling
        */
        cpu_reg_t cpc;

        /*
        * next pc
        * this is the address of the instruction that will be executed on next cpu cycle
        * needed to simulate branch delay slot
        */
        cpu_reg_t npc;

        /*
        * cop0 registers
        *
        * reg 3 BPC, used to generate a breakpoint exception when the PC takes the given value
        * reg 5 BDA, the data breakpoint. breaks when a certain address is accessed on a data load/store
        * reg 6 no info
        * reg 7 DCIC, used to enable and disable the various hardware breakpoints
        * reg 9 BDAM, it’s a bitmask applied when testing for BDA above. can trigger on a range of address instead of a single one.
        * reg 11 BPCM, same as BDAM but for BPC
        * reg 13 cause of exception. readonly.  bits [9:8] are writable to force an exception
        * 
        * reg 12 (status register):
        * bit 16 - redirects all bus r/w to cache
        * bit 22 - boot exception vectors in RAM/ROM (0=RAM/KSEG0, 1=ROM/KSEG1)
        */
        cpu_reg_t c0regs[32];

        bus_t* bus;

        cpu_state_t state;

        // ! debug data
        uint32_t instr_exec_cnt; // * number of instructions executed
        set_t<mem_addr_t> breakpoints; // * breakpoints
    };

     // * init scpu state
    void cpu_init(cpu_t*, bus_t*);

     // * clear scpu state
    void cpu_exit(cpu_t*);

    // * advance by one instruction
    void cpu_tick(cpu_t*);

    // * put cpu in specified state
    void cpu_set_state(cpu_t*, cpu_state_t);
    
    // * save cpu state to file
    void cpu_save_state(cpu_t*);

    // * load cpu state from file
    void cpu_load_state(cpu_t*);
}
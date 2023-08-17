#include "cpu.h"
#include "bus.h"
#include "logger.h"
#include "file.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // * magic value for debugging
    const cpu_instr_t nop = 0x00000000; // * no operation

    // * status register bits
    constexpr uint32_t SR_ISOLATE_CACHE_BIT = 1 << 16; // * redirect all subsequent R/W to cache
    constexpr uint32_t SR_BOOT_EXCEPTION_VECTORS_BIT = 1 << 22; // * BEV bit, 0=RAM/KSEG0, 1=ROM/KSEG1

    uint32_t sign_extend_16(uint32_t value) {
        return (uint32_t)(int16_t)value;
    }

    uint32_t sign_extend_8(uint32_t value) {
        return (uint32_t)(int8_t)value;
    }

    bool is_add_overflow(int32_t a, int32_t b) {
        return (a > 0 && b > 0 && a + b < 0) || (a < 0 && b < 0 && a + b > 0);
    }
}

namespace ps1 {
    // * all load and store operations are redirected to d-cache instead of main memory
    bool is_cache_isolated(cpu_t* cpu) {
        return cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT;
    }

    mem_addr_t get_exception_handler_addr(cpu_t* cpu) {
        return cpu->c0regs[12] & SR_BOOT_EXCEPTION_VECTORS_BIT ? 0xBFC00180 : 0x80000080;
    }
}

namespace ps1 {
    uint32_t get_reg(cpu_t* cpu, uint32_t i) {
        return cpu->in_regs[i];
    }

    void set_reg(cpu_t* cpu, uint32_t i, uint32_t v) {
        cpu->out_regs[i] = v;
        cpu->out_regs[0] = 0; // * $zero is always zero
    }

    void set_reg_delayed(cpu_t* cpu, uint32_t i, uint32_t v) {
        cpu->load_delay_target = i;
        cpu->load_delay_value = v;
    }

    uint32_t get_c0reg(cpu_t* cpu, uint32_t i) {
        return cpu->c0regs[i];
    }

    void set_c0reg(cpu_t* cpu, uint32_t i, uint32_t v) {
        DEBUG_CODE(
            if ((i == 3 || i == 5 || i == 6 || i == 7 || i == 9 || i == 11 || i == 13) && v != 0) 
                logger::push("nonzero value written to hardware breakpoint register", logger::type_t::warning, "cpu")
        );

        cpu->c0regs[i] = v;
    }
}

namespace ps1 {
    enum struct exception_t : uint32_t {
        load = 0x4,
        store = 0x5,
        syscall = 0x8,
        overflow = 0xC,
    };

    /*
    * throw exception
    */
    void throw_exception(cpu_t* cpu, exception_t cause) {
        mem_addr_t handler_func_addr = get_exception_handler_addr(cpu);

        // * push no interrupt/kernel mode to interrupt/kernel-user mode stack
        cpu_reg_t status = cpu->c0regs[12];
        status = (status & (~0x3F)) | ((status << 2) & 0x3F);

        cpu->c0regs[12] = status;
        cpu->c0regs[13] = ((uint32_t)cause) << 2;
        cpu->c0regs[14] = cpu->cpc;

        // ! might not work in case of 4 byte forward jump in branching
        if ((int32_t)cpu->pc - (int32_t)cpu->cpc != sizeof(cpu_instr_t)) {
            cpu->c0regs[14] -= sizeof(cpu_instr_t);
            cpu->c0regs[13] |= 1 << 31;
        }

        cpu->pc = handler_func_addr;
        cpu->npc = cpu->pc + sizeof(cpu_instr_t);
    }

    /*
    * return from exception
    */
    void op_rfe(cpu_t* cpu, cpu_instr_t instr) {
        cpu_reg_t status = cpu->c0regs[12];
        cpu->c0regs[12] = (status & (~0x3F)) | ((status & 0x3F) >> 2);
    }

    /*
    * syscall
    */
    void op_syscall(cpu_t* cpu, cpu_instr_t instr) {
        throw_exception(cpu, exception_t::syscall);
    }

    /*
    * load upper immediate
    * stores 16 bit immediate value into 16 msb of target register
    */
    void op_lui(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, instr.b.imm16 << 16);
    }
    
    /*
    * bitwise and immediate
    * applies bitwise and operator to target register and immediate value
    */
    void op_andi(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) & instr.b.imm16);
    }
    
    /*
    * bitwise or immediate
    * applies bitwise or operator to target register and immediate value
    */
    void op_ori(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) | instr.b.imm16);
    }

    /*
    * store word
    * 32 bit memory aligned store into bus
    */
    void op_sw(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        mem_addr_t addr = get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16);

        if (addr % 4 == 0) {
            bus_store32(cpu->bus, addr, get_reg(cpu, instr.b.rt));
        } else {
            throw_exception(cpu, exception_t::store);
        }
    }

    /*
    * store halfword
    * 16 bit memory aligned store into bus
    */
    void op_sh(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        mem_addr_t addr = get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16);

        if (addr % 2 == 0) {
            bus_store16(cpu->bus, addr, get_reg(cpu, instr.b.rt));
        } else {
            throw_exception(cpu, exception_t::store);
        }
    }

    /*
    * store byte
    * 8 bit store into bus
    */
    void op_sb(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        bus_store8(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    /*
    * load word
    * 32 bit memory aligned fetch from bus
    */
    void op_lw(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        mem_addr_t addr = get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16);

        if (addr % 4 == 0) {
            set_reg_delayed(cpu, instr.b.rt, bus_fetch32(cpu->bus, addr));
        } else {
            throw_exception(cpu, exception_t::load);
        }
    }

    /*
    * load halfword
    * 16 bit memory aligned fetch from bus
    */
   void op_lh(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        mem_addr_t addr = get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16);

        if (addr % 2 == 0) {
            set_reg_delayed(cpu, instr.b.rt, sign_extend_16(bus_fetch16(cpu->bus, addr)));
        } else {
            throw_exception(cpu, exception_t::load);
        }
    }

    /*
    * load halfword unsigned
    * 16 bit memory aligned fetch from bus
    */
   void op_lhu(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        mem_addr_t addr = get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16);

        if (addr % 2 == 0) {
            set_reg_delayed(cpu, instr.b.rt, bus_fetch16(cpu->bus, addr));
        } else {
            throw_exception(cpu, exception_t::load);
        }
    }

    /*
    * load byte
    * 8 bit fetch from bus
    */
    void op_lb(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        set_reg_delayed(cpu, instr.b.rt, sign_extend_8(bus_fetch8(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16))));
    }

    /*
    * load byte unsigned
    * 8 bit fetch from bus
    */
    void op_lbu(cpu_t* cpu, cpu_instr_t instr) {
        if (is_cache_isolated(cpu)) return;

        set_reg_delayed(cpu, instr.b.rt, bus_fetch8(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16)));
    }

    /*
    * shift left logical
    * shifts value to left by n bits
    * also used as nop (sll $zero, $zero, 0) -> (instr 0x0)
    */
    void op_sll(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt) << instr.a.imm5);
    }

    /*
    * shift left logical variable
    */
    void op_sllv(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt) << (get_reg(cpu, instr.a.rs) & 0x1F)); // *we only care about the last 5 bits
    }

    /*
    * shift right logical
    * does not preserve sign bit
    */
    void op_srl(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt) >> instr.a.imm5);
    }

    /*
    * shift right arithmetic
    * preserves sign bit
    */
    void op_sra(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, ((int32_t)get_reg(cpu, instr.a.rt)) >> instr.a.imm5);
    }

    /*
    * add immediate unsigned
    * adds sign extended immediate value to register
    */
    void op_addiu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16));
    }

    /*
    * add immediate
    * adds sign extended immediate value to register
    * must throw and exeption when overflow occurs
    */
    void op_addi(cpu_t* cpu, cpu_instr_t instr) {
        int32_t a = get_reg(cpu, instr.b.rs);
        int32_t b = sign_extend_16(instr.b.imm16);

        if (is_add_overflow(a, b)) {
            throw_exception(cpu, exception_t::overflow);
        } else {
            set_reg(cpu, instr.b.rt, a + b);
        }
    }

    /*
    * add unsigned
    * adds two registers
    */
    void op_addu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) + get_reg(cpu, instr.a.rt));
    }

    /*
    * add unsigned
    * adds two registers
    * must throw and exeption when overflow occurs
    */
    void op_add(cpu_t* cpu, cpu_instr_t instr) {
        int32_t a = get_reg(cpu, instr.a.rs);
        int32_t b = get_reg(cpu, instr.a.rt);

        if (is_add_overflow(a, b)) {
            throw_exception(cpu, exception_t::overflow);
        } else {
            set_reg(cpu, instr.a.rd, a + b);
        }
    }

    /*
    * jump
    * jump within the current 256MB of addressable memory
    */
    void op_j(cpu_t* cpu, cpu_instr_t instr) {
        cpu->npc = (cpu->npc & 0xF0000000) | (instr.c.imm26 << 2); // * << 2 align with memory
    }

    /*
    * or
    * bitwise or two registers
    */
    void op_or(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rt, get_reg(cpu, instr.a.rs) | get_reg(cpu, instr.a.rt));
    }

    /*
    * and
    * bitwise and two registers
    */
    void op_and(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) & get_reg(cpu, instr.a.rt));
    }

    /*
    * move to cop0
    * move value from cpu reg to cop0 reg
    */
    void op_mtc0(cpu_t* cpu, cpu_instr_t instr) {
        set_c0reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt));
    }

    /*
    * move from cop0
    * move value from cop0 reg to cpu reg
    */
    void op_mfc0(cpu_t* cpu, cpu_instr_t instr) {
        ASSERT(instr.a.rd == 12 || instr.a.rd == 13 ||  instr.a.rd == 14, "only cop0 reg 12/13/14 is implemented");

        set_reg_delayed(cpu, instr.a.rt, get_c0reg(cpu, instr.a.rd));
    }

    /*
    * move program counter to new address
    * aligns offset with memory and then compensates one extra cpu cycle
    */
    void cpu_branch(cpu_t* cpu, uint32_t offset) {
        cpu->npc += (offset << 2) - sizeof(cpu_instr_t);
    }

    /*
    * branch if not equal
    */
    void op_bne(cpu_t* cpu, cpu_instr_t instr) {
        if (get_reg(cpu, instr.b.rs) != get_reg(cpu, instr.b.rt)) {
            cpu_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    /*
    * branch if equal
    */
    void op_beq(cpu_t* cpu, cpu_instr_t instr) {
        if (get_reg(cpu, instr.b.rs) == get_reg(cpu, instr.b.rt)) {
            cpu_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    /*
    * branch if greater than zero
    */
    void op_bgtz(cpu_t* cpu, cpu_instr_t instr) {
        if (get_reg(cpu, instr.b.rs) > 0) {
            cpu_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    /*
    * branch if less or equal to zero
    */
    void op_blez(cpu_t* cpu, cpu_instr_t instr) {
        if (get_reg(cpu, instr.b.rs) <= 0) {
            cpu_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    /*
    * bltz, bgez, bltzal, bgezal
    */
    void op_bbbb(cpu_t* cpu, cpu_instr_t instr) {
        if (((instr.b.rt >> 4) & 0x1) ^ (get_reg(cpu, instr.b.rs) < 0)) {
            if (instr.b.rt & 0x1) {
                set_reg(cpu, 31, cpu->npc);
            }

            cpu_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    /*
    * set on less than unsigned
    * if left operand register is less then right operand register sets value to 1, otherwise 0
    */
    void op_sltu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) < get_reg(cpu, instr.a.rt));
    }

    /*
    * set if less than immediate
    * if left operand register is less then immediate value sets value to 1, otherwise 0
    */
    void op_slti(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, ((int32_t)get_reg(cpu, instr.b.rs)) < sign_extend_16(instr.b.imm16));
    }

    /*
    * set if less than immediate unsigned
    * if left operand register is less then immediate value sets value to 1, otherwise 0
    */
    void op_sltiu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) < sign_extend_16(instr.b.imm16));
    }

    /*
    * set on less then
    * if left operand register is less then right operand value value sets value to 1, otherwise 0
    */
    void op_slt(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, (int32_t)get_reg(cpu, instr.a.rs) < (int32_t)get_reg(cpu, instr.a.rt));
    }

    /*
    * jump and link
    * jumps and stores current pc into reg 31 ($ra)
    * used to call functions
    */
    void op_jal(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, 31, cpu->npc);

        op_j(cpu, instr);
    }

    /*
    * jump register
    * jumps to and address stored in register
    * used as a return from function
    */
    void op_jr(cpu_t* cpu, cpu_instr_t instr) {
        cpu->npc = get_reg(cpu, instr.a.rs);
    }

    /*
    * jump and link register
    * jumps to and address stored in register
    * stores current pc into register
    */
    void op_jalr(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, cpu->npc);

        cpu->npc = get_reg(cpu, instr.a.rs);
    }

    /*
    * subtract unsigned
    */
    void op_subu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) - get_reg(cpu, instr.a.rt));
    }

    /*
    * divide
    TODO: needs to be properly emulated for correct timing
    */
    void op_div(cpu_t* cpu, cpu_instr_t instr) {
        int32_t n = get_reg(cpu, instr.a.rs);
        int32_t d = get_reg(cpu, instr.a.rt);
        
        if (d == 0) {
            cpu->lo = n < 0 ? 1 : -1;
            cpu->hi = n;
        } else if ((uint32_t)n == 0x80000000 && d == ~0) {
            cpu->lo = 0x80000000;
            cpu->hi = 0;
        } else {
            cpu->lo = n / d;
            cpu->hi = n % d;
        }
    }

    /*
    * divide unsigned
    TODO: needs to be properly emulated for correct timing
    */
    void op_divu(cpu_t* cpu, cpu_instr_t instr) {
        uint32_t n = get_reg(cpu, instr.a.rs);
        uint32_t d = get_reg(cpu, instr.a.rt);
        
        if (d == 0) {
            cpu->lo = ~0u;
            cpu->hi = n;
        } else {
            cpu->lo = n / d;
            cpu->hi = n % d;
        }
    }

    /*
    * move from LO
    TODO: will stall if div is not finished
    */
    void op_mflo(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, cpu->lo);
    }

    /*
    * move from HI
    TODO: will stall if div is not finished
    */
    void op_mfhi(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, cpu->hi);
    }

    /*
    * move to LO
    */
    void op_mtlo(cpu_t* cpu, cpu_instr_t instr) {
        cpu->lo = get_reg(cpu, instr.a.rs);
    }

    /*
    * move to HI
    */
    void op_mthi(cpu_t* cpu, cpu_instr_t instr) {
        cpu->hi = get_reg(cpu, instr.a.rs);
    }

    /*
    * nor (bitwise not or)
    */
   void op_nor(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, !(get_reg(cpu, instr.a.rs) | get_reg(cpu, instr.a.rt)));
    }

    // handle invalid cpu instruction
    void execute_err(cpu_t* cpu, cpu_instr_t instr) {
        DEBUG_CODE(char err_msg_buffer[64]);
        DEBUG_CODE(
            sprintf(
                err_msg_buffer,
                "COULD NOT EXECUTE 0x%08X (opcode %d-%02X)",
                (uint32_t)instr,
                (uint32_t)(bool)instr.a.opcode , instr.a.opcode ? instr.a.opcode : instr.a.subfunc
            )
        );
        // ASSERT(false, err_msg_buffer);

        DEBUG_CODE(logger::push(err_msg_buffer, logger::type_t::error, "cpu"));

        cpu_set_state(cpu, cpu_state_t::halted);
    }

    typedef void(*cpu_instr_handler_func)(cpu_t*, cpu_instr_t);

    // * execute cop0 instruction
    void execute_cop0(cpu_t* cpu, cpu_instr_t instr) {
        if (instr.a.opcode == 0b010000 && instr.a.rs == 0b00100) {
            op_mtc0(cpu, instr);
        } else if (instr.a.opcode == 0b010000 && instr.a.rs == 0b00000) {
            op_mfc0(cpu, instr);
        } else if (instr.a.opcode == 0b010000 && instr.a.rs == 0b10000 && instr.a.subfunc == 0b010000) {
            op_rfe(cpu, instr);
        } else {
            DEBUG_CODE(execute_err(cpu, instr));
        }
    }

    // * execute special instruction
    void execute_special(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_subfunc_t, cpu_instr_handler_func> opmap = {
            { cpu_subfunc_t::SLL, op_sll },
            { cpu_subfunc_t::SLLV, op_sllv },
            { cpu_subfunc_t::SYSCALL, op_syscall },
            { cpu_subfunc_t::OR, op_or },
            { cpu_subfunc_t::AND, op_and },
            { cpu_subfunc_t::SLTU, op_sltu },
            { cpu_subfunc_t::SLT, op_slt },
            { cpu_subfunc_t::ADDU, op_addu },
            { cpu_subfunc_t::ADD, op_add },
            { cpu_subfunc_t::JR, op_jr },
            { cpu_subfunc_t::JALR, op_jalr },
            { cpu_subfunc_t::SUBU, op_subu },
            { cpu_subfunc_t::SRA, op_sra },
            { cpu_subfunc_t::SRL, op_srl },
            { cpu_subfunc_t::DIV, op_div },
            { cpu_subfunc_t::DIVU, op_divu },
            { cpu_subfunc_t::MFLO, op_mflo },
            { cpu_subfunc_t::MFHI, op_mfhi },
            { cpu_subfunc_t::MTLO, op_mtlo },
            { cpu_subfunc_t::MTHI, op_mthi },
            { cpu_subfunc_t::NOR, op_nor },

        };

        auto subfunc = static_cast <cpu_subfunc_t> (instr.a.subfunc);

        if (opmap.contains(subfunc)) {
            opmap[subfunc](cpu, instr);

            return;
        }

        DEBUG_CODE(execute_err(cpu, instr));
    }
    
    // * execute instruction
    void execute(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_opcode_t, cpu_instr_handler_func> opmap = {
            { cpu_opcode_t::SPECIAL, execute_special },
            { cpu_opcode_t::BBBB, op_bbbb },
            { cpu_opcode_t::LUI, op_lui },
            { cpu_opcode_t::ANDI, op_andi },
            { cpu_opcode_t::ORI, op_ori },
            { cpu_opcode_t::SW, op_sw },
            { cpu_opcode_t::SH, op_sh },
            { cpu_opcode_t::SB, op_sb },
            { cpu_opcode_t::LW, op_lw },
            { cpu_opcode_t::LH, op_lh },
            { cpu_opcode_t::LHU, op_lhu },
            { cpu_opcode_t::LB, op_lb },
            { cpu_opcode_t::LBU, op_lbu },
            { cpu_opcode_t::ADDIU, op_addiu },
            { cpu_opcode_t::ADDI, op_addi },
            { cpu_opcode_t::J, op_j },
            { cpu_opcode_t::JAL, op_jal },
            { cpu_opcode_t::BNE, op_bne },
            { cpu_opcode_t::BEQ, op_beq },
            { cpu_opcode_t::BGTZ, op_bgtz },
            { cpu_opcode_t::BLEZ, op_blez },
            { cpu_opcode_t::SLTI, op_slti },
            { cpu_opcode_t::SLTIU, op_sltiu },
            { cpu_opcode_t::COP, execute_cop0 },
        };

        auto opcode = static_cast <cpu_opcode_t> (instr.a.opcode);

        if (opmap.contains(opcode)) {
            opmap[opcode](cpu, instr);

            return;
        }

        DEBUG_CODE(execute_err(cpu, instr));
    }
}

void ps1::cpu_init(cpu_t* cpu, bus_t* bus) {
    cpu->bus = bus;

    // * initialize registers to garbage value
    for (int i = 1; i < 32; i++) {
        cpu->in_regs[i] = register_garbage_value;
        cpu->out_regs[i] = register_garbage_value;
        cpu->c0regs[i] = register_garbage_value;
    }

    // * $zero is always zero
    cpu->in_regs[0] = 0;
    cpu->out_regs[0] = 0;

    cpu->hi = register_garbage_value;
    cpu->lo = register_garbage_value;

    cpu->pc = BIOS_ENTRY; // * target BIOS entry function
    cpu->cpc = cpu->pc;
    cpu->npc = cpu->pc + sizeof(cpu_instr_t);

    set_reg_delayed(cpu, 0, 0);

    cpu->c0regs[12] = 0; // * set cop0 status register to 0

    cpu_set_state(cpu, cpu_state_t::sleeping);

    // ! debug
    cpu->instr_exec_cnt = 0;
}

void ps1::cpu_exit(cpu_t* cpu) {
    // ! implement
}

void ps1::cpu_tick(cpu_t* cpu) {
    cpu->cpc = cpu->pc; // * update current program counter

    if (cpu->cpc % sizeof(cpu_instr_t) != 0) {
        throw_exception(cpu, exception_t::load);

        return;
    }

    cpu_instr_t instr = bus_fetch32(cpu->bus, cpu->cpc); // * fetch current instruction from memory
    
    cpu->pc = cpu->npc; // * advance program counter
    cpu->npc += sizeof(cpu_instr_t); // * advance program counter

    // * move value from load delay slot to output register values
    set_reg(cpu, cpu->load_delay_target, cpu->load_delay_value);
    set_reg_delayed(cpu, 0, 0);

    execute(cpu, instr); // * execute next instruction
    
    // * update reg values
    memcpy(cpu->in_regs, cpu->out_regs, sizeof(cpu_reg_t) * 32);
    
    // ! debug
    cpu->instr_exec_cnt++;

    // ! debug breakpoints
    {
        // if (bus_fetch32(cpu->bus, cpu->pc) == 0x0040c827) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->pc < BIOS_ENTRY) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->instr_exec_cnt == 71540) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->pc == 0x000005bc) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->pc == 0xbfc06850) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping); // * A0 write
        // if (cpu->pc == 0xbfc06858) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->pc == 0x600) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->instr_exec_cnt == 79285) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->instr_exec_cnt == 79310) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
    }

    
    if (cpu->breakpoints.contains(cpu->pc)) {
        cpu_set_state(cpu, cpu_state_t::sleeping);

        return;
    }
}

void ps1::cpu_set_state(cpu_t* cpu, cpu_state_t cpu_state) {
    cpu->state = cpu_state;

    DEBUG_CODE(if (cpu_state == cpu_state_t::halted) logger::push("CPU HALTED", logger::type_t::error, "cpu"));
}

void ps1::cpu_save_state(cpu_t* cpu) {
    file::write32(cpu->load_delay_target);
    file::write32(cpu->load_delay_value);

    for (auto val : cpu->in_regs) {
        file::write32(val);
    }

    for (auto val : cpu->out_regs) {
        file::write32(val);
    }

    file::write32(cpu->hi);
    file::write32(cpu->lo);

    file::write32(cpu->cpc);
    file::write32(cpu->pc);
    file::write32(cpu->npc);

    for (auto val : cpu->c0regs) {
        file::write32(val);
    }

    file::write32(cpu->instr_exec_cnt);
}

void ps1::cpu_load_state(cpu_t* cpu) {
    cpu->load_delay_target = file::read32();
    cpu->load_delay_value = file::read32();

    for (auto& val : cpu->in_regs) {
        val = file::read32();
    }

    for (auto& val : cpu->out_regs) {
        val = file::read32();
    }

    cpu->hi = file::read32();
    cpu->lo = file::read32();

    cpu->cpc = file::read32();
    cpu->pc = file::read32();
    cpu->npc = file::read32();

    for (auto& val : cpu->c0regs) {
        val = file::read32();
    }

    cpu->instr_exec_cnt = file::read32();

    cpu_set_state(cpu, cpu_state_t::sleeping);
}
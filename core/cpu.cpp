#include "cpu.h"
#include "bus.h"
#include "logger.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // * magic value for debugging
    const cpu_instr_t nop = 0x00000000; // * no operation

    constexpr uint32_t SR_ISOLATE_CACHE_BIT = 1 << 16; // * redirect all subsequent R/W to cache

    uint32_t sign_extend_16(uint32_t value) {
        return (uint32_t)(int16_t)value;
    }

    uint32_t sign_extend_8(uint32_t value) {
        return (uint32_t)(int8_t)value;
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
        if (cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT) return; // * should be redirected to cache

        bus_store32(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    /*
    * store halfword
    * 16 bit memory aligned store into bus
    */
    void op_sh(cpu_t* cpu, cpu_instr_t instr) {
        if (cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT) return; // * should be redirected to cache

        bus_store16(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    /*
    * store byte
    * 8 bit store into bus
    */
    void op_sb(cpu_t* cpu, cpu_instr_t instr) {
        if (cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT) return; // * should be redirected to cache

        bus_store8(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    /*
    * load word
    * 32 bit memory aligned fetch from bus
    */
    void op_lw(cpu_t* cpu, cpu_instr_t instr) {
        cpu->load_delay_target = instr.b.rt;
        cpu->load_delay_value = bus_fetch32(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16));
    }

    /*
    * load byte
    * 8 bit fetch from bus
    */
    void op_lb(cpu_t* cpu, cpu_instr_t instr) {
        cpu->load_delay_target = instr.b.rt;
        cpu->load_delay_value = sign_extend_8(bus_fetch8(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16)));
    }

    /*
    * shift left logical
    * shifts value to left by n bits
    * also used as nop (sll $zero, $zero, 0) -> (instr 0x0)
    */
    void op_ssl(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rt, get_reg(cpu, instr.a.rs) << instr.a.imm5);
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
    * must throw and exeption when overflow occurs
    */
    void op_addi(cpu_t* cpu, cpu_instr_t instr) {
        op_addiu(cpu, instr); // TODO: MUST CHECK FOR OVERFLOW
    }

    /*
    * add unsigned
    * adds two registers
    */
    void op_addu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) + get_reg(cpu, instr.a.rt));  // ? MUST CHECK FOR OVERFLOW ?
    }

    /*
    * jump
    * jump within the current 256MB of addressable memory
    */
    void op_j(cpu_t* cpu, cpu_instr_t instr) {
        cpu->pc = (cpu->pc & 0xF0000000) | (instr.c.imm26 << 2); // * << 2 align with memory
    }

    /*
    * or
    * bitwise or two registers
    */
    void op_or(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) | get_reg(cpu, instr.a.rt));
    }

    /*
    * move to cop0
    * move value from cpu reg to cop0 reg
    */
    void op_mtc0(cpu_t* cpu, cpu_instr_t instr) {
        set_c0reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt));
    }

    /*
    * move program counter to new address
    * aligns offset with memory and then compensates one extra cpu cycle
    */
    void cpu_branch(cpu_t* cpu, uint32_t offset) {
        cpu->pc += (offset << 2) - sizeof(cpu_instr_t);
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
    * set on less than unsigned
    * if left operand register is less then right operand register sets value to 1, otherwise 0
    */
    void op_sltu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) < get_reg(cpu, instr.a.rt));
    }

    /*
    * jump and link
    * jumps and stores current pc into reg 31 ($ra)
    * used to call functions
    */
    void op_jal(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, 31, cpu->pc);

        op_j(cpu, instr);
    }

    /*
    * jump register
    * jumps to and address stored in register
    * used as a return from function
    */
    void op_jr(cpu_t* cpu, cpu_instr_t instr) {
        cpu->pc = get_reg(cpu, instr.a.rs);
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
        } else {
            DEBUG_CODE(execute_err(cpu, instr));
        }
    }

    // * execute special instruction
    void execute_special(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_subfunc_t, cpu_instr_handler_func> opmap = {
            { cpu_subfunc_t::SSL, op_ssl },
            { cpu_subfunc_t::OR, op_or },
            { cpu_subfunc_t::SLTU, op_sltu },
            { cpu_subfunc_t::ADDU, op_addu },
            { cpu_subfunc_t::JR, op_jr },
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
            { cpu_opcode_t::LUI, op_lui },
            { cpu_opcode_t::ANDI, op_andi },
            { cpu_opcode_t::ORI, op_ori },
            { cpu_opcode_t::SW, op_sw },
            { cpu_opcode_t::SH, op_sh },
            { cpu_opcode_t::SB, op_sb },
            { cpu_opcode_t::LW, op_lw },
            { cpu_opcode_t::LB, op_lb },
            { cpu_opcode_t::ADDIU, op_addiu },
            { cpu_opcode_t::ADDI, op_addi },
            { cpu_opcode_t::J, op_j },
            { cpu_opcode_t::JAL, op_jal },
            { cpu_opcode_t::BNE, op_bne },
            { cpu_opcode_t::BEQ, op_beq },
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

    cpu->instr_delay_slot = nop;
    cpu->load_delay_target = 0;
    cpu->load_delay_value = 0;

    cpu->c0regs[12] = 0; // * set cop0 status register to 0

    cpu_set_state(cpu, cpu_state_t::sleeping);

    // ! debug
    cpu->instr_exec_cnt = 0;
}

void ps1::cpu_tick(cpu_t* cpu) {
    cpu_instr_t instr = cpu->instr_delay_slot; // * fetch instruction from delay slot
    cpu->instr_delay_slot = bus_fetch32(cpu->bus, cpu->pc); // * fetch instruction from memory

    cpu->pc += sizeof(cpu_instr_t); // * advance program counter

    // * move value from load delay slot to output register values
    set_reg(cpu, cpu->load_delay_target, cpu->load_delay_value);
    cpu->load_delay_target = 0;
    cpu->load_delay_value = 0;

    execute(cpu, instr); // * execute next instruction
    
    // * update reg values
    memcpy(cpu->in_regs, cpu->out_regs, sizeof(cpu_instr_t) * 32);
    
    // ! debug
    cpu->instr_exec_cnt++;

    // ! debug breakpoints
    {
        // if (cpu->in_regs[29] == 2149580520) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
        // if (cpu->instr_exec_cnt == 17367) ps1::cpu_set_state(cpu, ps1::cpu_state_t::sleeping);
    }
}

void ps1::cpu_set_state(cpu_t* cpu, cpu_state_t cpu_state) {
    cpu->state = cpu_state;

    DEBUG_CODE(if (cpu_state == cpu_state_t::halted) logger::push("CPU HALTED", logger::type_t::error, "cpu"));
}

#include "cpu.h"
#include "bus.h"
#include "logger.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // magic value for debugging
    const cpu_instr_t nop = 0x00000000; // no operation

    constexpr uint32_t SR_ISOLATE_CACHE_BIT = 1 << 16; // redirect all subsequent R/W to cache

    uint32_t sign_extend_16(uint32_t value) {
        return (uint32_t)(int16_t)value;
    }
}

namespace ps1 {
    uint32_t get_reg(cpu_t* cpu, uint32_t i) {
        return cpu->in_regs[i];
    }

    void set_reg(cpu_t* cpu, uint32_t i, uint32_t v) {
        cpu->out_regs[i] = v;
        cpu->out_regs[0] = 0; // $zero is always zero
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
    void op_lui(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, instr.b.imm16 << 16);
    }
    
    void op_ori(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) | instr.b.imm16);
    }

    void op_sw(cpu_t* cpu, cpu_instr_t instr) {
        if (cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT) return; // * should be redirected to cache

        bus_store32(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    void op_lw(cpu_t* cpu, cpu_instr_t instr) {
        if (cpu->c0regs[12] & SR_ISOLATE_CACHE_BIT) return; // * should be redirected to cache

        cpu->load_delay_target = instr.b.rt;
        cpu->load_delay_value = bus_fetch32(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16));
    }

    void op_ssl(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rt, get_reg(cpu, instr.a.rs) << instr.a.imm5);
    }

    void op_addiu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16)); // * not a signed addition
    }

    void op_addi(cpu_t* cpu, cpu_instr_t instr) {
        op_addiu(cpu, instr); // TODO: MUST CHECK FOR OVERFLOW
    }

    void op_j(cpu_t* cpu, cpu_instr_t instr) {
        cpu->pc = (cpu->pc & 0xF0000000) | (instr.c.imm26 << 2); // * << 2 align with memory
    }

    void op_or(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) | get_reg(cpu, instr.a.rt));
    }

    void op_mtc0(cpu_t* cpu, cpu_instr_t instr) {
        set_c0reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rt));
    }

    void op_branch(cpu_t* cpu, uint32_t offset) {
        cpu->pc += (offset << 2) - sizeof(cpu_instr_t);
    }

    void op_bne(cpu_t* cpu, cpu_instr_t instr) {
        if (get_reg(cpu, instr.b.rs) != get_reg(cpu, instr.b.rt)) {
            op_branch(cpu, sign_extend_16(instr.b.imm16));
        }
    }

    void op_sltu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) < get_reg(cpu, instr.a.rt));
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

    void execute_cop0(cpu_t* cpu, cpu_instr_t instr) {
        if (instr.a.opcode == 0b010000 && instr.a.rs == 0b00100) {
            op_mtc0(cpu, instr);
        } else {
            DEBUG_CODE(execute_err(cpu, instr));
        }
    }

     // execute special instruction
    void execute_special(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_subfunc_t, cpu_instr_handler_func> opmap = {
            { cpu_subfunc_t::SSL, op_ssl },
            { cpu_subfunc_t::OR, op_or },
            { cpu_subfunc_t::SLTU, op_sltu },
        };

        auto subfunc = static_cast <cpu_subfunc_t> (instr.a.subfunc);

        if (opmap.contains(subfunc)) {
            opmap[subfunc](cpu, instr);

            return;
        }

        DEBUG_CODE(execute_err(cpu, instr));
    }
    
     // execute instruction
    void execute(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_opcode_t, cpu_instr_handler_func> opmap = {
            { cpu_opcode_t::SPECIAL, execute_special },
            { cpu_opcode_t::LUI, op_lui },
            { cpu_opcode_t::ORI, op_ori },
            { cpu_opcode_t::SW, op_sw },
            { cpu_opcode_t::LW, op_lw },
            { cpu_opcode_t::ADDIU, op_addiu },
            { cpu_opcode_t::ADDI, op_addi },
            { cpu_opcode_t::J, op_j },
            { cpu_opcode_t::BNE, op_bne },
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

    // initialize registers to garbage value
    for (int i = 1; i < 32; i++) {
        cpu->in_regs[i] = register_garbage_value;
        cpu->out_regs[i] = register_garbage_value;
        cpu->c0regs[i] = register_garbage_value;
    }

    cpu->in_regs[0] = 0; // $zero is always zero
    cpu->out_regs[0] = 0;

    cpu->hi = register_garbage_value;
    cpu->lo = register_garbage_value;

    cpu->pc = BIOS_ENTRY; // start at BIOS main function

    cpu->instr_delay_slot = nop;
    cpu->load_delay_target = 0;
    cpu->load_delay_value = 0;

    // set cop0 status register to 0
    cpu->c0regs[12] = 0;

    cpu_set_state(cpu, cpu_state_t::sleeping);

    // ! debug
    cpu->instr_exec_cnt = 0;
}

void ps1::cpu_tick(cpu_t* cpu) {
    cpu_instr_t instr = cpu->instr_delay_slot; // fetch instruction from delay slot
    cpu->instr_delay_slot = bus_fetch32(cpu->bus, cpu->pc); // fetch instruction from memory

    cpu->pc += sizeof(cpu_instr_t); // advance program counter

    set_reg(cpu, cpu->load_delay_target, cpu->load_delay_value);
    cpu->load_delay_target = 0;
    cpu->load_delay_value = 0;

    execute(cpu, instr); // execute instruction
    
    memcpy(cpu->in_regs, cpu->out_regs, sizeof(cpu_instr_t) * 32);
    
    // ! debug
    cpu->instr_exec_cnt++;
}

void ps1::cpu_set_state(cpu_t* cpu, cpu_state_t cpu_state) {
    cpu->state = cpu_state;

    DEBUG_CODE(if (cpu_state == cpu_state_t::halted) logger::push("CPU HALTED", logger::type_t::error, "cpu"));
}

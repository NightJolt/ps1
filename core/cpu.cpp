#include "cpu.h"
#include "bus.h"
#include "logger.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // magic value for debugging
    const cpu_instr_t nop = 0x00000000; // no operation

    uint32_t sign_extend_16(uint32_t value) {
        return (uint32_t)(int16_t)value;
    }
}

namespace ps1 {
    uint32_t get_reg(cpu_t* cpu, uint32_t i) {
        return cpu->regs[i];
    }

    void set_reg(cpu_t* cpu, uint32_t i, uint32_t v) {
        cpu->regs[i] = v;
        cpu->regs[0] = 0; // $zero is always zero
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
        bus_store32(cpu->bus, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(cpu, instr.b.rt));
    }

    void op_ssl(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rt, get_reg(cpu, instr.a.rs) << instr.a.imm5);
    }

    void op_addiu(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.b.rt, get_reg(cpu, instr.b.rs) + sign_extend_16(instr.b.imm16)); // * not a signed addition
    }

    void op_j(cpu_t* cpu, cpu_instr_t instr) {
        cpu->pc = (cpu->pc & 0xF0000000) | (instr.c.imm26 << 2);
    }

    void op_or(cpu_t* cpu, cpu_instr_t instr) {
        set_reg(cpu, instr.a.rd, get_reg(cpu, instr.a.rs) | get_reg(cpu, instr.a.rt));
    }

     // handle invalid cpu instruction
    void execute_err(cpu_t* cpu, cpu_instr_t instr) {
        DEBUG_CODE(char err_msg_buffer[64]);
        DEBUG_CODE(
            sprintf(
                err_msg_buffer,
                "COULD NOT EXECUTE 0x%08X: 0x%08X (opcode %d-%02X)",
                cpu->pc, (uint32_t)instr,
                (uint32_t)(bool)instr.a.opcode , instr.a.opcode ? instr.a.opcode : instr.a.subfunc
            )
        );
        // ASSERT(false, err_msg_buffer);

        DEBUG_CODE(logger::push(err_msg_buffer, logger::type_t::error, "cpu"));

        cpu_set_state(cpu, cpu_state_t::halted);
    }

    typedef void(*cpu_instr_handler_func)(cpu_t*, cpu_instr_t);

     // execute special instruction
    void execute_special(cpu_t* cpu, cpu_instr_t instr) {
        static umap_t <cpu_subfunc_t, cpu_instr_handler_func> opspecmap = {
            { cpu_subfunc_t::SSL, op_ssl },
            { cpu_subfunc_t::OR, op_or },
        };

        auto subfunc = static_cast <cpu_subfunc_t> (instr.a.subfunc);

        if (opspecmap.contains(subfunc)) {
            opspecmap[subfunc](cpu, instr);

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
            { cpu_opcode_t::ADDIU, op_addiu },
            { cpu_opcode_t::J, op_j },
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

    for (int i = 1; i < 32; i++) {
        cpu->regs[i] = register_garbage_value; // initialize registers to garbage value
    }

    cpu->regs[0] = 0; // $zero is always zero

    cpu->delay_slot = nop;

    cpu->pc = BIOS_ENTRY; // start at BIOS main function

    cpu->hi = register_garbage_value;
    cpu->lo = register_garbage_value;

    cpu_set_state(cpu, cpu_state_t::sleeping);

    // ! debug
    cpu->instr_exec_cnt = 0;
}

void ps1::cpu_tick(cpu_t* cpu) {
    cpu_instr_t instr = cpu->delay_slot; // fetch instruction from delay slot
    cpu->delay_slot = bus_fetch32(cpu->bus, cpu->pc); // fetch instruction from memory

    cpu->pc += sizeof(cpu_instr_t); // advance program counter

    execute(cpu, instr); // execute instruction
    
    // ! debug
    cpu->instr_exec_cnt++;
}

void ps1::cpu_set_state(cpu_t* cpu, cpu_state_t cpu_state) {
    cpu->state = cpu_state;

    DEBUG_CODE(if (cpu_state == cpu_state_t::halted) logger::push("CPU HALTED", logger::type_t::error, "cpu"));
}

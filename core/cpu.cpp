#include "cpu.h"
#include "bus.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // magic value for debugging
}

ps1::cpu_instr_t::cpu_instr_t(uint32_t value) : raw(value) {}

ps1::cpu_instr_t::operator uint32_t() {
    return raw;
}

ps1::cpu_t::cpu_t(bus_t* bus) : bus(bus) {
    for (int i = 1; i < 32; i++) {
        regs[i] = register_garbage_value; // initialize registers to garbage value
    }

    regs[0] = 0; // $zero is always zero

    pc = BIOS_ENTRY; // start at BIOS main function

    hi = register_garbage_value;
    lo = register_garbage_value;
}

void ps1::cpu_t::tick() {
    cpu_instr_t instr = bus->fetch32(pc); // fetch instruction from memory

    execute(instr); // execute instruction

    pc += sizeof cpu_instr_t; // advance program counter
}

void ps1::cpu_t::execute(cpu_instr_t instr) {
    switch (static_cast <cpu_opcode_t> (instr.a.opcode)) {
        case cpu_opcode_t::special: {
            goto execute_special;
        }

        case cpu_opcode_t::lui: {
            op_lui(instr);

            break;
        }

        case cpu_opcode_t::ori: {
            op_ori(instr);

            break;
        }

        case cpu_opcode_t::sw: {
            op_sw(instr);

            break;
        }

        case cpu_opcode_t::addiu: {
            op_addiu(instr);

            break;
        }

        default: {
            goto execute_err;
        }
    }

    return;

execute_special:
    switch (static_cast <cpu_subfunc_t> (instr.a.subfunc)) {
        case cpu_subfunc_t::ssl: {
            op_ssl(instr);

            break;
        }

        default: {
            goto execute_err;
        }
    }

    return;

execute_err:
    DEBUG_CODE(char err_msg_buffer[64]);
    DEBUG_CODE(
        sprintf(
            err_msg_buffer,
            "COULD NOT EXECUTE 0x%08X: 0x%08X (opcode %d-%d)",
            pc, (uint32_t)instr,
            (uint32_t)(bool)instr.a.opcode , instr.a.opcode ? instr.a.opcode : instr.a.subfunc
        )
    );
    ASSERT(false, err_msg_buffer);
}

uint32_t ps1::cpu_t::get_reg(uint32_t i) {
    return regs[i];
}

void ps1::cpu_t::set_reg(uint32_t i, uint32_t v) {
    regs[i] = v;
    regs[0] = 0; // $zero is always zero
}

void ps1::cpu_t::op_lui(cpu_instr_t instr) {
    set_reg(instr.b.rt, instr.b.imm16 << 16);
}

void ps1::cpu_t::op_ori(cpu_instr_t instr) {
    set_reg(instr.b.rt, get_reg(instr.b.rs) | instr.b.imm16);
}

void ps1::cpu_t::op_sw(cpu_instr_t instr) {
    bus->store32(get_reg(instr.b.rs) + (uint32_t)(int16_t)instr.b.imm16, get_reg(instr.b.rt));
}

void ps1::cpu_t::op_ssl(cpu_instr_t instr) {
    set_reg(instr.a.rt, get_reg(instr.a.rs) << instr.a.imm5);
}

void ps1::cpu_t::op_addiu(cpu_instr_t instr) {
    set_reg(instr.b.rt, get_reg(instr.b.rs) + (uint32_t)(int16_t)instr.b.imm16); // * not a signed addition
}
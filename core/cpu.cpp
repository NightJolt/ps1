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

ps1::cpu_instr_t::cpu_instr_t(uint32_t value) : raw(value) {}

ps1::cpu_instr_t::operator uint32_t() {
    return raw;
}

ps1::cpu_t::cpu_t(bus_t* bus) : bus(bus), halted(false) {
    for (int i = 1; i < 32; i++) {
        regs[i] = register_garbage_value; // initialize registers to garbage value
    }

    regs[0] = 0; // $zero is always zero

    delay_slot = nop;

    pc = BIOS_ENTRY; // start at BIOS main function

    hi = register_garbage_value;
    lo = register_garbage_value;

    opmap = {
        { cpu_opcode_t::SPECIAL, [this](cpu_instr_t instr) { this->execute_special(instr); } },
        { cpu_opcode_t::LUI, [this](cpu_instr_t instr) { this->op_lui(instr); } },
        { cpu_opcode_t::ORI, [this](cpu_instr_t instr) { this->op_ori(instr); } },
        { cpu_opcode_t::SW, [this](cpu_instr_t instr) { this->op_sw(instr); } },
        { cpu_opcode_t::ADDIU, [this](cpu_instr_t instr) { this->op_addiu(instr); } },
        { cpu_opcode_t::J, [this](cpu_instr_t instr) { this->op_j(instr); } },
    };

    opspecmap = {
        { cpu_subfunc_t::SSL, [this](cpu_instr_t instr) { this->op_ssl(instr); } },
        { cpu_subfunc_t::OR, [this](cpu_instr_t instr) { this->op_or(instr); } },
    };
}

void ps1::cpu_t::tick() {
    cpu_instr_t instr = delay_slot; // fetch instruction from delay slot
    delay_slot = bus->fetch32(pc); // fetch instruction from memory

    pc += sizeof(cpu_instr_t); // advance program counter

    execute(instr); // execute instruction
}

void ps1::cpu_t::execute(cpu_instr_t instr) {
    auto opcode = static_cast <cpu_opcode_t> (instr.a.opcode);

    DEBUG_CODE(printf("Executing 0x%08X: 0x%08X\n", pc, (uint32_t)instr));

    if (opmap.contains(opcode)) {
        opmap[opcode](instr);

        return;
    }

    DEBUG_CODE(execute_err(instr));
}

void ps1::cpu_t::execute_special(cpu_instr_t instr) {
    auto subfunc = static_cast <cpu_subfunc_t> (instr.a.subfunc);

    if (opspecmap.contains(subfunc)) {
        opspecmap[subfunc](instr);

        return;
    }

    DEBUG_CODE(execute_err(instr));
}

void ps1::cpu_t::execute_err(cpu_instr_t instr) {
    DEBUG_CODE(char err_msg_buffer[64]);
    DEBUG_CODE(
        sprintf(
            err_msg_buffer,
            "COULD NOT EXECUTE 0x%08X: 0x%08X (opcode %d-%02X)",
            pc, (uint32_t)instr,
            (uint32_t)(bool)instr.a.opcode , instr.a.opcode ? instr.a.opcode : instr.a.subfunc
        )
    );
    // ASSERT(false, err_msg_buffer);

    DEBUG_CODE(logger::push(err_msg_buffer, "cpu"));

    halt();
}

void ps1::cpu_t::halt() {
    halted = true;

    DEBUG_CODE(logger::push("cpu halted!", "cpu"));
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
    bus->store32(get_reg(instr.b.rs) + sign_extend_16(instr.b.imm16), get_reg(instr.b.rt));
}

void ps1::cpu_t::op_ssl(cpu_instr_t instr) {
    set_reg(instr.a.rt, get_reg(instr.a.rs) << instr.a.imm5);
}

void ps1::cpu_t::op_addiu(cpu_instr_t instr) {
    set_reg(instr.b.rt, get_reg(instr.b.rs) + sign_extend_16(instr.b.imm16)); // * not a signed addition
}

void ps1::cpu_t::op_j(cpu_instr_t instr) {
    pc = (pc & 0xF0000000) | (instr.c.imm26 << 2);
}

void ps1::cpu_t::op_or(cpu_instr_t instr) {
    set_reg(instr.a.rd, get_reg(instr.a.rs) | get_reg(instr.a.rt));
}

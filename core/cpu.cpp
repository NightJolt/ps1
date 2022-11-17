#include "cpu.h"
#include "bus.h"

namespace ps1 {
    constexpr cpu_reg_t register_garbage_value = 0xDEADBEEF; // magic value for debugging
    constexpr cpu_reg_t bios_main_func_addr = 0xBFC00000; // BIOS main function address
}

ps1::cpu_instr_t::cpu_instr_t(uint32_t value) : raw(value) {}

ps1::cpu_instr_t::operator uint32_t() {
    return raw;
}

ps1::cpu_t::cpu_t(bus_t* bus) : bus(bus) {
    for (int i = 0; i < 32; i++) {
        regs[i] = register_garbage_value; // initialize registers to garbage value
    }

    regs[0] = 0; // $zero is always zero

    pc = bios_main_func_addr; // start at BIOS main function

    hi = register_garbage_value;
    lo = register_garbage_value;
}

void ps1::cpu_t::tick() {
    cpu_instr_t instr = bus->fetch32(pc); // fetch instruction from memory

    pc += sizeof cpu_instr_t; // advance program counter

    execute(instr); // execute instruction
}

void ps1::cpu_t::execute(cpu_instr_t instr) {
    printf("EXECUTING 0x%08X: 0x%08X\n", pc, (uint32_t)instr); // print instruction
}
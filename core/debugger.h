#pragma once

#include "defs.h"
#include "cpu.h"
#include "ram.h"

namespace ps1::debugger {
    void display_cpu_info(cpu_t*, bus_t*);
    void display_instr_view(cpu_t*, bus_t*);
    void display_ram_view(ram_t*);
}
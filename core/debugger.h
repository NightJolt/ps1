#pragma once

#include "defs.h"
#include "cpu.h"

namespace ps1::debugger {
    void display_cpu_info(cpu_t*);
    void display_instr_view(cpu_t*, bus_t*);
}
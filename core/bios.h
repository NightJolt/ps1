#pragma once

#include "defs.h"

namespace ps1 {
    class bios_t {
    public:
        bios_t(const str_t&);

        cpu_instr_t fetch_instruction(mem_addr_t) const;

    private:
        dyn_arr_t <uint8_t> data;
    };
}
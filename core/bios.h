#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    class bios_t : public peripheral_i {
    public:
        bios_t(const str_t&);
        ~bios_t() override = default;
        
        // peripheral_i
        uint32_t fetch32(mem_addr_t) const override;
        void store32(mem_addr_t, uint32_t) override;

    private:
        dyn_arr_t <uint8_t> data;
    };
}
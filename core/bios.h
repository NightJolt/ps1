#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    class bios_t : public peripheral_i {
    public:
        bios_t(const str_t&);
        ~bios_t() noexcept override = default;

        uint32_t fetch32(mem_addr_t) const override;

    private:
        dyn_arr_t <uint8_t> data;
    };
}
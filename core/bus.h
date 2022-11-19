#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    class bus_t : public peripheral_i {
    public:
        ~bus_t() override = default;

        void add_device(peripheral_i*, mem_range_t);

        // peripheral_i
        uint32_t fetch32(mem_addr_t) const override;
        void store32(mem_addr_t, uint32_t) override;
        
    private:
        dyn_arr_t <pair_t <mem_range_t, peripheral_i*>> devices;
    };
}
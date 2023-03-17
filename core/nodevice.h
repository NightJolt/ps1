#pragma once

#include "defs.h"
#include "peripheral.h"

namespace ps1 {
    class nodevice_t : public peripheral_i {
    public:
        ~nodevice_t() override = default;

        // peripheral_i
        uint32_t fetch32(mem_addr_t) const override;
        void store32(mem_addr_t, uint32_t) override;
        
    private:
    };
}
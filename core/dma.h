#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"

namespace ps1 {
    struct dma_t {
        struct channel_t { // ! members not to be rearranged
            uint32_t base; // * only [0:23] bits are used

            /*
            * sync 0:
            * [0:15] number of words
            * 
            * sync 1:
            * [0:15] block size
            * [16:31] block count
            */
            uint32_t block;

            union {
                struct {
                    uint32_t direction : 1; // * 0 = device to ram, 1 = ram to device
                    uint32_t addr_step : 1; // * 0 = increment, 1 = decrement 4 bytes
                    uint32_t _0 : 6;
                    uint32_t chopping_enable : 1;
                    uint32_t sync_mode : 2; // * 0 = manual/immediate, 1 = request, 2 = linked list, 3 = reserved
                    uint32_t _1 : 5;
                    uint32_t chopping_dma_window_size : 3;
                    uint32_t _2 : 1;
                    uint32_t chopping_cpu_window_size : 3;
                    uint32_t _3 : 1;
                    uint32_t start_busy : 1; // * 0 = stopped/completed, 1 = start/enable/busy
                    uint32_t _4 : 3;
                    uint32_t start_trigger : 1; // * 0 = normal, 1 = manual start; for sync mode 0
                    uint32_t _5 : 3;
                };

                uint32_t raw;
            } control;
        };

        union interrupt_t {
            interrupt_t() {}
            interrupt_t(uint32_t value) : raw(value) {}

            struct {
                uint32_t _0 : 6;
                uint32_t _1 : 9;
                uint32_t irq_force : 1;
                uint32_t irq_enable : 7;
                uint32_t irq_master_enable : 1;
                uint32_t irq_flag : 7;
                uint32_t irq_flag_master : 1; // ! not to be directly accessed
            };

            uint32_t raw;
        };

        enum struct port_t {
            mdecin = 0,
            mdecout = 1,
            gpu = 2,
            cdrom = 3,
            spu = 4,
            pio = 5,
            otc = 6
        };
        
        channel_t channel[7]; // * +0x00
        uint32_t control; // * +0x70
        interrupt_t interrupt; // * +0x74
    };

    void dma_init(dma_t*);
    void dma_exit(dma_t*);
    
    // void dma_save_state(dma_t*);
    // void dma_load_state(dma_t*);

    // * IF b15=1 OR (b23=1 AND (b16-22 AND b24-30)>0) THEN b31=1 ELSE b31=0
    // ! needs testing (its either == 0x7f or > 0)
    inline bool get_irq_flag_master(dma_t* dma) {
        return dma->interrupt.irq_force || (dma->interrupt.irq_master_enable && dma->interrupt.irq_enable == 0x7F && dma->interrupt.irq_flag == 0x7F);
    }

    inline uint32_t get_interrupt(dma_t* dma) {
        auto interrupt = dma->interrupt;
        interrupt.irq_flag_master = get_irq_flag_master(dma);

        return interrupt.raw;
    }

    inline void set_interrupt(dma_t* dma, dma_t::interrupt_t value) {
        value.irq_flag = (~value.irq_flag) & dma->interrupt.irq_flag;
        dma->interrupt = value;
    }

    FETCH_FN(dma_t) fetch(void* device, mem_addr_t offset) {
        DEBUG_CODE(logger::push("fetching", logger::type_t::warning, "dma"));

        dma_t* dma = (dma_t*)device;

        if (offset == 0x70) {
            return dma->control;
        } else if (offset == 0x74) {
            return get_interrupt(dma);
        } else {
            auto& channel = dma->channel[offset >> 4];
            uint32_t field = offset & 0xF;

            if (field == 0) {
                return channel.base;
            } else if (field == 4) {
                return channel.block;
            } else if (field == 8) {
                return channel.control.raw;
            }
        }

        ASSERT(false, "unhandled dma fetch");
        
        return 0;
    }

    STORE_FN(dma_t) store(void* device, mem_addr_t offset, type_t value) {
        DEBUG_CODE(logger::push("storing", logger::type_t::warning, "dma"));

        dma_t* dma = (dma_t*)device;

        if (offset == 0x70) {
            dma->control = value;
        } else if (offset == 0x74) {
            set_interrupt(dma, value);
        } else {
            auto& channel = dma->channel[offset >> 4];
            uint32_t field = offset & 0xF;

            if (field == 0) {
                channel.base = value & 0xFFFFFF;
            } else if (field == 4) {
                channel.block = value;
            } else if (field == 8) {
                channel.control.raw = value;
            } else {
                ASSERT(false, "unaligned dma store");
            }
        }
    }
}
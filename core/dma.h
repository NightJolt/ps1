#pragma once

#include "defs.h"
#include "peripheral.h"
#include "logger.h"
#include "ram.h"

namespace ps1 {
    struct dma_t {
        ram_t* ram;

        struct channel_t { // ! members not to be rearranged
            union control_t {
                bool is_active() {
                    return start_busy && ((uint32_t)sync_mode != 0 || start_trigger);
                }

                // * disable dma channel so it does not recopies data
                void disable() {
                    start_busy = false;
                    start_trigger = false;
                }

                enum struct transfer_dir_t : uint32_t {
                    device_to_ram = 0,
                    ram_to_device = 1,
                };

                enum struct sync_mode_t : uint32_t {
                    immediate = 0,      // * immediately transfer data
                    request = 1,        // * transfer after trigger signal
                    linked_list = 2,    // * transfer from linked list structure
                };

                struct {
                    transfer_dir_t direction : 1; // * 0 = device to ram, 1 = ram to device
                    uint32_t addr_step : 1; // * 0 = +4bytes, 1 = -4bytes
                    uint32_t _0 : 6;
                    uint32_t chopping_enable : 1;
                    sync_mode_t sync_mode : 2; // * 0 = manual/immediate, 1 = request, 2 = linked list, 3 = reserved
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
            };

            // * onlu for immediate and request sync modes
            uint32_t get_transfer_size() {
                return base * (control.sync_mode == control_t::sync_mode_t::immediate) ? block : 1;
            }
            
            uint32_t base; // * only [0:23] bits are used

            /*
            * sync 0:
            * [0:15] number of words
            * 
            * sync 1:
            * [0:15] block size in words
            * [16:31] block count
            */
            uint32_t block;
            control_t control;
        };

        union interrupt_t {
            void set(uint32_t v) {
                auto* value = (interrupt_t*)&v;

                value->irq_flag = (~value->irq_flag) & irq_flag;
                raw = *(uint32_t*)value;
            }
            
            // * IF b15=1 OR (b23=1 AND (b16-22 AND b24-30)>0) THEN b31=1 ELSE b31=0
            // ! needs testing (its either == 0x7f or > 0)
            uint32_t get() {
                irq_flag_master = irq_force || (irq_master_enable && irq_enable == 0x7F && irq_flag == 0x7F);

                return raw;
            }

            void clear() {
                raw = 0;
            }

            private: struct {
                uint32_t _0 : 6;
                uint32_t _1 : 9;
                uint32_t irq_force : 1;
                uint32_t irq_enable : 7;
                uint32_t irq_master_enable : 1;
                uint32_t irq_flag : 7;
                uint32_t irq_flag_master : 1; // ! not to be directly accessed
            };

            private: uint32_t raw;
        };

        enum struct port_t : uint32_t {
            mdecin = 0,
            mdecout = 1,
            gpu = 2,
            cdrom = 3,
            spu = 4,
            pio = 5,
            otc = 6
        };
        
        channel_t channels[7]; // * +0x00
        uint32_t control; // * +0x70
        interrupt_t interrupt; // * +0x74
    };

    void dma_init(dma_t*, ram_t*);
    void dma_exit(dma_t*);
    
    // void dma_save_state(dma_t*);
    // void dma_load_state(dma_t*);

    /*
    * we copy all data in one go without chopping
    * its not accurate but should not cause any issues either excluding some games
    */
    void dma_process_block_copy(dma_t*, uint32_t);
    void dma_process_linked_list(dma_t*, uint32_t);
    void dma_process(dma_t*, uint32_t);

    FETCH_FN(dma_t) fetch(void* device, mem_addr_t offset) {
        DEBUG_CODE(logger::push("fetching", logger::type_t::warning, "dma"));

        dma_t* dma = (dma_t*)device;

        if (offset == 0x70) {
            return dma->control;
        } else if (offset == 0x74) {
            return dma->interrupt.get();
        } else {
            auto& channel = dma->channels[offset >> 4];
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
            dma->interrupt.set(value);
        } else {
            uint32_t port = offset >> 4;
            auto& channel = dma->channels[port];
            uint32_t field = offset & 0xF;

            if (field == 0) {
                channel.base = value & 0xFFFFFF;
            } else if (field == 4) {
                channel.block = value;
            } else if (field == 8) {
                channel.control.raw = value;

                if (channel.control.is_active()) {
                    dma_process(dma, port);
                }
            } else {
                ASSERT(false, "unaligned dma store");
            }
        }
    }
}
#include "dma.h"
#include "ram.h"
#include "gpu.h"
#include "file.h"

void ps1::dma_init(dma_t* dma, ram_t* ram, gpu_t* gpu) {
    dma->ram = ram;
    dma->gpu = gpu;

    for (auto& channel : dma->channels) {
        channel.base = 0;
        channel.block = 0;
        channel.control.raw = 0;
    }

    dma->control = 0x07654321;
    dma->interrupt.clear();
}

void ps1::dma_exit(dma_t* dma) {}

void ps1::dma_save_state(dma_t* dma) {
    for (auto& channel : dma->channels) {
        file::write32(channel.base);
        file::write32(channel.block);
        file::write32(channel.control.raw);
    }

    file::write32(dma->control);
    file::write32(dma->interrupt.get_raw());
}

void ps1::dma_load_state(dma_t* dma) {
    for (auto& channel : dma->channels) {
        channel.base = file::read32();
        channel.block = file::read32();
        channel.control.raw = file::read32();
    }

    dma->control = file::read32();
    dma->interrupt.set_raw(file::read32());
}

namespace {
    constexpr uint32_t ignore_2_lsb_mask = 0x1ffffc;
    constexpr uint32_t term_addr = 0xffffff;
    constexpr uint32_t wrap_addr_mask = ps1::RAM_SIZE - 1;
}

void ps1::dma_process_block_copy(dma_t* dma, uint32_t port) {
    dma_t::channel_t& channel = dma->channels[port];
    int32_t step = channel.control.addr_step ? -4 : 4;
    int32_t size = channel.get_transfer_size();

    /*
    * two LSB is ignored. not documented on psx-spx
    * ref: https://github.com/libretro-mirrors/mednafen-git/blob/master/src/psx/dma.cpp
    */
    mem_addr_t addr = channel.base & ignore_2_lsb_mask;

    if (channel.control.direction == dma_t::channel_t::control_t::transfer_dir_t::device_to_ram) {
        while (size > 0) {
            switch(port) {
                case (uint32_t)dma_t::port_t::otc: {
                    uint32_t val = size == 1 ? term_addr : ((addr - 4) & wrap_addr_mask);

                    store<ram_t, uint32_t>((void*)dma->ram, addr, val);

                    break;
                }

                default: {
                    ASSERT(false, "unimplemented port. should not happen");
                }
            }

            addr += step;
            size--;
        }
    } else {
        while (size > 0) {
            switch(port) {
                case (uint32_t)dma_t::port_t::gpu: {
                    gp0(dma->gpu, fetch<ram_t, uint32_t>((void*)dma->ram, addr));

                    break;
                }

                default: {
                    ASSERT(false, "unimplemented port. should not happen");
                }
            }

            addr += step;
            size--;
        }
    }

    channel.control.disable();
}

void ps1::dma_process_linked_list(dma_t* dma, uint32_t port) {
    dma_t::channel_t& channel = dma->channels[port];
    mem_addr_t addr = channel.base & ignore_2_lsb_mask;

    if (channel.control.direction == dma_t::channel_t::control_t::transfer_dir_t::device_to_ram) {
        ASSERT(false, "device to ram not implemented");
    } else {
        ASSERT(port == (uint32_t)dma_t::port_t::gpu, "linked list only supported for gpu channel");

        while (true) {
            mem_addr_t header = fetch<ram_t, uint32_t>(dma->ram, addr);
            uint32_t data_size = header >> 24; // * size in words

            while (data_size > 0) {
                addr = (addr + 4) & ignore_2_lsb_mask;

                gp0(dma->gpu, fetch<ram_t, uint32_t>(dma->ram, addr));

                data_size--;
            }

            // * instead of checking against 0xffffff hardware probably check against bit 0x800000, which is not part of any valid address
            if (header & 0x800000) {
                break;
            }

            addr = header & ignore_2_lsb_mask; // * also ignores 8 MSB
        }
    }

    channel.control.disable();
}

void ps1::dma_process(dma_t* dma, uint32_t port)  {
    if (dma->channels[port].control.sync_mode == dma_t::channel_t::control_t::sync_mode_t::linked_list) {
        dma_process_linked_list(dma, port);
    } else {
        dma_process_block_copy(dma, port);
    }
}
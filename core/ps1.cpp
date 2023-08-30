#include "ps1.h"

#include "file.h"

#define SETUP_FETCH(type, info)\
    info.fetch32 = ps1::fetch<type, uint32_t>;\
    info.fetch16 = ps1::fetch<type, uint16_t>;\
    info.fetch8 = ps1::fetch<type, uint8_t>;

#define SETUP_STORE(type, info)\
    info.store32 = ps1::store<type, uint32_t>;\
    info.store16 = ps1::store<type, uint16_t>;\
    info.store8 = ps1::store<type, uint8_t>;

#define SETUP_STORE_FETCH(type, info)\
    SETUP_FETCH(type, info);\
    SETUP_STORE(type, info);

namespace {
    ps1::nodevice_t nodevice;

    void ps1_interconnect(ps1::ps1_t* console) {
        // * nodevice
        ps1::device_info_t nodevice_info;
        nodevice_info.device = &nodevice;
        SETUP_STORE_FETCH(ps1::nodevice_t, nodevice_info);

        // * bios
        ps1::device_info_t bios_info;
        bios_info.device = &console->bios;
        SETUP_FETCH(ps1::bios_t, bios_info);

        // * ram
        ps1::device_info_t ram_info;
        ram_info.device = &console->ram;
        SETUP_STORE_FETCH(ps1::ram_t, ram_info);

        // * hardware registers
        ps1::device_info_t hardreg_info;
        hardreg_info.device = &console->hardreg;
        hardreg_info.fetch32 = ps1::hardreg_fetch32;
        hardreg_info.store32 = ps1::hardreg_store32;

        // * expansion
        ps1::device_info_t expansion_info;
        expansion_info.device = &console->expansion;
        expansion_info.fetch8 = ps1::expansion_fetch8;

        // * gpu
        ps1::device_info_t gpu_info;
        gpu_info.device = &console->gpu;
        SETUP_STORE_FETCH(ps1::gpu_t, gpu_info);

        // * dma
        ps1::device_info_t dma_info;
        dma_info.device = &console->dma;
        SETUP_STORE_FETCH(ps1::dma_t, dma_info);

        // * important to map nodevices first to override subregions
        {
            // * Cache control registers
            nodevice_info.mem_range = { 0xFFFE0130, 4 };
            ps1::bus_connect(&console->bus, nodevice_info);

            // * SPU memory region
            nodevice_info.mem_range = { 0x1F801C00, 0x1F801E80 - 0x1F801C00 };
            ps1::bus_connect(&console->bus, nodevice_info);

            // * Expansion 2 memory region. Used for debugging
            nodevice_info.mem_range = { 0x1F802000, 0x1F802042 - 0x1F802000 };
            ps1::bus_connect(&console->bus, nodevice_info);

            // * Interrupt control registers. Not yet needed
            nodevice_info.mem_range = { 0x1F801070, 8 };
            ps1::bus_connect(&console->bus, nodevice_info);

            // * Timer
            nodevice_info.mem_range = { 0x1F801100, 0x1F80112F - 0x1F801100 };
            ps1::bus_connect(&console->bus, nodevice_info);
        }

        {
            gpu_info.mem_range = { 0x1F801810, 8 };
            ps1::bus_connect(&console->bus, gpu_info);

            // * DMA (Direct Memory Access)
            dma_info.mem_range = { 0x1F801080, 0x1F801100 - 0x1F801080 };
            ps1::bus_connect(&console->bus, dma_info);
        }

        bios_info.mem_range = { ps1::BIOS_ADDR, ps1::BIOS_SIZE };
        ps1::bus_connect(&console->bus, bios_info);
        
        ram_info.mem_range = { ps1::RAM_ADDR, ps1::RAM_SIZE };
        ps1::bus_connect(&console->bus, ram_info);

        hardreg_info.mem_range = { ps1::HARDREG_ADDR, ps1::HARDREG_SIZE };
        ps1::bus_connect(&console->bus, hardreg_info);

        expansion_info.mem_range = { ps1::EXPANSION1_ADDR, ps1::EXPANSION1_SIZE };
        ps1::bus_connect(&console->bus, expansion_info);
    }
}
    
void ps1::ps1_init(ps1_t* console, const str_t& bios_path) {
    ps1_interconnect(console);
    bios_init(&console->bios, bios_path);
    vram_init(&console->vram);
    ps1_soft_reset(console);
}

void ps1::ps1_exit(ps1_t* console) {
    cpu_exit(&console->cpu);
    bus_exit(&console->bus);
    ram_exit(&console->ram);
    dma_exit(&console->dma);
    vram_exit(&console->vram);
    bios_exit(&console->bios);
}

void ps1::ps1_soft_reset(ps1_t* console) {
    dma_exit(&console->dma);
    gpu_exit(&console->gpu);
    ram_exit(&console->ram);
    cpu_exit(&console->cpu);

    cpu_init(&console->cpu, &console->bus);
    ram_init(&console->ram);
    gpu_init(&console->gpu);
    dma_init(&console->dma, &console->ram, &console->gpu);
}

void ps1::ps1_save_state(ps1_t* console, const str_t& path) {
    file::open_writable(path);
    cpu_save_state(&console->cpu);
    ram_save_state(&console->ram);
    gpu_save_state(&console->gpu);
    dma_save_state(&console->dma);
    file::close_writable();
}

void ps1::ps1_load_state(ps1_t* console, const str_t& path) {
    file::open_readable(path);
    cpu_load_state(&console->cpu);
    ram_load_state(&console->ram);
    gpu_load_state(&console->gpu);
    dma_load_state(&console->dma);
    file::close_readable();
}
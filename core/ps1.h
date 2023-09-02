#pragma once

#include "defs.h"
#include "cpu.h"
#include "bios.h"
#include "bus.h"
#include "ram.h"
#include "hardreg.h"
#include "expansion.h"
#include "gpu.h"
#include "dma.h"
#include "nodevice.h"
#include "vram.h"

namespace ps1 {
    struct ps1_t {
        bus_t bus;
        cpu_t cpu;
        bios_t bios;
        ram_t ram;
        hardreg_t hardreg;
        expansion_t expansion;
        gpu_t gpu;
        dma_t dma;
        vram_t vram;
    };

    void ps1_init(ps1_t*, const str_t&);
    void ps1_exit(ps1_t*);

    void ps1_soft_reset(ps1_t*);

    /*
    * cpu
    * ram
    * gpu
    * dma
    ? vram: not really needed since we will get new frame instantly after launch
    */
    void ps1_save_state(ps1_t*, const str_t&);
    void ps1_load_state(ps1_t*, const str_t&);
}
#include "defs.h"
#include "cpu.h"
#include "bios.h"
#include "bus.h"
#include "hardreg.h"
#include "ram.h"
#include "nodevice.h"
#include "expansion.h"

#include "render.h"
#include "logger.h"
#include "debugger.h"

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

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu;

    ps1::cpu_init(&cpu, &bus);

    // ? use memory region masking instead ?

    ps1::bios_t bios;
    ps1::bios_init(&bios, "../bios/SCPH1001.bin"); 
    ps1::device_info_t bios_info;
    bios_info.device = &bios;
    SETUP_FETCH(ps1::bios_t, bios_info);

    ps1::ram_t ram;
    ps1::ram_init(&ram);
    ps1::device_info_t ram_info;
    ram_info.device = &ram;
    SETUP_STORE_FETCH(ps1::ram_t, ram_info);

    ps1::hardreg_t hardreg;
    ps1::device_info_t hardreg_info;
    hardreg_info.device = &hardreg;
    hardreg_info.fetch32 = ps1::hardreg_fetch32;
    hardreg_info.store32 = ps1::hardreg_store32;

    ps1::expansion_t expansion;
    ps1::device_info_t expansion_info;
    expansion_info.device = &expansion;
    expansion_info.fetch8 = ps1::expansion_fetch8;

    ps1::nodevice_t nodevice;
    ps1::device_info_t nodevice_info;
    nodevice_info.device = &nodevice;
    SETUP_STORE_FETCH(ps1::nodevice_t, nodevice_info);

    {
        // * important to map nodevices first to override subregions
        {
            // * Cache control registers
            nodevice_info.mem_range = { 0xFFFE0130, 4 };
            ps1::bus_connect(&bus, nodevice_info);

            // * SPU memory region
            nodevice_info.mem_range = { 0x1F801C00, 0x1F801E80 - 0x1F801C00 };
            ps1::bus_connect(&bus, nodevice_info);

            // * Expansion 2 memory region. Used for debugging
            nodevice_info.mem_range = { 0x1F802000, 0x1F802042 - 0x1F802000 };
            ps1::bus_connect(&bus, nodevice_info);

            // * Interrupt control registers. Not yet needed
            nodevice_info.mem_range = { 0x1F801070, 8 };
            ps1::bus_connect(&bus, nodevice_info);

            // * Timer
            nodevice_info.mem_range = { 0x1F801100, 0x1F80112F - 0x1F801100 };
            ps1::bus_connect(&bus, nodevice_info);
        }

        bios_info.mem_range = { ps1::BIOS_KSEG1, ps1::BIOS_SIZE };
        ps1::bus_connect(&bus, bios_info);

        ram_info.mem_range = { ps1::RAM_KUSEG, ps1::RAM_SIZE };
        ps1::bus_connect(&bus, ram_info);

        ram_info.mem_range = { ps1::RAM_KSEG0, ps1::RAM_SIZE };
        ps1::bus_connect(&bus, ram_info);

        ram_info.mem_range = { ps1::RAM_KSEG1, ps1::RAM_SIZE };
        ps1::bus_connect(&bus, ram_info);

        hardreg_info.mem_range = { ps1::HARDREG_KUSEG, ps1::HARDREG_SIZE };
        ps1::bus_connect(&bus, hardreg_info);

        expansion_info.mem_range = { ps1::EXPANSION1_KUSEG, ps1::EXPANSION1_SIZE };
        ps1::bus_connect(&bus, expansion_info);
    }

    ps1::render::init();

    while (!ps1::render::should_close()) {
        ps1::render::begin_frame();

        if (cpu.state == ps1::cpu_state_t::running) ps1::cpu_tick(&cpu);

        ps1::logger::display();
        ps1::debugger::display_cpu_info(&cpu);
        ps1::debugger::display_instr_view(&cpu, &bus);
        ps1::debugger::display_ram_view(&ram);
        ps1::render::end_frame();
    }

    ps1::render::exit();

    ps1::bios_del(&bios);
    ps1::ram_del(&ram);
    
    return 0;
}
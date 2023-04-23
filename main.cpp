#include "defs.h"
#include "cpu.h"
#include "bios.h"
#include "bus.h"
#include "hardreg.h"
#include "ram.h"
#include "nodevice.h"

#include "render.h"
#include "logger.h"
#include "debugger.h"

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu;

    ps1::cpu_init(&cpu, &bus);

    // ? use memory region masking instead ?

    ps1::bios_t bios;
    ps1::bios_init(&bios, "../bios/SCPH1001.bin");
    ps1::device_info_t bios_info;
    bios_info.device = &bios;
    bios_info.fetch32 = ps1::bios_fetch32;
    bios_info.store32 = ps1::bios_store32;

    ps1::ram_t ram;
    ps1::ram_init(&ram);
    ps1::device_info_t ram_info;
    ram_info.device = &ram;
    ram_info.fetch32 = ps1::ram_fetch32;
    ram_info.store32 = ps1::ram_store32;

    ps1::hardreg_t hardreg;
    ps1::device_info_t hardreg_info;
    hardreg_info.device = &hardreg;
    hardreg_info.fetch32 = ps1::hardreg_fetch32;
    hardreg_info.store32 = ps1::hardreg_store32;

    ps1::nodevice_t nodevice;
    ps1::device_info_t nodevice_info;
    nodevice_info.device = &nodevice;
    nodevice_info.fetch32 = ps1::nodevice_fetch32;
    nodevice_info.store32 = ps1::nodevice_store32;
    nodevice_info.store16 = ps1::nodevice_store16;

    {
        // * important to map nodevices first to override subregions
        {
            nodevice_info.mem_range = { 0xFFFE0130, 4 };
            ps1::bus_connect(&bus, nodevice_info);

            // * PSU memory region
            nodevice_info.mem_range = { 0x1F801C00, 0x1F801E80 - 0x1F801C00 };
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
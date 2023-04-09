#include "defs.h"
#include "cpu.h"
#include "bios.h"
#include "bus.h"
#include "hardreg.h"
#include "nodevice.h"

#include "render.h"
#include "logger.h"

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu(&bus);

    ps1::bios_t bios("../bios/SCPH1001.bin");
    ps1::hardreg_t hardreg;

    bus.add_device(&bios,       { ps1::BIOS_KSEG1,      ps1::BIOS_SIZE });
    bus.add_device(&hardreg,    { ps1::HARDREG_KUSEG,   ps1::HARDREG_SIZE });

    ps1::nodevice_t nodevice;
    bus.add_device(&nodevice,   { 0xFFFE0130,           4 });

    ps1::render::init();

    while (!ps1::render::should_close()) {
        ps1::render::begin_frame();

        if (!cpu.halted) cpu.tick();

        ps1::logger::display();
        ps1::render::end_frame();
    }

    ps1::render::exit();
    
    return 0;
}
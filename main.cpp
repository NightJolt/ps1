#include "core/defs.h"
#include "core/cpu.h"
#include "core/bios.h"
#include "core/bus.h"
#include "core/hardreg.h"

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu(&bus);

    ps1::bios_t bios("../bios/SCPH1001.bin");
    ps1::hardreg_t hardreg;

    bus.add_device(&bios,       { ps1::BIOS_KSEG1, ps1::BIOS_SIZE });
    bus.add_device(&hardreg,    { ps1::HARDREG_KUSEG, ps1::HARDREG_SIZE });
    
    while (true) {
        cpu.tick();
    }
    
    return 0;
}
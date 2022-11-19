#include "core/cpu.h"
#include "core/bios.h"
#include "core/bus.h"

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu(&bus);
    ps1::bios_t bios("../bios/SCPH1001.bin");

    bus.add_device(&bios, { 0xBFC00000, 0x80000 });
    
    while (true) {
        cpu.tick();
    }
    
    return 0;
}
#include "bios.h"
#include "file.h"
#include "logger.h"

void ps1::bios_init(bios_t* bios, const str_t& path) {
    bios->data = file::read_binary(path);
    
    ASSERT(bios->data, "Failed to load BIOS binary");

    DEBUG_CODE(logger::push("bios loaded", logger::type_t::info, "bios"));
}

void ps1::bios_exit(bios_t* bios) {
    delete[] bios->data;
}
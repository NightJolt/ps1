#include "ram.h"
#include "logger.h"

void ps1::ram_init(ram_t* ram) {
    ram->data = new uint8_t[RAM_SIZE];
}

void ps1::ram_exit(ram_t* ram) {
    delete[] ram->data;
}

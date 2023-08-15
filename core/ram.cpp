#include "ram.h"
#include "logger.h"
#include "file.h"

void ps1::ram_init(ram_t* ram) {
    ram->data = new uint8_t[RAM_SIZE];
}

void ps1::ram_exit(ram_t* ram) {
    delete[] ram->data;
}

void ps1::ram_save_state(ram_t* ram) {
    file::write(ram->data, RAM_SIZE);
}

void ps1::ram_load_state(ram_t* ram) {
    file::read(ram->data, RAM_SIZE);
}
#include "ram.h"
#include "logger.h"

void ps1::ram_init(ram_t* ram) {
    ram->data = new uint8_t[RAM_SIZE];
}

void ps1::ram_del(ram_t* ram) {
    delete[] ram->data;
}

uint32_t ps1::ram_fetch32(void* ram, mem_addr_t offset) {
    return *(uint32_t*)(((ram_t*)ram)->data + offset);
}

void ps1::ram_store32(void* ram, mem_addr_t offset, uint32_t value) {
    *(uint32_t*)(((ram_t*)ram)->data + offset) = value;
}
#include "file.h"

#include <fstream>

uint8_t* ps1::read_binary(const str_t& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) return nullptr;

    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    uint8_t* data = new uint8_t[size];
    file.read((char*)data, size);

    return data;
}
#include "file.h"

#include <fstream>
#include <limits>

namespace {
    uint8_t* read_file(const str_t& path, uint32_t flags) {
        std::ifstream file(path, flags);

        if (!file.is_open()) return nullptr;

        file.ignore(std::numeric_limits<std::streamsize>::max());
        uint32_t size = file.gcount();
        file.clear();
        file.seekg(0, std::ios::beg);

        uint8_t* data = new uint8_t[size];
        file.read((char*)data, size);

        return data;
    }
}

uint8_t* ps1::file::read_binary(const str_t& path) {
    return read_file(path, std::ios::binary);
}

std::string ps1::file::read_text(const str_t& path) {
    std::ifstream file(path);
    std::string data;
    std::string line;

    if (!file.is_open()) return nullptr;

    while (std::getline(file, line)) {
        data += line + "\n";
    }

    return data;
}

namespace {
    std::ofstream writable;
    std::ifstream readable;
}

void ps1::file::open_writable(const str_t& path) {
    ::writable.open(path, std::ios::binary | std::ios::trunc);
}

void ps1::file::write(uint8_t* ptr, size_t size) {
    ::writable.write((char*)ptr, size);
}

void ps1::file::write32(uint32_t value) {
    ::writable.write((char*)&value, sizeof(value));
}

void ps1::file::close_writable() {
    ::writable.close();
}

void ps1::file::open_readable(const str_t& path) {
    ::readable.open(path, std::ios::binary);
}

void ps1::file::read(uint8_t* ptr, size_t size) {
    ::readable.read((char*)ptr, size);
}

uint32_t ps1::file::read32() {
    uint32_t value;
    ::readable.read((char*)&value, sizeof(value));
    return value;
}

void ps1::file::close_readable() {
    ::readable.close();
}
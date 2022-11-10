#include "file.h"

#include <fstream>

optional_t <dyn_arr_t <uint8_t>> ps1::read_binary(const str_t& path) {
    dyn_arr_t <uint8_t> data;

    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) return std::nullopt;

    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    data.resize(size);
    file.read(reinterpret_cast <char*> (data.data()), size);

    return std::move(data);
}
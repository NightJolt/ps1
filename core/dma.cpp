#include "dma.h"


void ps1::dma_init(dma_t* dma, ram_t* ram) {
    dma->ram = ram;

    for (auto& channel : dma->channels) {
        channel.base = 0;
        channel.block = 0;
        channel.control.raw = 0;
    }

    dma->control = 0x07654321;
    dma->interrupt.clear();
}

void ps1::dma_exit(dma_t* dma) {}

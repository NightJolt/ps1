#include "dma.h"


void ps1::dma_init(dma_t* dma) {
    for (auto& channel : dma->channel) {
        channel.base = 0;
        channel.block = 0;
        channel.control.raw = 0;
    }

    dma->control = 0x07654321;
    dma->interrupt.raw = 0;
}

void ps1::dma_exit(dma_t* dma) {}

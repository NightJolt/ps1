#include "dma.h"


void ps1::dma_init(dma_t* dma) {
    dma->control = 0x07654321;
}

void ps1::dma_exit(dma_t* dma) {}

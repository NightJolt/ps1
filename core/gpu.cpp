#include "gpu.h"

void ps1::gpu_init(gpu_t* gpu) {
    gpu->stat.raw = 0;
    gpu->stat.display_disable = 1;

    // ! temp override
    gpu->stat.ready_to_receive_cmd = 1;
    gpu->stat.ready_to_send_vram_to_cpu = 1;
    gpu->stat.ready_to_recieve_dma_block = 1;
}

void ps1::gpu_exit(gpu_t* gpu) {}

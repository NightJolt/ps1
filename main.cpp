#include "defs.h"

#include "ps1.h"

#include "cpu.h"
#include "bios.h"
#include "bus.h"
#include "hardreg.h"
#include "ram.h"
#include "nodevice.h"
#include "expansion.h"
#include "gpu.h"

#include "render.h"
#include "logger.h"
#include "debugger.h"

int main() {
    ps1::render::init();

    ps1::render::load_vertex_shader("../core/shaders/vertex.glsl");
    ps1::render::load_fragment_shader("../core/shaders/fragment.glsl");

    ps1::ps1_t console;
    ps1::ps1_init(&console, "../bios/SCPH1001.bin");

    while (!ps1::render::should_close()) {
        ps1::render::begin_frame();

        if (console.cpu.state == ps1::cpu_state_t::running) ps1::cpu_tick(&console.cpu);

        ps1::debugger::display(&console);

        ps1::render::end_frame();
    }

    ps1::ps1_exit(&console);

    ps1::render::exit();
    
    return 0;
}
#include "defs.h"

#include "ps1.h"
#include "emulation.h"
#include "render.h"
#include "logger.h"
#include "debugger.h"

int main() {
    ps1::render::init();
    ps1::render::make_shader("../core/shaders/vertex.glsl", "../core/shaders/fragment.glsl", 0);
    ps1::render::use_shader(0);

    ps1::ps1_t console;
    ps1::ps1_init(&console, "../bios/SCPH1001.bin");

    ps1::emulation_settings_t settings;
    settings.instr_per_frame = 30000;

    while (!ps1::render::should_close()) {
        ps1::render::begin_frame();

        if (console.cpu.state == ps1::cpu_state_t::running) {
            for (uint32_t i = 0; i < settings.instr_per_frame; i++) {
                ps1::cpu_tick(&console.cpu);
            }
        }

        ps1::debugger::display(&console, &settings);

        ps1::render::end_frame();
    }

    ps1::ps1_exit(&console);

    ps1::render::exit();
    
    return 0;
}
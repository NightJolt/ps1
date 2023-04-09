#include "core/defs.h"
#include "core/cpu.h"
#include "core/bios.h"
#include "core/bus.h"
#include "core/hardreg.h"
#include "core/nodevice.h"

#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

int main() {
    ps1::bus_t bus;
    ps1::cpu_t cpu(&bus);

    ps1::bios_t bios("../bios/SCPH1001.bin");
    ps1::hardreg_t hardreg;

    bus.add_device(&bios,       { ps1::BIOS_KSEG1, ps1::BIOS_SIZE });
    bus.add_device(&hardreg,    { ps1::HARDREG_KUSEG, ps1::HARDREG_SIZE });

    ps1::nodevice_t nodevice;
    bus.add_device(&nodevice,   { 0xFFFE0130, 4 });

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(640, 480, "ps1", NULL, NULL);
    glfwMakeContextCurrent(window);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // cpu.tick();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::End();

        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
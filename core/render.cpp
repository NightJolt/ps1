#include "render.h"

namespace {
    GLFWwindow* window;
}

GLFWwindow* ps1::render::init() {
    glfwInit();
    ::window = glfwCreateWindow(1280, 720, "ps1", NULL, NULL);
    glfwMakeContextCurrent(::window);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(::window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    return ::window;
}

void ps1::render::exit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(::window);
    glfwTerminate();
}

void ps1::render::begin_frame() {
    glfwPollEvents();
        
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ps1::render::end_frame() {
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(::window);
}

bool ps1::render::should_close() {
    return glfwWindowShouldClose(::window);
}
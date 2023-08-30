#include "render.h"
#include "logger.h"
#include "file.h"

namespace {
    GLFWwindow* window;
}

GLFWwindow* ps1::render::init() {
    glfwInit();
    ::window = glfwCreateWindow(1536, 864, "ps1", NULL, NULL);
    glfwMakeContextCurrent(::window);
    glfwSwapInterval(0);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(::window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();

    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

    glewInit();

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

namespace {
    uint32_t vertex_shader;
    uint32_t fragment_shader;

    void load_shader(const char* path, uint32_t shader_type, uint32_t* shader) {
        *shader = glCreateShader(shader_type);

        str_t code = ps1::file::read_text(path);
        const char* code_ptr = code.c_str();

        glShaderSource(*shader, 1, &code_ptr, nullptr);
        glCompileShader(*shader);

        int32_t res = GL_FALSE;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &res);
        int32_t log_length = 0;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            std::vector<char> error_log(log_length);

            glGetShaderInfoLog(*shader, log_length, &log_length, &error_log[0]);

            std::string log;
            for (int i = 0; i < error_log.size(); i++) {
                log += error_log[i];
            }

            ps1::logger::push(log, ps1::logger::type_t::error, "render");
        }

        if (res == GL_TRUE) {
            ps1::logger::push("successfully compiled shader", ps1::logger::type_t::info, "render");
        } else {
            ps1::logger::push("failed to compile shader", ps1::logger::type_t::error, "render");
        }
    }
}

void ps1::render::load_vertex_shader(const char* path) {
    load_shader(path, GL_VERTEX_SHADER, &vertex_shader);
}

void ps1::render::load_fragment_shader(const char* path) {
    load_shader(path, GL_FRAGMENT_SHADER, &fragment_shader);
}

uint32_t ps1::render::get_vertex_shader() {
    return vertex_shader;
}

uint32_t ps1::render::get_fragment_shader() {
    return fragment_shader;
}

#include "render.h"
#include "logger.h"
#include "file.h"

namespace {
    constexpr uint32_t window_width = 1536;
    constexpr uint32_t window_height = 864;

    GLFWwindow* window;
}

GLFWwindow* ps1::render::init() {
    glfwInit();
    ::window = glfwCreateWindow(window_width, window_height, "ps1", NULL, NULL);
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

    glViewport(0, 0, window_width, window_height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ps1::render::end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(::window);
}

bool ps1::render::should_close() {
    return glfwWindowShouldClose(::window);
}

namespace {
    uint32_t load_shader(const char* path, uint32_t shader_type) {
        uint32_t shader = glCreateShader(shader_type);

        str_t code = ps1::file::read_text(path);
        const char* code_ptr = code.c_str();

        glShaderSource(shader, 1, &code_ptr, nullptr);
        glCompileShader(shader);

        int32_t res = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
        int32_t log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            std::vector<char> error_log(log_length);

            glGetShaderInfoLog(shader, log_length, &log_length, &error_log[0]);

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

        return shader;
    }
}

uint32_t ps1::render::make_shader(const char* vertex_shader_path, const char* fragment_shader_path) {
    uint32_t vertex_shader = load_shader(vertex_shader_path, GL_VERTEX_SHADER);
    uint32_t fragment_shader = load_shader(fragment_shader_path, GL_FRAGMENT_SHADER);

    uint32_t program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glValidateProgram(program);
    glUseProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

set(LIB_NAME ps1_libs)

file (
    GLOB_RECURSE LIB_FILES

    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include/*.h

    ${CMAKE_CURRENT_SOURCE_DIR}/glew/include/*.h

    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/include/*.cpp
)

add_library(${LIB_NAME} ${LIB_FILES})

find_package(OpenGL REQUIRED)

target_link_libraries(
    ${LIB_NAME}

    ${OPENGL_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/win64/glfw3.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/glew/win64/glew32s.lib
)

target_include_directories(${LIB_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libs/glfw/include)
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libs/glew/include)
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libs/imgui/include)

target_compile_definitions(${LIB_NAME} PUBLIC GLEW_STATIC)

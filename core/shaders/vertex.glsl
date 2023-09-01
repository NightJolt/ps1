#version 330

layout(location = 0) in vec4 vert_pos;
layout(location = 1) in vec3 vert_col;

out vec3 color;

void main() {
    gl_Position = vert_pos;
    color = vert_col;
}
#version 330

layout(location = 0) in ivec4 vert_pos;
layout(location = 1) in uvec3 vert_col;

out vec3 color;

void main() {
    float x = float(vert_pos.x) / 512.0 - 1.0;
    float y = 1.0 - float(vert_pos.y) / 256.0;

    gl_Position.xyzw = vec4(x, y, 0.0, 1.0);

    color = vec3(float(vert_col.r) / 255.0, float(vert_col.g) / 255.0, float(vert_col.b) / 255.0);
}
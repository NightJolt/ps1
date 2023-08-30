#version 330

in ivec2 vertex_pos;
in uvec3 vertex_col;

out vec3 color;

void main() {
    float x = float(vertex_pos.x) / 512.0 - 1.0;
    float y = 1.0 - float(vertex_pos.y) / 256.0;

    gl_Position.xyzw = vec4(x, y, 0.0, 1.0);

    color = vec3(float(vertex_col.r) / 255.0, float(vertex_col.g) / 255.0, float(vertex_col.b) / 255.0);
}
#version 460 core

const vec4 VERTEX_LOCATIONS[6] = {
	vec4(-1.0, 1.0, 0, 1.0),
	vec4(1.0, 1.0, 0, 1.0),
	vec4(-1.0, -1.0, 0, 1.0),
	vec4(-1.0, -1.0, 0, 1.0),
	vec4(1.0, 1.0, 0, 1.0),
	vec4(1.0, -1.0, 0, 1.0)
};

layout(location = 0) in float pos_x;
layout(location = 1) in float pos_y;
layout(location = 2) in float u;
layout(location = 3) in float v;

out vec2 uv;

void main() {
	uv = vec2(u, v);
	gl_Position = vec4(pos_x, pos_y, 0, 1.0);
}
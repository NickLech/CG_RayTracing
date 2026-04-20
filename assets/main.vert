#version 460 core

const vec4 VERTEX_LOCATIONS[6] = {
	vec4(-1.0, 1.0, 0, 1.0),
	vec4(1.0, 1.0, 0, 1.0),
	vec4(-1.0, -1.0, 0, 1.0),
	vec4(-1.0, -1.0, 0, 1.0),
	vec4(1.0, 1.0, 0, 1.0),
	vec4(1.0, -1.0, 0, 1.0)
};

void main() {
	gl_Position = VERTEX_LOCATIONS[gl_VertexID];
}
#version 460 core

in vec2 uv;

layout(binding = 0) uniform sampler2D my_texture;

layout(location = 0) out vec4 color;

void main() {
	color = vec4(texture2D(my_texture, uv).xyz, 1.0);
}
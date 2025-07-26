#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_texture_coordinates;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D texture_sampler;

void main() {
	out_color = vec4(frag_color * texture(texture_sampler, frag_texture_coordinates).rgb, 1.0);
}

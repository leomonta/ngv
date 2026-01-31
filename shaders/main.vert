#version 450

layout(binding = 0) uniform MVP {
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_coords;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_tex_coords;

void main() {
	gl_Position = vec4(in_position, 1.);
	out_color = in_color;
	out_tex_coords = in_tex_coords;
	gl_PointSize = 2.;
}

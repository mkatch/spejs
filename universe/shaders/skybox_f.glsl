#version 460

precision highp float;

uniform sampler3D luminance;

in vec3 f_tex_coords;

out lowp vec4 frag_color;

void main() {
	lowp float L = texture(luminance, f_tex_coords).r;
	frag_color = vec4(L, 0, 0, 0.02 * L);
}
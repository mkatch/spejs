#version 460

uniform mat4 ProjectionView;
uniform mat4 Model;

in vec3 position;

out vec3 f_tex_coords;

void main() {
	vec4 m_position = Model * vec4(position, 1.0);
	f_tex_coords = m_position.xyz;	
	gl_Position = ProjectionView * m_position;
}
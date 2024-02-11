#version 460

uniform mat4 projection;
uniform mat4 model;
in vec3 position;
in vec3 normal;
out vec3 frag_normal;

void main()
{
  frag_normal = normal;
  gl_Position = projection * model * vec4(position, 1.0);
}

#version 460

in vec3 position;
in vec3 normal;
out vec3 frag_normal;

void main()
{
  frag_normal = normal;
  gl_Position = vec4(position, 1.0);
}

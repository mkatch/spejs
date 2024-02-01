#version 460

in vec4 position;
in vec4 color;
out vec4 frag_color;

void main()
{
  frag_color = color;
  gl_Position = position;
}

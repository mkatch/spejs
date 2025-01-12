#version 460

in vec4 frag_color;

out lowp vec4 _frag_color;

void main() {
  _frag_color = frag_color;
}

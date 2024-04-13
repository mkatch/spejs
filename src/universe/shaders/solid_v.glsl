#version 460

uniform mat4 Projection;
uniform mat4 Model;
uniform mat3 _Normal_model;
in vec3 position;
in vec3 normal;

out vec3 frag_position;
out vec3 frag_normal;

void main()
{
  vec4 model_position = Model * vec4(position, 1.0);
  frag_normal = _Normal_model * normal;
  frag_position = model_position.xyz;
  gl_Position = Projection * model_position;
}

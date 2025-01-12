#version 460

precision highp float;

uniform vec4 color;

uniform vec3 ambient_color;
uniform vec3 light0_position;
uniform vec3 light0_color;
uniform vec3 light1_position;
uniform vec3 light1_color;

in vec3 frag_position;
in vec3 frag_normal;

out lowp vec4 frag_color;

vec3 compute_light(vec3 light_position, vec3 light_color, vec3 n) {
  vec3 light_r = light_position - frag_position;
  float d = length(light_r);
  float diffuse = 15.0 * max(dot(frag_normal, light_r / d), 0.0) / (d * d);
  return light_color * diffuse;
}

void main() {
  vec3 n = normalize(frag_normal);
  frag_color = vec4(
    color.rgb * (
      ambient_color +
      compute_light(light0_position, light0_color, n) +
      compute_light(light1_position, light1_color, n)
    ),
    color.w
  );
}

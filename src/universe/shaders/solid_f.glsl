uniform vec4 color;
in vec3 frag_normal;

void main() {
  gl_FragColor = vec4(
    (0.5 + 0.5 * normalize(frag_normal)) * color.xyz,
    color.w
  );
}

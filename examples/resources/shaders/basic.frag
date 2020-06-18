#version 330
in vec2 pass_texcoord;

uniform sampler2D tex;

out vec4 out_color;

void main() {
  vec4 sample_color = texture(tex, pass_texcoord);

  if (sample_color.a == 0)
    discard;

  out_color = sample_color;
}

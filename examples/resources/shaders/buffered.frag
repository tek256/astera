#version 330

in vec2 pass_texcoord;
in vec4 pass_color;

uniform sampler2D sample_tex;

out vec4 out_color;

void main() {
  vec4 sample_color = texture(sample_tex, pass_texcoord);

  if (sample_color.a == 0) {
    discard;
  }

  out_color = sample_color * pass_color;
}


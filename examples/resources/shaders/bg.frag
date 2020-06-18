#version 330

in vec2 pass_texcoord;

uniform sampler2D tex;

out vec4 out_c;

void main() {
  out_c = texture(tex, pass_texcoord);
}

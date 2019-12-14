#version 330
in vec2 pass_texcoords;

uniform sampler2D screen_tex;
uniform float gamma = 2.2;

out vec4 out_color;

void main(){
  vec3 sample_color = texture(screen_tex, pass_texcoords).rgb;
  vec3 mapped = pow(sample_color, vec3(1.0 / gamma));
  //out_color = vec4(1.0, 0.0, 0.0, 1.0);
  //float avg = 0.2126 * sample_color.r + 0.7152 * sample_color.g + 0.0722 * sample_color.b;
  out_color = vec4(sample_color, 1.0);
}

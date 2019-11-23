#version 330
in vec2 pass_texcoords;

uniform sampler2D screen_tex;

out vec4 out_color;

void main(){
  vec3 sample_color = texture(screen_tex, pass_texcoords).rgb;
  //out_color = vec4(1.0, 0.0, 0.0, 1.0);
  out_color = vec4(sample_color, 1.0);
}

#version 330
in vec2 pass_texcoords;

uniform sampler2D screen_tex;
uniform float gamma = 2.2;

out vec4 out_color;

void main(){
  out_color = vec4(1.0);
  vec4 sample_color = vec4(texture(screen_tex, pass_texcoords).rgb, 1.0);

  vec3 gamma_corrected = pow(sample_color.rgb, vec3(1.0 / gamma));

  float amp = gamma_corrected.r + gamma_corrected.g + gamma_corrected.b;
  vec3 bw = vec3(amp);

  //out_color = sample_color; 
  out_color = vec4(bw, 1.0);
}

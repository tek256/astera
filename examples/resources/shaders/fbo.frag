#version 330
in vec2 pass_texcoords;

uniform sampler2D screen_tex;
uniform float gamma = 1.0;

out vec4 out_color;

void main(){
  vec4 sample_color = texture(screen_tex, pass_texcoords);

  if(sample_color.a == 0)
    discard;

  //vec3 gamma_corrected = pow(sample_color.rgb, vec3(1.0 / gamma));

  //vec4 clear = vec4(0);
  out_color = sample_color; 
}

#version 330
in vec2 pass_texcoords;

uniform sampler2D screen_tex;
uniform float gamma = 1.0;

uniform vec2 resolution;

uniform float use_vig = 0.f;
uniform float vig_intensity = 15.f;
uniform float vig_scale = 0.25f;

out vec4 out_color;

vec4 vig(in vec2 frag_coord){
  vec2 uv = frag_coord / resolution;
  uv *= 1.0 - uv.yx;
  float vig = uv.x * uv.y * vig_intensity;
  vig = pow(vig, vig_scale);
  return vec4(vig);
}

void main(){
  vec4 sample_color = texture(screen_tex, pass_texcoords);

  if(sample_color.a == 0)
    discard;

  //vec3 gamma_corrected = pow(sample_color.rgb, vec3(1.0 / gamma));

  //vec4 clear = vec4(0);
  if(use_vig > 0.f){
    out_color = sample_color * vig(gl_FragCoord.xy); 
  } else {
    out_color = sample_color;
  }
}

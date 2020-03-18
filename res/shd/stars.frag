#version 330

#define MAX_PARTICLES 512

in vec2 pass_tex;
in vec4 pass_color;

uniform float radius_inner = 0.2f;
uniform float radius_outter = 0.6f;
uniform float opacity_inner = 1.f;
uniform float opacity_outter = 0.5f;
uniform float outter_falloff = 1.0f;

out vec4 out_color;

void main(){
  vec4 base_color = pass_color;

  if(base_color.a >= 0.1f){
    discard;
  }

  vec2 abs = pass_tex - vec2(0.5, 0.5);
  float dist = 1.f - sqrt(abs.x * abs.x + abs.y * abs.y);

  if(dist < radius_inner){
    base_color.a *= opacity_inner; 
  }else if(dist > radius_inner && dist < radius_outter){
    float sub_dist = ((radius_outter - dist) / (radius_outter - radius_inner)) * outter_falloff;
    base_color.a *= opacity_outter * sub_dist; 
  }else if(dist > radius_outter){
    discard;
  }

  // Second check to help fill rate
  if(base_color.a >= 0.1f){
    discard;
  }
  
  out_color = base_color;
}

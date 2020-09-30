#version 330
layout(location = 0) in vec3 in_vert;
layout(location = 1) in vec2 in_texc;

uniform float depth_offset = 0.0f;

out vec2 pass_texcoords;

void main(){
  pass_texcoords = in_texc;
  vec4 position = vec4(in_vert, 1.0);
  position.z += depth_offset;

  gl_Position = position;
}

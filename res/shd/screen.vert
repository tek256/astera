#version 330
layout (location = 0) in vec4 in_vert;

out vec2 pass_texcoords;

void main(){
  pass_texcoords = in_vert.zw;
  //gl_Position = proj * model * vec4(in_vert.x, in_vert.y, 0.0, 1.0);
  gl_Position = vec4(in_vert.x, in_vert.y, 0.0, 1.0);
}

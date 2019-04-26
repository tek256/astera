#version 330

layout(location = 0) in vec3 in_pos;
layout(location = 2) in vec2 in_texc;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform int  sub_tex = 0;
uniform vec2 tex_size;
uniform vec2 sub_size;
uniform int  tex_id;

uniform int  flip_x = 0;
uniform int  flip_y = 0;

out vec2 o_texc;

int main(void){
  vec2 m_texc = in_texCoord;

  if(flip_x == 1){
    m_texc.x = 1 - m_texc.x;
  }

  if(flip_y == 1){
    m_texc.y = 1 - m_texc.y;
  }

  if(sub_texture == 1){
    int per_width = int(tex_size.x / sub_size.x);
    vec2 offset = vec2(sub_size.x * (tex_id % per_width), sub_size.y * (tex_id / per_width));
    m_texc = (offset / tex_size) + (sub_size / tex_size) * m_texc;
  }else{
    m_texc = in_texCoord;
  }

  gl_Position = projection * view * model * vec4(in_position, 1.0f);
  o_texc = m_texc;
}

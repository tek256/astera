#version 330

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texc;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 model;

uniform int  flip_x;
uniform int  flip_y;
uniform vec4 coords;
uniform vec4 color;

out vec2 pass_texcoord;
out vec4 pass_color;

void main() {
  vec2 mod_coord = in_texc;
  vec4 raw_coord = coords;

  if (flip_x == 1) {
    mod_coord.x = 1.0 - mod_coord.x;
  }

  if (flip_y == 1) {
    mod_coord.y = 1.0 - mod_coord.y;
  }

  vec2 tex_size = vec2(raw_coord.w - raw_coord.y, raw_coord.z - raw_coord.x);

  vec2 offset = raw_coord.xy;

  pass_texcoord = offset + (tex_size *  mod_coord);
  pass_color = color;

  gl_Position = projection * view * model * vec4(in_pos, 1.0f);
}

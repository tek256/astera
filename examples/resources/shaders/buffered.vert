#version 330 
// NOTE: This shader is meant for use with Uniform Buffers, not just array style
// uniforms

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texc;

uniform mat4 projection;
uniform mat4 view;

layout(std140) uniform sprite_data {
  vec4 coords;
  vec4 colors;
  int  flip_x;
  int  flip_y;
  mat4 models;
} sprites;

out vec2 pass_texcoord;
out vec4 pass_color;

void main() {
  vec2 mod_coord = in_texc;
  vec4 raw_coord = coords[gl_InstanceID];

  if (flip_x[gl_InstanceID] == 1) {
    mod_coord.x = 1.0 - mod_coord.x;
  }

  if (flip_y[gl_InstanceID] == 1) {
    mod_coord.y = 1.0 - mod_coord.y;
  }

  vec2 fix = vec2(sign(mod_coord.x - 0.51) * -0.005,
                  sign(mod_coord.y - 0.51) * -0.005);

  vec2 tex_size = vec2(raw_coord.w - raw_coord.y, raw_coord.z - raw_coord.x);

  vec2 offset = raw_coord.xy;

  pass_texcoord = offset + (tex_size * mod_coord);
  pass_color    = color[gl_InstanceID];

  gl_Position = projection * view * models[gl_InstanceID] * vec4(in_pos, 1.0f);
}

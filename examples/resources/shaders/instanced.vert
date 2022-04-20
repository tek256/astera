#version 330
#define MAX_BATCH_SIZE 32

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texc;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 mats[MAX_BATCH_SIZE];
uniform int  flip_x[MAX_BATCH_SIZE];
uniform int  flip_y[MAX_BATCH_SIZE];
uniform vec4 coords[MAX_BATCH_SIZE];
uniform vec4 colors[MAX_BATCH_SIZE];

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

  vec2 tex_size = vec2(raw_coord.w - raw_coord.y, raw_coord.z - raw_coord.x);

  vec2 offset = raw_coord.xy;

  // sprite ordering based on how far down on the screen it is
  vec4 mod_pos = vec4(in_pos, 1.0f);
  mod_pos.z += (180.f - mod_pos.y) * 0.01f;

  pass_texcoord = offset + (tex_size *  mod_coord);
  pass_color = colors[gl_InstanceID];

  gl_Position = projection * view * mats[gl_InstanceID] * mod_pos;
}

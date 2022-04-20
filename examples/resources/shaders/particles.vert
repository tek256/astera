#version 330
#define MAX_BATCH_SIZE 32

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texc;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 mats[MAX_BATCH_SIZE];
uniform vec4 coords[MAX_BATCH_SIZE];
uniform vec4 colors[MAX_BATCH_SIZE];

// 0 = colored only, 1 = textured
uniform int use_tex = 0;

out vec2 pass_texcoord;
out vec4 pass_color;
flat out int pass_usetex;

void main() {
  if (use_tex == 1) {
    vec2 mod_coord = in_texc;
    vec4 raw_coord = coords[gl_InstanceID];

    vec2 tex_size = vec2(raw_coord.w - raw_coord.y, raw_coord.z - raw_coord.x);
    vec2 offset = raw_coord.xy;
    pass_texcoord = offset + (tex_size * mod_coord);
  }

  pass_usetex = use_tex;

  pass_color = colors[gl_InstanceID];

  vec4 mod_pos = vec4(in_pos, 1.0f);
  mod_pos.z += (180.f - mod_pos.y) * 0.01f;

  gl_Position =
      projection * view * mats[gl_InstanceID] * mod_pos;
}

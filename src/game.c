#include <misc/linmath.h>

#include "game.h"

#include "audio.h"
#include "conf.h"
#include "debug.h"
#include "input.h"
#include "render.h"
#include "sys.h"

#include "asset.h"

typedef struct {
} g_entity;

g_entity ent;

r_sprite sprite, sprite2;
u32 *frames;

static int tex_ids, models, flip_x, flip_y;

int g_init(void) {
  r_init_anim_map(32);
  r_init_shader_map(2);
  r_init_batches(4);

  asset_t *window_icon = asset_get("sys", "res/tex/icon.png");
  r_window_set_icon(window_icon);
  asset_free(window_icon);
  window_icon = 0;

  // Initialize default shader
  asset_t *default_vert = asset_get("sys", "res/shd/main.vert");
  asset_t *default_frag = asset_get("sys", "res/shd/main.frag");
  r_shader default_shader = r_shader_create(default_vert, default_frag);
  asset_free(default_vert);
  asset_free(default_frag);

  // Initialize default texture sheet
  asset_t *default_sheet_file = asset_get("sys", "res/tex/test_sheet.png");
  r_sheet default_sheet = r_sheet_create(default_sheet_file, 16, 16);
  asset_free(default_sheet_file);

  // r_anim r_anim_create(r_sheet sheet, u32 *frames, int frame_count,
  //                  int frame_rate);

  // TODO make this not terrible. (It's gotten better)
  // Let me look at the picture and see if I'm actually just stupid
  // Pointers are silly
  frames = malloc(sizeof(u32) * 4);
  frames[0] = 0;
  frames[1] = 1;
  frames[2] = 2;
  frames[3] = 3;

  // frames = (u32 *){0, 1, 2, 3, 4};

  // I think I'm gonna call stream here tho, it's been a solid 8 hours.
  // I'm gonna send you guys over to my friend Tsoding, I'll be live again here
  // soon. Join the discord if you wanna chat with me off stream!
  //
  // Thanks for hanging out, I'll see yall later!
  r_anim anim = r_anim_create(default_sheet, frames, 4, 21);
  anim.loop = 1;
  r_anim_cache(anim, "default");

  // Initialize Uniform Arrays
  // void r_shader_setup_array(r_shader shader, r_sheet sheet, const char *name,
  //                          int capacity, int type, int uid);
  flip_x = r_shader_setup_array(default_shader, "flip_x", 512, r_int);
  flip_y = r_shader_setup_array(default_shader, "flip_y", 512, r_int);
  models = r_shader_setup_array(default_shader, "models", 512, r_mat);
  tex_ids = r_shader_setup_array(default_shader, "tex_ids", 512, r_int);

  vec2 size, position, position2;
  size[0] = 100.f;
  size[1] = 100.f;

  position2[0] = 100.f;

  // Maybe you are in a game.
  r_subtex sub_tex = (r_subtex){default_sheet, 1};
  r_subtex sub_tex2 = (r_subtex){default_sheet, 6};

  sprite = r_sprite_create(default_shader, position, size);
  sprite2 = r_sprite_create(default_shader, position2, size);
  r_sprite_set_tex(&sprite, sub_tex);
  r_sprite_set_anim(&sprite2, anim);

  return 1;
}

void g_exit(void) {}

void g_input(long delta) {
  if (i_key_clicked(GLFW_KEY_ESCAPE))
    r_window_request_close();

  int dir_x = 0;
  int dir_y = 0;
  if (i_key_down('A'))
    dir_x = -1;
  else if (i_key_down('D'))
    dir_x = 1;

  if (i_key_down('W'))
    dir_y = 1;
  else if (i_key_down('S'))
    dir_y = -1;

  // I mean, it does something I guess.
  // Let's make it loop
  if (i_key_clicked('P'))
    r_sprite_play(&sprite2);
  else if (i_key_clicked('O'))
    r_sprite_pause(&sprite2);

  r_cam_move(dir_x * 1.f * delta, dir_y * 1.f * delta);
}

void g_update(long delta) { //
  r_sprite_update(&sprite, delta);
  r_sprite_update(&sprite2, delta);
}

void g_render(long delta) {
  r_sprite_draw(sprite);
  int tex_id = r_sprite_get_tex_id(sprite);

  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);

  tex_id = r_sprite_get_tex_id(sprite2);

  r_sprite_draw(sprite2);
  r_shader_sprite_uniform(sprite2, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite2, models, &sprite2.model);
  r_shader_sprite_uniform(sprite2, flip_x, &sprite2.flip_x);
  r_shader_sprite_uniform(sprite2, flip_y, &sprite2.flip_y);
}

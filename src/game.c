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

r_sprite sprite;

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

  // Initialize Uniform Arrays
  // void r_shader_setup_array(r_shader shader, r_sheet sheet, const char *name,
  //                          int capacity, int type, int uid);
  flip_x = r_shader_setup_array(default_shader, "flip_x", 512, r_int);
  flip_y = r_shader_setup_array(default_shader, "flip_y", 512, r_int);
  models = r_shader_setup_array(default_shader, "models", 512, r_mat);
  tex_ids = r_shader_setup_array(default_shader, "tex_ids", 512, r_int);

  vec2 size, position;
  size[0] = 1000.f;
  size[1] = 1000.f;

  r_subtex sub_tex = (r_subtex){default_sheet, 0};

  sprite = r_sprite_create(default_shader, position, size);
  r_sprite_set_tex(&sprite, sub_tex);

  sprite.visible = 1;

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

  r_cam_move(dir_x * 1.f * delta, dir_y * 1.f * delta);
}

void g_update(long delta) { //
  r_sprite_update(&sprite, delta);
}

void g_render(long delta) {
  r_sprite_draw(sprite);
  int tex_id = r_sprite_get_tex_id(sprite);

  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);
}

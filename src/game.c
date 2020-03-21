#include <assert.h>
#include <misc/linmath.h>

#include "game.h"

#include "audio.h"
#include "conf.h"
#include "debug.h"
#include "input.h"
#include "render.h"
#include "sys.h"

#include "asset.h"

#include "level.h"
#include "ui.h"

#define test_sprite_count 4096 * 16
#define ROW_WIDTH         128
#define BATCH_SIZE        512

typedef enum { TEST0 = 0, TEST1, TEST2, TEST3 } obj_types;

typedef struct {
  l_obj    obj;
  r_sprite sprite;

  vec2 pos;
} g_obj;

r_sprite  sprite, sprite2;
r_sprite  circle_sprite, aabb_sprite;
uint32_t* frames;

static int tex_ids, models, flip_x, flip_y;

vec2 window_size;

r_shader screen_shader;

a_music* test_music;
a_buf    test_buf;
a_req    music_req, sfx_req;

l_aabb   aabb, aabb2;
l_circle circle, circle2;

uint32_t box_uid, button_uid, dropdown_uid;

r_shader    star_shader;
r_particles particles;

float         text_time, text_rate;
int           text_count;
r_shader      default_shader;
r_shader      standard_shader, particle_shader;
r_baked_sheet baked_sheet;

// TODO Finish g_obj impl
g_obj        test_obj;
a_fx_reverb* reverb;
a_fx         reverb_slot;

uint32_t step = 0;

l_quad_tree tree;

int g_init(void) {
  r_init_anim_map(32);
  r_init_shader_map(2);
  r_init_batches(4);

  // Initialize default shader
  asset_t* default_vert = asset_get("sys", "res/shd/main.vert");
  asset_t* default_frag = asset_get("sys", "res/shd/main.frag");
  default_shader        = r_shader_create(default_vert, default_frag);
  asset_free(default_vert);
  asset_free(default_frag);

  asset_t* standard_vert = asset_get("sys", "res/shd/std.vert");
  asset_t* standard_frag = asset_get("sys", "res/shd/std.frag");
  standard_shader        = r_shader_create(standard_vert, standard_frag);
  asset_free(standard_vert);
  asset_free(standard_frag);

  // Create the screen's framebuffer shader
  asset_t* screen_vert = asset_get("sys", "res/shd/screen.vert");
  asset_t* screen_frag = asset_get("sys", "res/shd/screen.frag");
  screen_shader        = r_shader_create(screen_vert, screen_frag);
  asset_free(screen_vert);
  asset_free(screen_frag);

  asset_t* star_vert = asset_get("sys", "res/shd/stars.vert");
  asset_t* star_frag = asset_get("sys", "res/shd/stars.frag");
  star_shader        = r_shader_create(star_vert, star_frag);
  asset_free(star_vert);
  asset_free(star_frag);

  asset_t* part_vert = asset_get("sys", "res/shd/particle.vert");
  asset_t* part_frag = asset_get("sys", "res/shd/particle.frag");
  particle_shader    = r_shader_create(part_vert, part_frag);
  asset_free(part_vert);
  asset_free(part_frag);

  // Create the screen's framebuffer proper
  int screen_width, screen_height;
  r_window_get_size(&screen_width, &screen_height);

  window_size[0] = screen_width;
  window_size[1] = screen_height;

  r_cam_set_size(320, 180);

  float camera_width, camera_height;
  r_cam_get_size(&camera_width, &camera_height);

  // Initialize default texture sheet
  asset_t* default_sheet_file = asset_get("sys", "res/tex/test_sheet.png");
  r_sheet  default_sheet      = r_sheet_create(default_sheet_file, 16, 16);
  asset_free(default_sheet_file);

  asset_t* dungeon_sheet_file = asset_get("sys", "res/tex/Dungeon_Tileset.png");
  r_sheet  dungeon_sheet      = r_sheet_create(dungeon_sheet_file, 16, 16);
  asset_free(dungeon_sheet_file);

  frames = malloc(sizeof(uint32_t) * 4);
  for (int i = 0; i < 4; ++i) {
    frames[i] = i;
  }

  r_anim anim = r_anim_create(default_sheet, frames, 4, 12);
  anim.loop   = 1;
  r_anim_cache(anim, "default");

  flip_x  = r_shader_setup_array(default_shader, "flip_x", BATCH_SIZE, r_int);
  flip_y  = r_shader_setup_array(default_shader, "flip_y", BATCH_SIZE, r_int);
  models  = r_shader_setup_array(default_shader, "models", BATCH_SIZE, r_mat);
  tex_ids = r_shader_setup_array(default_shader, "tex_ids", BATCH_SIZE, r_int);

  vec2 size, position, position2;
  size[0] = 16.f;
  size[1] = 16.f;

  position[0] = 0.0f;
  position[1] = 0.0f;

  position2[0] = 0.f;
  position2[1] = 0.f;

  r_subtex sub_tex  = (r_subtex){default_sheet, 1};
  r_subtex sub_tex2 = (r_subtex){default_sheet, 6};

  sprite  = r_sprite_create(default_shader, position, size);
  sprite2 = r_sprite_create(default_shader, position2, size);

  sprite.layer   = 5;
  sprite.change  = 1;
  sprite2.layer  = 5;
  sprite2.change = 1;

  r_sprite_set_tex(&sprite, sub_tex);
  r_sprite_set_anim(&sprite2, anim);

  r_subtex test_subtex = (r_subtex){dungeon_sheet, 0};

  srand(time(NULL));

  r_baked_quad* quads =
      (r_baked_quad*)malloc(sizeof(r_baked_quad) * test_sprite_count);

  if (!quads) {
    _e("Unable to alloc %i baked_quads for test sprites.\n", test_sprite_count);
    return 0;
  }

  int max_height = test_sprite_count / ROW_WIDTH;

  for (int i = 0; i < test_sprite_count; ++i) {
    int row    = i / ROW_WIDTH;
    int col    = i % ROW_WIDTH;
    int sprite = rand() % 6;

    quads[i].x      = col;
    quads[i].y      = row;
    quads[i].layer  = 0;
    quads[i].sub_id = sprite + 2;
    quads[i].flip_x = rand() % 1;
    quads[i].flip_y = rand() % 1;
  }

  baked_sheet = r_baked_sheet_create(dungeon_sheet, quads, test_sprite_count,
                                     (vec2){0.f, 0.f}, 0);

  // CREATE MUSIC
  music_req       = (a_req){0};
  music_req.stop  = 0;
  music_req.loop  = 0;
  music_req.range = 45.f;
  vec3_clear(music_req.pos);

  // music_req.layer = 0;
  music_req.gain  = 0.8f;
  music_req.range = 10.f;

  /*a_fx_reverb a_fx_reverb_create(float density, float diffusion, float gain,
                               float gaingf, float decay, float refl_gain,
                               float refl_delay);
   *
   */
  // Create a reverb with all the defaults
  // reverb = a_fx_reverb_create(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f,
  //                            -1.f, -1.f, -1.f, -1.f, -1.f);
  reverb  = (a_fx_reverb*)malloc(sizeof(a_fx_reverb));
  *reverb = a_fx_reverb_default();

  _l("REVERB: gain: %f density: %f\n", reverb->gain, reverb->density);

  reverb_slot = a_fx_create(REVERB, reverb);
  a_fx_slot_attach(&reverb_slot);

  a_fx** req_fx      = malloc(sizeof(a_fx*));
  req_fx[0]          = &reverb_slot;
  music_req.fx       = req_fx;
  music_req.fx_count = 1;

  asset_t* music_asset = asset_get("sys", "res/snd/test_song.ogg");
  test_music           = a_music_create(music_asset, NULL, &music_req);
  a_layer_add_music(1, test_music);

  r_particles_init(&particles, 1024);
  particles.colored = 1;
  particles.layer   = 2;

  vec4 particle_color   = {1.f, 1.f, 1.f, 0.5f};
  vec2 particle_size    = {1024.f, 1024.f};
  vec2 particle_pos     = {5.f, 5.f};
  particles.velocity[1] = 4.f;

  particles.spawn_rate = 100.f;

  particles.particle_size[0] = 2.f;
  particles.particle_size[1] = 2.f;

  particles.spawn_type    = SPAWN_BOX;
  particles.dir_type      = DIR_CIRCLE;
  particles.particle_life = 3.f;
  particles.system_life   = -1.f;
  vec2_dup(particles.position, particle_pos);
  vec2_dup(particles.size, particle_size);
  vec4_dup(particles.color, particle_color);

  float listener_ori[6] = {0.f, 1.f, 0.f, 0.f, 1.f, 0.f};
  a_set_orif(listener_ori);

  vec2 aa_pos  = {0.f, 0.f};
  vec2 ab_pos  = {100.f, 0.f};
  vec2 aa_size = {8.f, 8.f};

  vec2  ci_pos  = {0.f, 0.f};
  vec2  ci2_pos = {100.f, 35.f};
  float ci_rad  = 16.f;
  vec2  ci_size = {ci_rad * 2, ci_rad * 2};

  aabb    = l_aabb_create(aa_pos, aa_size);
  aabb2   = l_aabb_create(ab_pos, aa_size);
  circle  = l_circle_create(ci_pos, ci_rad);
  circle2 = l_circle_create(ci2_pos, ci_rad);

  circle_sprite = r_sprite_create(default_shader, ci2_pos, ci_size);
  aabb_sprite   = r_sprite_create(default_shader, ab_pos, aa_size);

  circle_sprite.layer  = 5;
  circle_sprite.change = 1;

  aabb_sprite.change = 1;
  aabb_sprite.layer  = 5;

  r_subtex circ_tex = (r_subtex){dungeon_sheet, 86};
  r_subtex aabb_tex = (r_subtex){dungeon_sheet, 88};

  r_sprite_set_tex(&circle_sprite, circ_tex);
  r_sprite_set_tex(&aabb_sprite, aabb_tex);

  vec2 tree_halfsize = {512.f, 512.f};
  vec2 tree_center   = {tree_halfsize[0], tree_halfsize[1]};

  l_aabb tree_range;
  vec2_dup(tree_center, tree_range.center);
  vec2_dup(tree_halfsize, tree_range.halfsize);
  tree = l_tree_create(tree_range, 128);

  // l_obj l_obj_create(l_col col, uint32_t uid, uint32_t type, uint32_t flags)
  // {

  /*  sprite  = r_sprite_create(default_shader, position, size);
  sprite2 = r_sprite_create(default_shader, position2, size);

  sprite.layer   = 5;
  sprite.change  = 1;
  sprite2.layer  = 5;
  sprite2.change = 1;

  r_sprite_set_tex(&sprite, sub_tex);
  r_sprite_set_anim(&sprite2, anim);

   */

  test_obj.obj = l_obj_create(l_col_create(&aabb, L_AABB, 0), 0, 0, 0);
  l_tree_insert(&tree, &test_obj.obj);

  vec2 test_obj_pos = {0.f, 0.f};
  vec2_dup(test_obj.pos, test_obj_pos);

  return 1;
}

static void g_update_obj(g_obj* obj, float delta) {
  // copy the collider's position to the obj representation
  l_obj_get_position(&obj->obj, obj->pos);
}

static int tex_id;

static void g_render_obj(g_obj* obj, float delta) {
  vec2_dup(obj->sprite.position, obj->pos);
  obj->sprite.change = 1;
  r_sprite_update(&obj->sprite, delta);
  r_sprite_draw(obj->sprite);

  tex_id = r_sprite_get_tex_id(obj->sprite);

  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &obj->sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &obj->sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &obj->sprite.flip_y);
}

void g_exit(void) {
  a_fx_destroy(reverb_slot);
}

static int left_over = 0, up_over = 0;
static int left_delta = 0, right_delta = 0;
static int up_delta = 0, down_delta = 0;

static int last_dirx, last_diry;

void g_input(time_s delta) {
  if (i_key_clicked('T')) {
    _l("test: %f %f\n", test_obj.pos[0], test_obj.pos[1]);
  }

  if (i_key_clicked(GLFW_KEY_ESCAPE))
    r_window_request_close();

  if (i_key_clicked('P')) {
    a_music_play(test_music);
  } else if (i_key_clicked('O')) {
    a_music_pause(test_music);
  }

  if (i_key_clicked('G')) {
  }

  vec2 mouse_pos = {i_get_mouse_x(), i_get_mouse_y()};

  int dir_x = 0;
  int dir_y = 0;

  int left  = i_key_down('A');
  int right = i_key_down('D');
  int up    = i_key_down('W');
  int down  = i_key_down('S');

  int left_released  = i_key_released('A');
  int right_released = i_key_released('D');
  int up_released    = i_key_released('W');
  int down_released  = i_key_released('S');

  if (left_over == 1) {
    if (right) {
      dir_x = 1;
      ++right_delta;
    } else if (left) {
      dir_x       = -1;
      right_delta = 0;
      ++left_delta;
    }
  } else if (left_over == -1) {
    if (left) {
      dir_x = -1;
      ++left_delta;
    } else if (right) {
      dir_x      = 1;
      left_delta = 0;
      ++right_delta;
    }
  } else {
    if (left) {
      left_over = 1;
      dir_x     = -1;
      ++left_delta;
      right_delta = 0;
    } else if (right) {
      left_over = -1;
      dir_x     = 1;

      ++right_delta;
      left_delta = 0;
    }
  }

  if (left_delta > 5 && right_released) {
    left_over   = 1;
    left_delta  = 0;
    right_delta = 0;
  } else if (right_delta > 5 && left_released) {
    left_over   = -1;
    left_delta  = 0;
    right_delta = 0;
  }

  if (!left && !right) {
    dir_x     = 0;
    left_over = 0;
  }

  if (up_over == 1) {
    if (down) {
      dir_y = -1;
      ++down_delta;
    } else if (up) {
      dir_y      = 1;
      down_delta = 0;
      ++up_delta;
    }
  } else if (up_over == -1) {
    if (up) {
      dir_y = 1;
      ++up_delta;
    } else if (down) {
      dir_y    = -1;
      up_delta = 0;
      ++down_delta;
    }
  } else {
    if (up) {
      up_over = 1;
      dir_y   = 1;
      ++up_delta;
      down_delta = 0;
    } else if (down) {
      up_over = -1;
      dir_y   = -1;

      ++down_delta;
      up_delta = 0;
    }
  }

  if (up_delta > 5 && down_released) {
    up_over    = 1;
    up_delta   = 0;
    down_delta = 0;
  } else if (down_delta > 5 && up_released) {
    up_over    = -1;
    up_delta   = 0;
    down_delta = 0;
  }

  if (!down && !up) {
    dir_y   = 0;
    up_over = 0;
  }

  if (dir_x != 0 || dir_y != 0)
    r_cam_move(dir_x * 0.2f * delta, dir_y * 0.2f * delta);

  dir_x = 0, dir_y = 0;
  if (i_key_down(GLFW_KEY_LEFT))
    dir_x = -1;
  else if (i_key_down(GLFW_KEY_RIGHT))
    dir_x = 1;

  if (i_key_down(GLFW_KEY_UP))
    dir_y = 1;
  else if (i_key_down(GLFW_KEY_DOWN))
    dir_y = -1;

  vec2 move = {0.f, 0.f};
  if (dir_x != 0 || dir_y != 0) {
    move[0] += dir_x * 0.2f * delta;
    move[1] -= dir_y * 0.2f * delta;
    // l_obj_move(&test_obj.obj, move);
    sprite2.position[0] += dir_x * 0.2f * delta;
    sprite2.position[1] -= dir_y * 0.2f * delta;

    if (dir_x < 0) {
      sprite2.flip_x = 1;
    } else if (dir_x > 0) {
      sprite2.flip_x = 0;
    }

    sprite2.change = 1;
    r_sprite_play(&sprite2);

    if (dir_x != 0)
      last_dirx = dir_x;

    if (dir_y != 0)
      last_diry = dir_y;

    vec3 a_dir = {last_dirx, 1.f, last_diry};
    a_set_ori(a_dir);

    vec2_dup(aabb.center, sprite2.position);
  } else {
    r_sprite_pause(&sprite2);
  }
}

void g_update(time_s delta) {
  // g_update_obj(&test_obj, delta);

  if (l_aabb_vs_cir(NULL, aabb, circle2)) {
    _l("AABB intersecting Circle:% i.\n", step);
  }

  if (l_aabb_vs_aabb(NULL, aabb, aabb2)) {
    _l("AABB intersecting AABB: %i.\n", step);
  }

  vec3 a_pos = {test_obj.pos[0], 0.f, test_obj.pos[1]};
  a_set_pos(a_pos);

  r_sprite_update(&sprite, delta);
  r_sprite_update(&sprite2, delta);
  r_sprite_update(&circle_sprite, delta);
  r_sprite_update(&aabb_sprite, delta);

  r_particles_update(&particles, delta);
  ++step;
}

static void g_draw_sprite(r_sprite sprite) {
  r_sprite_draw(sprite);

  tex_id = r_sprite_get_tex_id(sprite);
  // r_check_error_loc("Sprite1 Uniforms");
  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);
}

void g_render(time_s delta) {
  // g_render_obj(&test_obj, delta);
  r_check_error_loc("Sprite Drawcall");

  // g_draw_sprite(sprite);
  g_draw_sprite(sprite2);
  g_draw_sprite(circle_sprite);
  g_draw_sprite(aabb_sprite);

  r_baked_sheet_draw(standard_shader, &baked_sheet);
  r_particles_draw(&particles, particle_shader);
}

void g_frame_start() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void g_frame_end() {
}

#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <astera/asset.h>
#include <astera/audio.h>
#include <astera/col.h>
#include <astera/debug.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/input.h>
#include <astera/ui.h>

// #define DRAW_HITBOXES

#define USER_PREFS_FILE "user_prefs.ini"

#define MAX_ENEMIES           24
#define MAX_PRESPAWN          6
#define MAX_SPAWN_RATE        1500 // ms between enemy spawns
#define MAX_PARTICLE_EMITTERS 8

#define MAX_HEALTH_ORBS     8
#define MAX_HEALTH_ORB_LIFE 5000 // 5 sec
#define MAX_PLAYER_HEALTH   6
#define DAMAGE_DURATION     750 // ms
#define STAGGER_DURATION    100 // ms
#define KNOCKBACK_AMOUNT    12  // units

#define ATTACK_COOLDOWN 400 // ms

#define JOY_ID       0
#define JOY_DEADZONE 0.1f

// ui stuff

typedef enum {
  MENU_NONE     = 0,
  MENU_MAIN     = 1,
  MENU_SETTINGS = 2,
  MENU_PAUSE    = 3,
  MENU_END      = 4,
} MENU_PAGES;

typedef struct {
  // --- MAIN ---
  ui_text   main_title;
  ui_button play, settings, quit;

  // --- SETTINGS ---
  ui_text   master_label, sfx_label, music_label, settings_title;
  ui_slider master_vol, sfx_vol, music_vol;
  ui_box    s_bg;

  ui_text     res_label;
  ui_dropdown res_dd;

  ui_button back_button;

  // --- PAUSE ---
  ui_text   p_title;
  ui_box    p_bg;
  ui_button p_resume, p_settings, p_quit;

  // --- END ---
  ui_button w_again, w_quit;
  ui_text   w_title;
  ui_box    w_bg;

  // --- IN GAME ---
  ui_img heart_full, heart_half, heart_empty;

  // --- PAGES ---
  ui_tree  main_page, settings_page, pause_page, end_page;
  ui_tree* current_page;
  int      page_number, last_page;

  // --- COLORS ---
  ui_color red, white, black, grey, clear, offwhite, offblack;

  // --- OTHER ---
  ui_font  font;
  asset_t* font_data;
  float    scroll_timer, scroll_duration;
  float    joy_timer;
} menu_t;

// audio data
typedef struct {
  int sfx_layer, music_layer;

  int s_enemy_hit, s_menu_move, s_player_die, s_player_hit, s_menu_select,
      s_enemy_die, s_attack;

  /* 0 - attack
   * 1 - player hit
   * 2 - player die
   * 2 - menu select
   * 3 - menu move
   * 4 - enemy hit
   * 5 - enemy die */
  a_req reqs[8];
  int   slots[8];
  int   req_count;
} a_resources;

typedef enum {
  ENEMY_IDLE = 0,
  ENEMY_WALK,
  ENEMY_HIT,
  ENEMY_SPAWN,
  ENEMY_DIE,
} ENEMY_STATE;

typedef enum {
  ENEMY_EYE    = 0,
  ENEMY_SLIME  = 1,
  ENEMY_GOBLIN = 2,
} ENEMY_TYPES;

typedef enum {
  GAME_START = 0,
  GAME_PLAY  = 1,
  GAME_PAUSE = 2,
  GAME_WIN   = 3,
  GAME_LOSE  = 4,
  GAME_QUIT  = -1,
} GAME_STATE;

typedef struct {
  r_sprite sprite;
  c_circle circle;
  vec2     stagger, exit_point;
  int      health, max_health;
  int      state, state_change;
  int      type, take_damage, can_take_damage;
  float    damage_timer, move_speed, spawn_timer;
  int      alive;
} enemy_t;

typedef struct {
  vec2     center;
  c_aabb   aabb, hitbox;
  int      health, is_idle, take_damage, allow_spawns, kill_count;
  float    damage_timer, damage_duration, attack_timer;
  r_sprite sprite, sword, swoosh;
  int      hori, vert;
  int      left_priority, right_priority, up_priority, down_priority;
} player_t;

typedef struct {
  r_sprite sprite;
  vec2     center;
  int      active;
  float    timer;
} health_orb_t;

typedef struct {
  enemy_t* enemies;
  int      enemy_count, enemy_capacity;

  c_aabb* env;
  int     env_cols;

  c_aabb spawn_cols[4];

  player_t player;

  r_particles emitters[MAX_PARTICLE_EMITTERS];

  health_orb_t health_orbs[MAX_HEALTH_ORBS];

  vec2   spawn_points[4];
  int    to_spawn;
  time_s spawn_timer;
} level_t;

typedef struct {
  asset_t* raw_table;
  float    master_vol, sfx_vol, music_vol;
  int      res_width, res_height;
} user_preferences;

static user_preferences user_prefs    = {0};
static menu_t           menu          = {0};
static a_resources      a_res         = {0};
static level_t          level         = {0};
static int              is_continuous = 0, cont_round = 0;

r_ctx*  render_ctx;
i_ctx*  input_ctx;
ui_ctx* u_ctx;
a_ctx*  audio_ctx;

vec2 window_size;

r_shader shader, baked, fbo_shader, particle_shader;

r_baked_sheet baked_sheet;
r_framebuffer fbo, ui_fbo;
r_sheet       sheet, character_sheet;

static vec2 camera_size;

GLFWvidmode* vidmodes;
uint8_t      vidmode_count;

static int game_state = GAME_START;

s_timer render_timer;
s_timer update_timer;

static void debug_output(void) {
  for (int i = 0; i < level.enemy_capacity; ++i) {
    if (level.enemies[i].health != 0 && !level.enemies[i].alive) {
      printf("!!! %i is dead but has %i hp !!!\n", i, level.enemies[i].health);
    }

    printf("%i: [hp: %i alive: %i]\n", i, level.enemies[i].health,
           level.enemies[i].alive);
  }
}

static int rnd_range(int min, int max) {
  int value = rand() % (max - min);
  return value + min;
}

r_shader load_shader(const char* vs, const char* fs) {
  asset_t* vs_data = asset_get(vs);
  asset_t* fs_data = asset_get(fs);

  r_shader shader = r_shader_create(vs_data->data, fs_data->data);

  asset_free(vs_data);
  asset_free(fs_data);

  return shader;
}

r_sheet load_sheet(const char* sheet_file, int sub_width, int sub_height) {
  asset_t* sheet_data = asset_get(sheet_file);
  r_sheet  sheet      = r_sheet_create_tiled(
      sheet_data->data, sheet_data->data_length, sub_width, sub_height, 0, 0);
  asset_free(sheet_data);
  return sheet;
}

r_anim* load_anim(r_sheet* sheet, const char* name, int* frames, int length,
                  int rate, int loop) {
  r_anim anim = r_anim_create_fixed(sheet, frames, length, rate);
  anim.loop   = loop;
  return r_anim_cache(render_ctx, anim, name);
}

// move in direction / amount of vec2 (non-normalized)
void move_enemy(enemy_t* en, vec2 amount) {
  c_circle_move(&en->circle, amount);
  vec2_dup(en->sprite.position, en->circle.center);
}

void particle_spawn(r_particles* system, r_particle* particle) {
  float x = fmod(rand(), system->size[0]);
  float y = fmod(rand(), system->size[1]);

  particle->position[0] = x;
  particle->position[1] = y;
  particle->layer       = system->particle_layer;

  int dir_x = 0, dir_y = 0;

  dir_x = rnd_range(-2, 2);
  dir_y = rnd_range(-2, 0);

  particle->direction[0] = dir_x;
  particle->direction[1] = dir_y;
}

void particle_animate(r_particles* system, r_particle* particle) {
  float life_span = system->particle_life - particle->life;
  float progress  = life_span / system->particle_life;

  particle->frame = r_particles_frame_at(system, life_span);

  particle->velocity[0] =
      (sin(progress * 3.1459) * particle->direction[0]) * 0.015f;
  particle->velocity[1] =
      (sin(progress * 3.1459) * particle->direction[1]) * 0.015f;
}

void create_emitter(vec2 position, vec2 dir) {
  for (int i = 0; i < MAX_PARTICLE_EMITTERS; ++i) {
    r_particles* emitter = &level.emitters[i];
    if (r_particles_finished(emitter)) {
      r_particles_reset(emitter);
      r_particles_set_position(emitter, position);
      r_particles_start(emitter);
      break;
    }
  }
}

int spawn_enemy(void) {
  if (level.enemy_count == level.enemy_capacity - 1) {
    printf("No open slots for new enemy\n");
    return 0;
  }

  // find slot
  enemy_t* en = 0;
  for (int i = 0; i < level.enemy_capacity; ++i) {
    if (!level.enemies[i].alive) {
      en = &level.enemies[i];
      break;
    }
  }

  if (!en) {
    printf("Unable to find open slot for new enemy");
    return 0;
  }

  // up down left right
  int dir = rand() % 4;

  int within_range = 0;

  if ((within_range =
           vec2_dist(level.player.center, level.spawn_points[dir]) < 64.f)) {
    int tries = 0;
    while (within_range) {
      dir = rand() % 4;
      within_range =
          vec2_dist(level.player.center, level.spawn_points[dir]) < 64.f;

      ++tries;
      if (tries > 16)
        return 0;
    }
  }

  switch (dir) {
    case 0: // up
      en->exit_point[0] = 152.f;
      en->exit_point[1] = 16.f;
      break;
    case 1: // down
      en->exit_point[0] = 152.f;
      en->exit_point[1] = 164.f;
      break;
    case 2: // left
      en->exit_point[0] = 16.f;
      en->exit_point[1] = 82.f;
      break;
    case 3: // right
      en->exit_point[0] = 308.f;
      en->exit_point[1] = 82.f;
      break;
    default:
      break;
  }

  vec2 sprite_size = {16.f, 16.f};
  en->sprite = r_sprite_create(shader, level.spawn_points[dir], sprite_size);
  en->sprite.layer    = 4;
  en->state           = ENEMY_SPAWN;
  en->state_change    = 0;
  en->type            = rand() % 3;
  en->damage_timer    = 0.f;
  en->can_take_damage = 1;
  en->take_damage     = 0;

  r_anim* anim = 0;

  float radius = 6.f;

  switch (en->type) {
    case ENEMY_EYE:
      en->max_health = 2;
      anim           = r_anim_get_name(render_ctx, "eye_idle");
      radius         = 5.f;
      en->move_speed = 2.f;
      break;
    case ENEMY_GOBLIN:
      en->max_health = 4;
      anim           = r_anim_get_name(render_ctx, "goblin_walk");
      en->move_speed = 3.f;
      break;
    case ENEMY_SLIME:
      en->max_health = 3;
      anim           = r_anim_get_name(render_ctx, "slime_walk");
      en->move_speed = 4.f;
      break;
  }

  float rand_anim_advance = (float)(rand() % 1500);

  en->circle = c_circle_create(level.spawn_points[dir], radius);
  en->health = en->max_health;
  r_sprite_set_anim(&en->sprite, anim);
  r_sprite_anim_play(&en->sprite);
  r_sprite_update(&en->sprite, rand_anim_advance);

  // placement check
  for (int i = 0; i < level.enemy_capacity; ++i) {
    enemy_t* other = &level.enemies[i];
    int      tries = 0;
    if (other != en) {
      if (other->alive) {
        c_manifold man = c_circle_vs_circle_man(en->circle, other->circle);
        while (man.distance != 0.f && tries < 4) {
          vec2  adjust     = {0.f, 0.f};
          float rnd_adjust = (float)rnd_range(6, 10);
          // adjust position
          switch (dir) {
            case 0: // up
              adjust[1] = -rnd_adjust;
              break;
            case 1: // down
              adjust[1] = rnd_adjust;
              break;
            case 2: // left
              adjust[0] = -rnd_adjust;
              break;
            case 3: // right
              adjust[0] = rnd_adjust;
              break;
          }
          move_enemy(en, adjust);
          man = c_circle_vs_circle_man(en->circle, other->circle);
          ++tries;
        }
      }
      if (tries == 8) {
        return 0;
      }
    }
  }

  en->alive = 1;
  ++level.enemy_count;
  return 1;
}

void spawn_health_orb(vec2 pos) {
  for (int i = 0; i < MAX_HEALTH_ORBS; ++i) {
    health_orb_t* orb = &level.health_orbs[i];
    if (!orb->active) {
      orb->timer = 0.f;
      vec2_dup(orb->center, pos);
      r_sprite_set_pos(&level.health_orbs[i].sprite, pos);
      orb->active = 1;
      break;
    }
  }
}

void update_player_sprites(void) {
  int  swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);
  vec2 center       = {0.f};

  vec2 hitbox_pos, hitbox_halfsize = {19.5f, 24.f};
  vec2_dup(hitbox_pos, level.player.center);
  hitbox_pos[0] += (level.player.sprite.flip_x) ? -9.75f : 9.75f;
  vec2_scale(hitbox_halfsize, hitbox_halfsize, 0.5f);
  c_aabb_set(&level.player.hitbox, hitbox_pos, hitbox_halfsize);

  c_aabb_get_center(center, level.player.aabb);
  r_sprite_set_pos(&level.player.sprite, center);
  vec2 sword_pos;
  vec2_dup(sword_pos, level.player.sprite.position);
  sword_pos[0] += (level.player.sprite.flip_x) ? -9.f : 9.f;
  sword_pos[1] -= 2.f;

  level.player.sword.flip_x = level.player.sprite.flip_x;
  r_sprite_set_pos(&level.player.sword, sword_pos);
  sword_pos[0] += (level.player.sprite.flip_x) ? -5.f : 5.f;
  sword_pos[1] += 2.f;
  if (!swoosh_state)
    r_sprite_set_pos(&level.player.swoosh, sword_pos);

  if (level.player.sword.flip_x) {
    level.player.sword.layer = 9;
  } else {
    level.player.sword.layer = 5;
  }
}

// set the menu page based on enum
static void menu_set_page(int page) {
  // invalid page type
  if (!(page == MENU_NONE || page == MENU_SETTINGS || page == MENU_PAUSE ||
        page == MENU_MAIN || page == MENU_END)) {
    ASTERA_DBG("Invalid page type.\n");
    return;
  }

  menu.last_page   = menu.page_number;
  menu.page_number = page;

  switch (page) {
    case MENU_NONE: {
      menu.current_page = 0;
    } break;
    case MENU_MAIN: {
      menu.current_page = &menu.main_page;
    } break;
    case MENU_SETTINGS: {
      menu.current_page = &menu.settings_page;
    } break;
    case MENU_PAUSE: {
      menu.current_page = &menu.pause_page;
    } break;
    case MENU_END: {
      menu.current_page = &menu.end_page;
    } break;
    default:
      ASTERA_DBG("Invalid page\n");
      break;
  }
}

void game_end(int won) {
  // set end game
  menu_set_page(MENU_END);
  game_state        = won ? GAME_WIN : GAME_LOSE;
  menu.w_title.text = (game_state == GAME_WIN) ? "YOU WON!" : "YOU DIED!";
}

// setup keybindings / etc
void init_input(void) {
  i_binding_add(input_ctx, "left", KEY_A, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "left", KEY_LEFT, ASTERA_BINDING_KEY);

  i_binding_add(input_ctx, "right", KEY_D, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "right", KEY_RIGHT, ASTERA_BINDING_KEY);

  i_binding_add(input_ctx, "up", KEY_W, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "up", KEY_UP, ASTERA_BINDING_KEY);

  i_binding_add(input_ctx, "down", KEY_S, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "down", KEY_DOWN, ASTERA_BINDING_KEY);

  i_binding_add(input_ctx, "select", KEY_SPACE, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "select", KEY_ENTER, ASTERA_BINDING_KEY);

  i_binding_add(input_ctx, "attack", KEY_RIGHT_CTRL, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "attack", KEY_SPACE, ASTERA_BINDING_KEY);

  i_joy_create(input_ctx, JOY_ID);
}

void init_ui(void) {
  u_ctx = ui_ctx_create(window_size, 1.f, 1, 1, 1);
  ui_get_color(menu.white, "FFF");
  ui_get_color(menu.offwhite, "EEE");
  ui_get_color(menu.red, "de0c0c");
  ui_get_color(menu.grey, "777");
  ui_get_color(menu.black, "0a0a0a");
  ui_get_color(menu.offblack, "444");
  vec4_clear(menu.clear);

  menu.font_data = asset_get("resources/fonts/OpenSans-Regular.ttf");
  menu.font      = ui_font_create(u_ctx, menu.font_data->data,
                                  menu.font_data->data_length, "OpenSans");

  ui_attrib_setc(u_ctx, UI_DROPDOWN_BG, menu.grey);
  ui_attrib_setc(u_ctx, UI_DROPDOWN_BG_HOVER, menu.white);
  ui_attrib_setc(u_ctx, UI_DROPDOWN_COLOR, menu.black);
  ui_attrib_setc(u_ctx, UI_DROPDOWN_COLOR_HOVER, menu.offblack);
  ui_attrib_setc(u_ctx, UI_DROPDOWN_SELECT_COLOR, menu.offblack);
  ui_attrib_setc(u_ctx, UI_DROPDOWN_SELECT_COLOR_HOVER, menu.offblack);
  ui_attrib_setf(u_ctx, UI_DROPDOWN_BORDER_RADIUS, 5.f);
  ui_attrib_seti(u_ctx, UI_DROPDOWN_ALIGN, UI_ALIGN_CENTER);
  ui_attrib_setf(u_ctx, UI_DROPDOWN_FONT_SIZE, 24.f);
  ui_attrib_seti(u_ctx, UI_DROPDOWN_FONT, menu.font);
  ui_attrib_setc(u_ctx, UI_BUTTON_BG, menu.clear);
  ui_attrib_setc(u_ctx, UI_BUTTON_BG_HOVER, menu.clear);
  ui_attrib_seti(u_ctx, UI_BUTTON_TEXT_ALIGN, UI_ALIGN_CENTER);
  ui_attrib_setc(u_ctx, UI_BUTTON_COLOR, menu.grey);
  ui_attrib_setc(u_ctx, UI_BUTTON_COLOR_HOVER, menu.white);
  ui_attrib_seti(u_ctx, UI_DEFAULT_FONT, menu.font);
  ui_attrib_seti(u_ctx, UI_TEXT_FONT, menu.font);
  ui_attrib_setc(u_ctx, UI_TEXT_COLOR, menu.white);
  ui_attrib_seti(u_ctx, UI_TEXT_ALIGN, UI_ALIGN_CENTER);

  ui_attrib_setc(u_ctx, UI_SLIDER_BG, menu.clear);
  ui_attrib_setc(u_ctx, UI_SLIDER_ACTIVE_BG, menu.clear);
  ui_attrib_setc(u_ctx, UI_SLIDER_FG, menu.grey);
  ui_attrib_setc(u_ctx, UI_SLIDER_ACTIVE_FG, menu.white);
  ui_attrib_setc(u_ctx, UI_SLIDER_ACTIVE_FG, menu.white);
  ui_attrib_setc(u_ctx, UI_SLIDER_BORDER_COLOR, menu.grey);
  ui_attrib_setc(u_ctx, UI_SLIDER_ACTIVE_BORDER_COLOR, menu.white);
  ui_attrib_setc(u_ctx, UI_SLIDER_BUTTON_COLOR, menu.clear);
  ui_attrib_setc(u_ctx, UI_SLIDER_BUTTON_ACTIVE_COLOR, menu.clear);
  ui_attrib_setc(u_ctx, UI_SLIDER_BUTTON_BORDER_COLOR, menu.clear);
  ui_attrib_setc(u_ctx, UI_SLIDER_BUTTON_ACTIVE_BORDER_COLOR, menu.clear);
  ui_attrib_setf(u_ctx, UI_SLIDER_FILL_PADDING, 6.f);
  ui_attrib_setf(u_ctx, UI_SLIDER_BORDER_SIZE, 2.f);
  ui_attrib_setf(u_ctx, UI_SLIDER_BORDER_RADIUS, 5.f);

  vec2 temp = {0.5f, 0.2f}, temp2 = {0.25f, 0.1f};
  vec2 zero = {0.f, 0.f};

  // for main, pause, and settings menu usage
  menu.p_bg         = ui_box_create(u_ctx, zero, zero);
  menu.p_bg.size[0] = 1.f;
  menu.p_bg.size[1] = 1.f;
  ui_get_color(menu.p_bg.bg, "000");
  menu.p_bg.bg[3] = 0.4f;

  // -- MAIN MENU --
  menu.main_title =
      ui_text_create(u_ctx, temp, "FIGHTER", 72.f, menu.font, UI_ALIGN_CENTER);

  ui_element tmp_ele;

  temp[0] = 0.5f;
  temp[1] += 0.25f; // 0.5f, 0.45f;
  menu.play =
      ui_button_create(u_ctx, temp, temp2, "PLAY", UI_ALIGN_CENTER, 48.f);
  tmp_ele = ui_element_get(&menu.play, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  temp[1] += 0.15f; // 0.5f, 0.6f;
  menu.settings =
      ui_button_create(u_ctx, temp, temp2, "SETTINGS", UI_ALIGN_CENTER, 48.f);
  tmp_ele = ui_element_get(&menu.settings, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  temp[1] += 0.15f; // 0.5f, 0.75f
  menu.quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 48.f);
  tmp_ele = ui_element_get(&menu.quit, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  menu.main_page      = ui_tree_create(8);
  menu.main_page.loop = 1;
  ui_tree_add(u_ctx, &menu.main_page, &menu.main_title, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.main_page, &menu.play, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.main_page, &menu.settings, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.main_page, &menu.quit, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.main_page, &menu.p_bg, UI_BOX, 0, 0, -1);

  ui_tree_reset(&menu.main_page);

  // WIN SCREEN

  menu.w_bg         = ui_box_create(u_ctx, zero, zero);
  menu.w_bg.size[0] = 1.f;
  menu.w_bg.size[1] = 1.f;
  ui_get_color(menu.w_bg.bg, "000");
  menu.w_bg.bg[3] = 0.2f;

  vec2_clear(temp2);
  temp2[0] = 0.25f;
  temp2[1] = 0.1f;
  temp[0]  = 0.5f;
  temp[1]  = 0.25f;
  menu.w_title =
      ui_text_create(u_ctx, temp, "YOU WON!", 48.f, menu.font, UI_ALIGN_CENTER);

  temp[0] = 0.375f;
  temp[1] += 0.2f;
  menu.w_again =
      ui_button_create(u_ctx, temp, temp2, "KEEP GOING", UI_ALIGN_CENTER, 48.f);

  temp[1] += 0.15f;
  menu.w_quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 48.f);

  menu.end_page      = ui_tree_create(5);
  menu.end_page.loop = 1;
  ui_tree_add(u_ctx, &menu.end_page, &menu.w_title, UI_TEXT, 0, 0, 1);
  ui_tree_add(u_ctx, &menu.end_page, &menu.w_bg, UI_BOX, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.end_page, &menu.w_again, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.end_page, &menu.w_quit, UI_BUTTON, 1, 1, 1);
  ui_tree_reset(&menu.end_page);

  // SETTINGS MENU
  vec2_clear(temp2);

  temp[0] = 0.5f;
  temp[1] = 0.125f;
  menu.settings_title =
      ui_text_create(u_ctx, temp, "SETTINGS", 48.f, menu.font, UI_ALIGN_CENTER);

  temp[1] += 0.15f;

  temp2[0] = 0.45f;
  temp2[1] = 0.05f;

  vec2 temp3;
  temp3[0] = 0.15f;
  temp3[1] = 0.15f;

  // master
  menu.master_label =
      ui_text_create(u_ctx, temp, "MASTER", 16.f, menu.font, UI_ALIGN_LEFT);
  temp[1] += 0.05f;
  float master_percent = a_listener_get_gain(audio_ctx);
  menu.master_vol      = ui_slider_create(u_ctx, temp, temp2, temp3, 1,
                                          master_percent, 0.f, 1.f, 20);

  tmp_ele = ui_element_get(&menu.master_vol, UI_SLIDER);
  ui_element_center_to(tmp_ele, temp);

  float left_pos =
      menu.master_vol.position[0]; // - (menu.master_vol.size[0] * 0.5f);
  menu.master_label.position[0] = left_pos;

  // music
  temp[0] = left_pos;
  temp[1] += temp2[1] + 0.025f;

  menu.music_label =
      ui_text_create(u_ctx, temp, "MUSIC", 16.f, menu.font, UI_ALIGN_LEFT);

  temp[1] += 0.05;

  float music_percent = a_layer_get_gain(audio_ctx, a_res.music_layer);
  menu.music_vol = ui_slider_create(u_ctx, temp, temp2, temp3, 1, music_percent,
                                    0.f, 1.f, 20);

  temp[0] = 0.5f;
  tmp_ele = ui_element_get(&menu.music_vol, UI_SLIDER);
  ui_element_center_to(tmp_ele, temp);

  // sfx
  temp[0] = left_pos;
  temp[1] += temp2[1] + 0.025f;

  menu.sfx_label =
      ui_text_create(u_ctx, temp, "SFX", 16.f, menu.font, UI_ALIGN_LEFT);

  temp[0] = 0.5f;
  temp[1] += 0.05f;
  float sfx_percent = a_layer_get_gain(audio_ctx, a_res.sfx_layer);
  menu.sfx_vol =
      ui_slider_create(u_ctx, temp, temp2, temp3, 1, sfx_percent, 0.f, 1.f, 20);

  tmp_ele = ui_element_get(&menu.sfx_vol, UI_SLIDER);
  ui_element_center_to(tmp_ele, temp);

  // resolution
  temp[0] = left_pos;
  temp[1] += temp2[1] + 0.025f;
  menu.res_label =
      ui_text_create(u_ctx, temp, "RESOLUTION", 16.f, menu.font, UI_ALIGN_LEFT);

  temp[0] = 0.5f;
  temp[1] += 0.05f;

  vidmodes = r_get_vidmodes_by_usize(render_ctx, &vidmode_count);

  char     option_buffer[16]    = {0};
  char     options_buffer[1024] = {0};
  uint16_t option_index         = 0;
  char**   option_list          = (char**)calloc(vidmode_count, sizeof(char*));

  int32_t distance = 60000, closest = -1;

  for (uint8_t i = 0; i < vidmode_count; ++i) {
    uint8_t str_len = r_get_vidmode_str_simple(
        &options_buffer[option_index], 1024 - option_index, vidmodes[i]);
    option_list[i] = &options_buffer[option_index];

    option_index += str_len + 1;
    const GLFWvidmode* cursor = &vidmodes[i];

    int cur_dist = (_max(window_size[0], cursor->width) -
                    _min(window_size[0], cursor->width)) +
                   (_max(window_size[1], cursor->height) -
                    _min(window_size[1], cursor->height)) -
                   cursor->refreshRate;

    if (cur_dist < distance) {
      closest  = i;
      distance = cur_dist;
    }
  }

  // dropdown scrolling
  menu.scroll_timer    = 0.f;
  menu.scroll_duration = 1000.f;

  // Joystick repeating
  menu.joy_timer = 0.0f;

  menu.res_dd =
      ui_dropdown_create(u_ctx, temp, temp2, option_list, vidmode_count);

  // array pointer
  menu.res_dd.data              = vidmodes;
  menu.res_dd.border_size       = 2.f;
  menu.res_dd.option_display    = 4;
  menu.res_dd.bottom_scroll_pad = 1;
  menu.res_dd.top_scroll_pad    = 1;
  menu.res_dd.font_size         = 16.f;
  menu.res_dd.align             = UI_ALIGN_CENTER;
  menu.res_dd.selected          = (closest > -1) ? closest : 0;

  // option_list is copied into the structure for safety
  free(option_list);

  temp[0] = 0.5f;
  tmp_ele = ui_element_get(&menu.res_dd, UI_DROPDOWN);
  ui_element_center_to(tmp_ele, temp);

  // back button
  temp[0] = left_pos;
  temp[1] += temp2[1] + 0.025f;

  temp2[0] = 0.075f;
  temp2[1] = 0.1f;
  // temp[0] += (temp2[0] * 0.5f);

  menu.back_button = ui_button_create(u_ctx, temp, temp2, "BACK",
                                      UI_ALIGN_LEFT | UI_ALIGN_MIDDLE_Y, 24.f);
  ui_color_dup(menu.back_button.hover_bg, menu.clear);

  menu.settings_page      = ui_tree_create(16);
  menu.settings_page.loop = 1;
  ui_tree_add(u_ctx, &menu.settings_page, &menu.settings_title, UI_TEXT, 0, 0,
              0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.master_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.music_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.sfx_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.master_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.music_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.sfx_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.res_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.res_dd, UI_DROPDOWN, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.back_button, UI_BUTTON, 1, 1,
              0);
  ui_tree_add(u_ctx, &menu.settings_page, &menu.p_bg, UI_BOX, 0, 0, -1);
  ui_tree_reset(&menu.settings_page);

  menu.settings_page.loop = 0;

  // --- PAUSE MENU ---

  vec2_clear(temp2);
  temp2[0] = 0.25f;
  temp2[1] = 0.1f;

  temp[0] = 0.5f;
  temp[1] = 0.25f;
  menu.p_title =
      ui_text_create(u_ctx, temp, "PAUSED", 72.f, menu.font, UI_ALIGN_CENTER);

  temp[0] = 0.375f;
  temp[1] += 0.15f;
  menu.p_resume =
      ui_button_create(u_ctx, temp, temp2, "RESUME", UI_ALIGN_CENTER, 48.f);

  temp[1] += 0.15f;
  menu.p_settings =
      ui_button_create(u_ctx, temp, temp2, "SETTINGS", UI_ALIGN_CENTER, 48.f);

  temp[1] += 0.15f;
  menu.p_quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 48.f);

  menu.pause_page      = ui_tree_create(8);
  menu.pause_page.loop = 1;
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_title, UI_TEXT, 0, 0, 1);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_resume, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_settings, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_quit, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_bg, UI_BOX, 0, 0, 0);
  ui_tree_reset(&menu.pause_page);

  // -- IN GAME --

  vec2 img_pos = {0.f, 0.f}, img_size = {0.025f, 0.015};
  img_size[1]    = ui_square_width(u_ctx, img_size[0]);
  asset_t* asset = asset_get("resources/textures/heart_full.png");

  menu.heart_full = ui_img_create(u_ctx, asset->data, asset->data_length,
                                  IMG_NEAREST, img_pos, img_size);
  asset_free(asset);
  asset           = asset_get("resources/textures/heart_half.png");
  menu.heart_half = ui_img_create(u_ctx, asset->data, asset->data_length,
                                  IMG_NEAREST, img_pos, img_size);

  asset_free(asset);
  asset            = asset_get("resources/textures/heart_empty.png");
  menu.heart_empty = ui_img_create(u_ctx, asset->data, asset->data_length,
                                   IMG_NEAREST, img_pos, img_size);
  asset_free(asset);

  menu_set_page(MENU_MAIN);
}

int load_sfx(const char* path, const char* name) {
  asset_t* data = asset_get(path);

  if (!data) {
    return -1;
  }

  char*    split  = strchr(path, '.');
  int      is_ogg = split && !strcmp(split, ".ogg");
  uint16_t id =
      a_buf_create(audio_ctx, data->data, data->data_length, name, is_ogg);
  asset_free(data);
  return id;
}

void init_audio() {
  a_res.sfx_layer   = a_layer_create(audio_ctx, "sfx", 16, 0);
  a_res.music_layer = a_layer_create(audio_ctx, "music", 0, 2);

  a_layer_set_gain(audio_ctx, a_res.sfx_layer, user_prefs.sfx_vol);
  a_layer_set_gain(audio_ctx, a_res.music_layer, user_prefs.music_vol);

  a_res.s_enemy_hit  = load_sfx("resources/audio/enemy_hit.wav", "enemy_hit");
  a_res.s_attack     = load_sfx("resources/audio/attack.wav", "attack");
  a_res.s_menu_move  = load_sfx("resources/audio/menu_move.wav", "menu_move");
  a_res.s_player_die = load_sfx("resources/audio/player_die.wav", "player_die");
  a_res.s_player_hit = load_sfx("resources/audio/player_hit.wav", "player_hit");
  a_res.s_menu_select = load_sfx("resources/audio/select.wav", "select");
  a_res.s_enemy_die   = load_sfx("resources/audio/enemy_die.wav", "enemy_die");
}

void play_sfx(int sfx_id) {
  if (!a_can_play(audio_ctx)) {
    return;
  }

  int   slot_index = 0;
  float gain       = 1.f;

  if (sfx_id == a_res.s_attack) {
    slot_index = 0;
    gain       = 0.8f;
  } else if (sfx_id == a_res.s_player_hit) {
    slot_index = 1;
  } else if (sfx_id == a_res.s_player_die) {
    slot_index = 2;
  } else if (sfx_id == a_res.s_menu_select) {
    slot_index = 3;
  } else if (sfx_id == a_res.s_menu_move) {
    slot_index = 4;
  } else if (sfx_id == a_res.s_enemy_hit) {
    slot_index = 5;
  } else if (sfx_id == a_res.s_enemy_die) {
    slot_index = 6;
  } else {
    return;
  }

  a_req* req  = &a_res.reqs[slot_index];
  int*   slot = &a_res.slots[slot_index];

  if (*slot) {
    a_sfx_stop(audio_ctx, *slot);
  }

  vec3 a_pos = {0.f, 0.f, 0.f};

  *req  = a_req_create(a_pos, gain, 0.f, 0, 0, 0, 0, 0);
  *slot = a_sfx_play(audio_ctx, a_res.sfx_layer, sfx_id, req);
}

void init_collision(void) {
  vec2 level_min = {0.f, 0.f};
  vec2 level_max = {320.f, 180.f};
  vec2 wall_size = {8.f, 8.f};
  vec2 gap       = {32.f, 32.f};

  vec2 gapped_half_max = {(level_max[0] * 0.5f) - gap[0],
                          (level_max[1] * 0.5f) - gap[1]};

  // 2x side, gaps in between
  level.env_cols = 8;
  level.env      = (c_aabb*)calloc(sizeof(c_aabb), level.env_cols);

  for (int i = 0; i < 4; ++i) {
    int    vert = i < 2;
    c_aabb top, bottom;
    vec2   i_size, i_pos;

    if (vert) {
      i_size[0] = gapped_half_max[0] * 0.5f;
      i_size[1] = wall_size[1];
      i_pos[0]  = (gapped_half_max[0] * 0.5f) + wall_size[0];

      if (i == 0) { // top (dir)
        i_pos[1] = 0.f;
      } else { // bottom (dir)
        i_pos[1] = level_max[1];
      }

      // top/bottom left
      top = c_aabb_create(i_pos, i_size);
      // top/bottom right
      i_pos[0] = level_max[0] - (gapped_half_max[0] * 0.5f) - (wall_size[0]);
      bottom   = c_aabb_create(i_pos, i_size);
    } else {
      i_size[0] = wall_size[0];
      i_size[1] = gapped_half_max[1] * 0.5f;
      i_pos[1]  = (gapped_half_max[1] * 0.5f) + wall_size[1] - 8.f;

      if (i == 2) { // left (dir)
        i_pos[0] = 0.f;
      } else { // right (dir)
        i_pos[0] = level_max[0];
      }

      // left/right top
      top = c_aabb_create(i_pos, i_size);
      // left/right bottom
      i_pos[1] = level_max[1] - (gapped_half_max[1] * 0.5f) - (wall_size[1]);
      i_size[1] += 4.f;
      bottom = c_aabb_create(i_pos, i_size);
    }

    level.env[(i * 2)]     = top;
    level.env[(i * 2) + 1] = bottom;
  }

  vec2 i_size = {gap[0] - wall_size[0], wall_size[1]},
       i_pos  = {level_max[0] * 0.5f, -8.f};
  // top
  level.spawn_cols[0] = c_aabb_create(i_pos, i_size);

  // bottom
  i_pos[1]            = level_max[1] + 8.f;
  level.spawn_cols[1] = c_aabb_create(i_pos, i_size);
  // left
  i_size[0]           = i_size[1];
  i_size[1]           = gap[1] - wall_size[1];
  i_pos[0]            = -8.f;
  i_pos[1]            = (level_max[1] * 0.5f) - (wall_size[1] * 0.5f);
  level.spawn_cols[2] = c_aabb_create(i_pos, i_size);

  // right
  i_pos[0]            = level_max[0] + 8.f;
  level.spawn_cols[3] = c_aabb_create(i_pos, i_size);
}

void init_render(void) {
  // get the vector size of the window
  r_window_get_vsize(render_ctx, window_size);

  // set the window icon
  asset_t* icon = asset_get("resources/textures/icon.png");
  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  asset_free(icon);

  // load in shaders we'll need
  shader = load_shader("resources/shaders/instanced.vert",
                       "resources/shaders/instanced.frag");

  baked = load_shader("resources/shaders/simple.vert",
                      "resources/shaders/simple.frag");

  fbo_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  particle_shader = load_shader("resources/shaders/particles.vert",
                                "resources/shaders/particles.frag");

  // create the two framebuffers we'll use
  fbo    = r_framebuffer_create((int)window_size[0], (int)window_size[1],
                                fbo_shader, 0);
  ui_fbo = r_framebuffer_create((int)window_size[0], (int)window_size[1],
                                fbo_shader, 0);

  // load in the texture sheets
  sheet           = load_sheet("resources/textures/tilemap.png", 16, 16);
  character_sheet = load_sheet("resources/textures/spritesheet.png", 16, 16);

  // calculate sizes of the camera's size
  int sheet_width  = (320 / 16) + 1;
  int sheet_height = (180 / 16) + 1;
  int midpoint_x   = sheet_width / 2;
  int midpoint_y   = sheet_height / 2;

  int torch_count    = (sheet_height * 2) + (sheet_width);
  int torches_placed = 0;

  // create baked sheet
  r_baked_quad* quads = (r_baked_quad*)malloc(
      sizeof(r_baked_quad) * ((sheet_width * sheet_height) + torch_count));
  for (int i = 0; i < (sheet_width * sheet_height); ++i) {
    int texture = 23;
    int flip_x  = 0;
    int flip_y  = 0;
    int x       = i % sheet_width;
    int y       = i / sheet_width;

    if (x == 0 && y == 0) { // top left
      texture = 0;
    } else if (x >= sheet_width - 1 && y == 0) { // top right
      texture = 0;
      flip_x  = 1;
    } else if (x == 0 && y >= sheet_height - 1) { // bottom left
      texture = 40;
    } else if (x >= sheet_width - 1 && y >= sheet_height - 1) { // top right
      texture = 40;
      flip_x  = 1;
    } else if (x == 0) { // left
      if (y == midpoint_y - 2) {
        texture = 13; // top edge
      } else if (y == midpoint_y) {
        texture = 32;
      } else if (y == midpoint_y - 1) {
        texture = 23;
      } else {
        texture = 10;
      }
    } else if (x >= sheet_width - 1) { // right
      if (y == midpoint_y - 2) {
        texture = 13; // top edge
      } else if (y == midpoint_y) {
        texture = 32;
      } else if (y == midpoint_y - 1) {
        texture = 23;
      } else {
        texture = 10;
        flip_x  = 1;
      }
    } else if (y == 0) { // top
      if (x == midpoint_x - 1) {
        // left edge
        texture = 11;
      } else if (x == midpoint_x) {
        texture = 13;
      } else if (x == midpoint_x + 1) {
        // right edge
        texture = 14;
      } else {
        texture = 1;
      }
    } else if (y >= sheet_height - 1) { // bottom
      if (x == midpoint_x - 1) {
        // left edge
        texture = 31;
      } else if (x == midpoint_x) {
        texture = 33;
      } else if (x == midpoint_x + 1) {
        // right edge
        texture = 34;
      } else {
        texture = 41;
      }

    } else if (y == sheet_height - 2 && x == 1) { // bottom left
      texture = 31;
    } else if (y == 1 && x == 1) { // top left
      texture = 11;
    } else if (x == sheet_width - 2 && y == 1) { // top right
      texture = 14;
    } else if (x == sheet_width - 2 && y == sheet_height - 2) { // bottom right
      texture = 34;
    } else if (x == sheet_width - 2 &&
               !(y == 1 || y == sheet_height - 2)) { // right edge
      texture = (y >= midpoint_y - 2) ? (y <= midpoint_y) ? 23 : 24 : 24;
      flip_y  = rand() % 2;
    } else if (x == 1 && !(y == 1 || y == sheet_height - 2)) { // left edge
      texture = (y >= midpoint_y - 2) ? (y <= midpoint_y) ? 23 : 21 : 21;
      flip_y  = rand() % 2;
    } else if (y == 1 && !(x == 1 || x == sheet_width - 2)) { // top edge
      texture = (x >= midpoint_x - 1) ? (x <= midpoint_x + 1) ? 23 : 13 : 13;
      flip_x  = rand() % 2;
    } else if (y == sheet_height - 2 &&
               !(x == 1 || x == sheet_width - 2)) { // bottom edge
      texture = (x >= midpoint_x - 1) ? (x <= midpoint_x + 1) ? 23 : 32 : 32;
      flip_x  = rand() % 2;
    }

    // left side torches
    if (x == 1 && !(y == 0 || y >= sheet_height - 1) && y % 2 == 1 && y != 5) {
      quads[(sheet_width * sheet_height) + torches_placed] =
          (r_baked_quad){.x      = (float)x * 16.f,
                         .y      = (float)y * 16.f,
                         .width  = 16.f,
                         .height = 16.f,
                         .subtex = 91,
                         .layer  = 1,
                         .flip_x = 0,
                         .flip_y = 0};
      ++torches_placed;
      // right side torches
    } else if (x == sheet_width - 2 && !(y == 0 || y >= sheet_height - 1) &&
               y % 2 == 1 && y != 5) {
      quads[(sheet_width * sheet_height) + torches_placed] =
          (r_baked_quad){.x      = (float)x * 16.f,
                         .y      = (float)y * 16.f,
                         .width  = 16.f,
                         .height = 16.f,
                         .subtex = 91,
                         .layer  = 1,
                         .flip_x = 1,
                         .flip_y = 0};
      ++torches_placed;
      // top torches
    } else if (y == 0 && !(x == 0 || x >= sheet_width - 1) && x % 2 == 1 &&
               x != 9 && x != 11) {
      quads[(sheet_width * sheet_height) + torches_placed] =
          (r_baked_quad){.x      = (float)x * 16.f,
                         .y      = (float)y * 16.f,
                         .width  = 16.f,
                         .height = 16.f,
                         .subtex = 90,
                         .layer  = 1,
                         .flip_x = 0,
                         .flip_y = 0};
      ++torches_placed;
    }

    quads[i] = (r_baked_quad){.x      = (float)x * 16.f,
                              .y      = (float)y * 16.f,
                              .width  = 16.f,
                              .height = 16.f,
                              .subtex = texture,
                              .layer  = 0,
                              .flip_x = flip_x,
                              .flip_y = flip_y};
  }

  vec2 baked_sheet_pos = {0.f, 8.f};

  baked_sheet = r_baked_sheet_create(
      &sheet, quads, (sheet_width * sheet_height) + torches_placed,
      baked_sheet_pos);

  free(quads);

  int     eye_idle_frames[] = {0, 1, 2, 3};
  r_anim* eye_idle =
      load_anim(&character_sheet, "eye_idle", eye_idle_frames, 4, 12, 1);

  int     sword_swoosh_frames[] = {4, 5, 6};
  r_anim* sword_swoosh          = load_anim(&character_sheet, "sword_swoosh",
                                            sword_swoosh_frames, 3, 24, 0);

  int     goblin_idle_frames[] = {7, 8, 9, 10, 11, 12};
  r_anim* goblin_idle =
      load_anim(&character_sheet, "goblin_idle", goblin_idle_frames, 6, 8, 1);

  int     goblin_run_frames[] = {14, 15, 16, 17, 18, 19};
  r_anim* goblin_run =
      load_anim(&character_sheet, "goblin_walk", goblin_run_frames, 6, 10, 1);

  int     slime_idle_frames[] = {21, 22, 23, 24, 25, 26};
  r_anim* slime_idle =
      load_anim(&character_sheet, "slime_idle", slime_idle_frames, 6, 8, 1);

  int     slime_bounce_frames[] = {28, 29, 30, 31, 32, 33};
  r_anim* slime_bounce =
      load_anim(&character_sheet, "slime_walk", slime_bounce_frames, 6, 10, 1);

  int     player_idle_frames[] = {35, 36, 37, 38, 39, 40};
  r_anim* player_idle =
      load_anim(&character_sheet, "player_idle", player_idle_frames, 6, 12, 1);

  int     player_walk_frames[] = {42, 43, 44, 45, 46, 47};
  r_anim* player_walk =
      load_anim(&character_sheet, "player_walk", player_walk_frames, 6, 19, 1);

  vec4 particle_color;
  vec2 particle_size     = {1.5f, 1.5f};
  vec2 system_position   = {160.f, 90.f};
  vec2 system_size       = {8.f, 8.f};
  vec4 particle_velocity = {0.0f, 0.0f};
  r_get_color4f(particle_color, "#de1f1f");

  for (int i = 0; i < MAX_PARTICLE_EMITTERS; ++i) {
    r_particles* emitter = &level.emitters[i];
    *emitter = r_particles_create(50, 500.f, 15, 15, PARTICLE_COLORED, 1, 32);
    emitter->particle_layer = 100;
    r_particles_set_particle(emitter, particle_color, 500.f, particle_size,
                             particle_velocity);
    r_particles_set_color(emitter, particle_color, 1);

    r_particles_set_position(emitter, system_position);
    r_particles_set_size(emitter, system_size);

    // set the custom particle animator & spawn
    r_particles_set_spawner(emitter, particle_spawn);
    r_particles_set_animator(emitter, particle_animate);
  }

  vec2 sprite_size = {6.f, 6.f}, sprite_pos = {0.f, 0.f};

  // health orb initialization
  for (int i = 0; i < MAX_HEALTH_ORBS; ++i) {
    r_sprite sprite = r_sprite_create(shader, sprite_pos, sprite_size);
    r_sprite_set_tex(&sprite, &character_sheet, 20);
    sprite.layer                = 127;
    level.health_orbs[i].sprite = sprite;
    level.health_orbs[i].active = 0;
    vec2_clear(level.health_orbs[i].center);
  }

  // 16x9 * 20
  camera_size[0] = 320.f;
  camera_size[1] = 180.f;
  // vec2 camera_offset = {-120.f, -80.f};
  r_camera_set_size(r_ctx_get_camera(render_ctx), camera_size);
  // r_camera_set_position(r_ctx_get_camera(render_ctx), camera_offset);
}

void init_game(void) {
  int width  = 320 / 16.f;
  int height = 180 / 16.f;

  // initialize player stuff _before_ enemies so they're not within range of
  // spawn
  vec2   player_pos      = {152.f, 82.f};
  vec2   player_halfsize = {4.5f, 5.f};
  c_aabb player_col      = c_aabb_create(player_pos, player_halfsize);
  vec2   sword_pos       = {player_pos[0] + 12.f, player_pos[1] + 4.f};
  vec2   sword_halfsize  = {10.f, 4.f};
  c_aabb sword_col       = c_aabb_create(sword_pos, sword_halfsize);
  vec2   zero = {0.f, 0.f}, halfsize = {8.f, 8.f};
  vec2   sprite_size = {16.f, 16.f};

  level.player        = (player_t){.aabb = player_col, .hitbox = sword_col, 0};
  level.player.health = MAX_PLAYER_HEALTH;
  vec2_dup(level.player.center, player_pos);
  level.player.sprite       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.sprite.layer = 4;
  r_sprite_set_tex(&level.player.sprite, &character_sheet, 42);

  level.player.sword       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.sword.layer = 5;
  r_sprite_set_tex(&level.player.sword, &character_sheet, 13);

  level.player.swoosh       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.swoosh.layer = 100;
  r_anim* sword_swoosh      = r_anim_get_name(render_ctx, "sword_swoosh");
  r_sprite_set_anim(&level.player.swoosh, sword_swoosh);

  // up
  level.spawn_points[0][0] = 152.f;
  level.spawn_points[0][1] = -16.f;

  // down
  level.spawn_points[1][0] = 152.f;
  level.spawn_points[1][1] = 196.f;

  // left
  level.spawn_points[2][0] = -16.f;
  level.spawn_points[2][1] = 82.f;

  // right
  level.spawn_points[3][0] = 336.f;
  level.spawn_points[3][1] = 82.f;
}

// initialize player data

// (320 / 2) - (16 / 2), (180 / 2) - (16 / 2)
// centered position

void clear_level(void) {
  if (level.enemies)
    free(level.enemies);
  level.enemy_count = 0;
  for (int i = 0; i < MAX_HEALTH_ORBS; ++i) {
    level.health_orbs[i].active = 0;
  }

  for (int i = 0; i < MAX_PARTICLE_EMITTERS; ++i) {
    r_particles_reset(&level.emitters[i]);
  }
}

void init_level(void) {
  // enemies
  /*level.to_spawn =
      rnd_range((MAX_ENEMIES * (cont_round % 4)) / ((cont_round % 4) + 1),
                (MAX_ENEMIES * ((cont_round % 4) + 1)) /
                    ((cont_round % 4) + 2)) *
      ((cont_round / 4) + 1);*/
  int to_spawn = 2;
  for (int i = 0; i < cont_round; ++i) {
    to_spawn += 3 * (int)pow(2, (i / 7));
  }
  level.to_spawn       = to_spawn;
  level.enemy_capacity = MAX_ENEMIES;
  level.enemies        = (enemy_t*)calloc(sizeof(enemy_t), MAX_ENEMIES);

  // player
  level.player.health         = MAX_PLAYER_HEALTH;
  level.player.damage_timer   = 0;
  level.player.sprite.visible = 1;
  level.player.sword.visible  = 1;
  level.player.swoosh.visible = 1;
  level.player.allow_spawns   = 0;

  if (!is_continuous) {
    level.player.kill_count = 0;
  }

  vec2 player_pos      = {152.f, 82.f};
  vec2 player_halfsize = {4.5f, 5.f};
  c_aabb_set(&level.player.aabb, player_pos, player_halfsize);
  vec2_dup(level.player.center, player_pos);

  int max_pre = level.to_spawn > MAX_PRESPAWN ? MAX_PRESPAWN : level.to_spawn;

  for (int i = 0; i < max_pre; ++i) {
    if (spawn_enemy()) {
      --level.to_spawn;
    }
  }
}

void game_resized_to(vec2 size) {
  r_set_can_render(render_ctx, 0);

  ui_ctx_resize(u_ctx, size);
  r_framebuffer_destroy(fbo);
  r_framebuffer_destroy(ui_fbo);
  fbo            = r_framebuffer_create(size[0], size[1], fbo_shader, 0);
  ui_fbo         = r_framebuffer_create(size[0], size[1], fbo_shader, 0);
  window_size[0] = size[0];
  window_size[1] = size[1];

  r_set_can_render(render_ctx, 1);
}

void handle_ui(void) {
  // prevent any false usages
  if (!menu.current_page)
    return;

  // All the input handling for the UI
  switch (menu.page_number) {
    case MENU_NONE: // none
      break;
    case MENU_MAIN: { //
      if (ui_tree_check_event(&menu.main_page, menu.play.id) == 1) {
        play_sfx(a_res.s_menu_select);
        game_state = GAME_PLAY;
        init_level();
        menu_set_page(MENU_NONE);
        return;
      }

      if (ui_tree_check_event(&menu.main_page, menu.settings.id) == 1) {
        menu_set_page(MENU_SETTINGS);
        play_sfx(a_res.s_menu_select);
      }

      if (ui_tree_check_event(&menu.main_page, menu.quit.id) == 1) {
        // quit game
        play_sfx(a_res.s_menu_select);
        menu_set_page(MENU_NONE);
        game_state = GAME_QUIT;
        return;
      }
    } break;
    case MENU_SETTINGS: {
      if (i_mouse_up(input_ctx, MOUSE_LEFT)) {
        if (menu.music_vol.holding) {
          menu.music_vol.holding = 0;
        } else if (menu.sfx_vol.holding) {
          menu.sfx_vol.holding = 0;
        } else if (menu.master_vol.holding) {
          menu.master_vol.holding = 0;
        }
      }

      uint8_t can_joy  = menu.joy_timer <= 0.0f;
      float   joy_hori = i_joy_axis(input_ctx, JOY_ID, XBOX_L_X);
      float   joy_vert = i_joy_axis(input_ctx, JOY_ID, XBOX_L_Y) * -1.0f;

      if (fabs(joy_vert) > fabs(joy_hori)) {
        joy_hori = 0.0f;
      }

      if (i_binding_clicked(input_ctx, "right") ||
          (can_joy && joy_hori > JOY_DEADZONE)) {
        if (ui_tree_is_active(u_ctx, menu.current_page, menu.master_vol.id)) {
          ui_slider_next_step(&menu.master_vol);
          a_listener_set_gain(audio_ctx, menu.master_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.sfx_vol.id)) {
          ui_slider_next_step(&menu.sfx_vol);
          a_layer_set_gain(audio_ctx, a_res.sfx_layer, menu.sfx_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.music_vol.id)) {
          ui_slider_next_step(&menu.music_vol);
          a_layer_set_gain(audio_ctx, a_res.music_layer, menu.music_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        }
      }

      if (i_binding_clicked(input_ctx, "left") ||
          (can_joy && joy_hori < -JOY_DEADZONE)) {
        if (ui_tree_is_active(u_ctx, menu.current_page, menu.master_vol.id)) {
          ui_slider_prev_step(&menu.master_vol);
          a_listener_set_gain(audio_ctx, menu.master_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.sfx_vol.id)) {
          ui_slider_prev_step(&menu.sfx_vol);
          a_layer_set_gain(audio_ctx, a_res.sfx_layer, menu.sfx_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.music_vol.id)) {
          ui_slider_prev_step(&menu.music_vol);
          a_layer_set_gain(audio_ctx, a_res.music_layer, menu.music_vol.value);
          play_sfx(a_res.s_menu_move);
          menu.joy_timer = 150.0f;
        }
      }

      if (ui_tree_check_event(&menu.settings_page, menu.back_button.id) == 1) {
        play_sfx(a_res.s_menu_select);
        menu_set_page(menu.last_page);
      }

      if (menu.master_vol.holding) {
        float gain_prev = a_listener_get_gain(audio_ctx);
        if (gain_prev != menu.master_vol.value) {
          play_sfx(a_res.s_menu_move);
        }
        a_listener_set_gain(audio_ctx, menu.master_vol.value);
        user_prefs.master_vol = menu.master_vol.value;
      }

      if (menu.sfx_vol.holding) {
        float gain_prev = a_layer_get_gain(audio_ctx, a_res.sfx_layer);
        if (gain_prev != menu.sfx_vol.value) {
          play_sfx(a_res.s_menu_move);
        }

        a_layer_set_gain(audio_ctx, a_res.sfx_layer, menu.sfx_vol.value);
      }

      if (menu.music_vol.holding) {
        float gain_prev = a_layer_get_gain(audio_ctx, a_res.music_layer);
        if (gain_prev != menu.music_vol.value) {
          play_sfx(a_res.s_menu_move);
        }

        a_layer_set_gain(audio_ctx, a_res.music_layer, menu.music_vol.value);
      }

      int32_t event_type = 0;
      if ((event_type =
               ui_tree_check_event(&menu.settings_page, menu.res_dd.id)) == 1) {
        if (menu.res_dd.showing) {
          menu.res_dd.showing = 0;
        } else {
          menu.res_dd.showing = 1;
        }

        if (ui_dropdown_has_change(&menu.res_dd) && !menu.res_dd.showing) {
          GLFWvidmode* modes = (GLFWvidmode*)menu.res_dd.data;
          r_select_vidmode(render_ctx, modes[menu.res_dd.selected], 0, 1, 0);
          r_window_get_vsize(render_ctx, window_size);
          game_resized_to(window_size);
        }
      }

    } break;
    case MENU_PAUSE: {
      if (ui_tree_check_event(&menu.pause_page, menu.p_settings.id) == 1) {
        play_sfx(a_res.s_menu_select);
        menu_set_page(MENU_SETTINGS);
      }

      if (ui_tree_check_event(&menu.pause_page, menu.p_resume.id) == 1) {
        play_sfx(a_res.s_menu_select);
        menu_set_page(MENU_NONE);
        game_state = GAME_PLAY;
        return;
      }

      if (ui_tree_check_event(&menu.pause_page, menu.p_quit.id) == 1) {
        play_sfx(a_res.s_menu_select);
        // clear old framebuffer
        r_framebuffer_bind(fbo);
        r_window_clear_color_empty();
        r_window_clear();
        r_framebuffer_unbind();
        clear_level();
        menu_set_page(MENU_MAIN);
        game_state    = GAME_START;
        is_continuous = 0;
        return;
      }
    } break;
    case MENU_END:
      if (ui_tree_check_event(&menu.end_page, menu.w_quit.id) == 1) {
        r_framebuffer_bind(fbo);
        r_window_clear_color_empty();
        r_window_clear();
        r_framebuffer_unbind();

        clear_level();
        menu_set_page(MENU_MAIN);
        game_state              = GAME_START;
        level.player.kill_count = 0;
      }

      if (ui_tree_check_event(&menu.end_page, menu.w_again.id) == 1) {
        r_framebuffer_bind(fbo);
        r_window_clear_color_empty();
        r_window_clear();
        r_framebuffer_unbind();

        if (game_state == GAME_WIN) {
          is_continuous = 1;
          ++cont_round;
        } else {
          is_continuous = 0;
        }

        clear_level();
        game_state = GAME_PLAY;
        init_level();
        menu_set_page(MENU_NONE);
      }
      break;
  }
}

void input(float delta) {
  vec2 mouse_pos = {i_mouse_get_x(input_ctx), i_mouse_get_y(input_ctx)};
  ui_ctx_update(u_ctx, mouse_pos);

  if (menu.current_page) {
    int32_t active = ui_tree_check(u_ctx, menu.current_page);

    if (i_mouse_clicked(input_ctx, MOUSE_LEFT)) {
      ui_tree_select(u_ctx, menu.current_page, 1, 1);
    }

    if (i_mouse_released(input_ctx, MOUSE_LEFT)) {
      ui_tree_select(u_ctx, menu.current_page, 0, 1);
    }

    uint8_t joy_b = i_joy_clicked(input_ctx, JOY_ID, XBOX_B);
    if (i_key_clicked(input_ctx, KEY_ESCAPE) || joy_b) {
      play_sfx(a_res.s_menu_select);
      switch (menu.page_number) {
        case MENU_MAIN: {
          // Don't quit the game when B is pressed
          if (joy_b)
            break;

          // quit game
          game_state = GAME_QUIT;
          menu_set_page(MENU_NONE);
          return;
        } break;
        case MENU_PAUSE: {
          // resume
          game_state = GAME_PLAY;
          menu_set_page(MENU_NONE);
          return;
        } break;
        case MENU_SETTINGS: {
          // go back a page
          menu_set_page(menu.last_page);
        } break;
        case MENU_END: {
          r_framebuffer_bind(fbo);
          r_window_clear_color_empty();
          r_window_clear();
          r_framebuffer_unbind();

          clear_level();

          game_state = GAME_START;
          menu_set_page(MENU_MAIN);
        } break;
      }
    }

    float   joy_vert = i_joy_axis(input_ctx, JOY_ID, XBOX_L_Y) * -1.0f;
    float   joy_hori = i_joy_axis(input_ctx, JOY_ID, XBOX_L_X);
    uint8_t can_joy  = menu.joy_timer <= 0.0f;

    float abs_vert = fabs(joy_vert);
    float abs_hori = fabs(joy_hori);

    // Clamp to single direction at a time for menuing
    if (abs_hori > abs_vert) {
      joy_vert = 0.0f;
    } else if (abs_vert > abs_hori) {
      joy_hori = 0.0f;
    }

    if (i_binding_clicked(input_ctx, "down") ||
        (joy_vert <= -JOY_DEADZONE && can_joy)) {
      menu.joy_timer = 250.0f;
      ui_tree_next(menu.current_page);
      play_sfx(a_res.s_menu_move);
    }

    if (i_binding_clicked(input_ctx, "up") ||
        (joy_vert >= JOY_DEADZONE && can_joy)) {
      menu.joy_timer = 250.0f;
      ui_tree_prev(menu.current_page);
      play_sfx(a_res.s_menu_move);
    }

    if (menu.joy_timer > 0.0f) {
      menu.joy_timer -= delta;
    }

    if (i_binding_clicked(input_ctx, "select") ||
        i_joy_clicked(input_ctx, JOY_ID, XBOX_A)) {
      ui_tree_select(u_ctx, menu.current_page, 1, 0);
      play_sfx(a_res.s_menu_move);
    }

    if (menu.scroll_timer > menu.scroll_duration) {
      menu.scroll_timer += delta;
    } else {
      double scroll_y = i_scroll_get_dy(input_ctx);
      if (scroll_y > 0.f) {
        ui_tree_scroll_up(menu.current_page, 1, 1);
        i_scroll_reset(input_ctx); // for responsiveness
      } else if (scroll_y < 0.f) {
        ui_tree_scroll_down(menu.current_page, 1, 1);
        i_scroll_reset(input_ctx); // for responsiveness
      }
    }

    handle_ui();
  } else {
    if (i_key_clicked(input_ctx, KEY_ESCAPE) ||
        i_joy_clicked(input_ctx, JOY_ID, XBOX_START)) {
      if (game_state == GAME_PLAY) {
        game_state = GAME_PAUSE;
        menu_set_page(MENU_PAUSE);
      }
    }

#ifdef DRAW_HITBOXES
    if (i_key_clicked(input_ctx, KEY_P)) {
      debug_output();
    }
#endif

    // Handle User Input normally now
    vec2  move = {0.f};
    int   vert = 0, hori = 0;
    float joy_vert = 0.0f, joy_hori = 0.0f;

    if (i_joy_exists(input_ctx, JOY_ID)) {
      joy_vert = i_joy_axis(input_ctx, JOY_ID, XBOX_L_Y) * -1.0f;
      joy_hori = i_joy_axis(input_ctx, JOY_ID, XBOX_L_X);
    }

    if (i_binding_up(input_ctx, "up") && joy_vert <= -JOY_DEADZONE) {
      level.player.up_priority = 0;
    }

    if (i_binding_up(input_ctx, "down") && joy_vert >= JOY_DEADZONE) {
      level.player.down_priority = 0;
    }

    if (i_binding_clicked(input_ctx, "up") || joy_vert > JOY_DEADZONE) {
      level.player.down_priority = 0;
      level.player.up_priority   = 1;
    }

    if (i_binding_clicked(input_ctx, "down") || joy_vert < -JOY_DEADZONE) {
      level.player.down_priority = 1;
      level.player.up_priority   = 0;
    }

    if (i_binding_up(input_ctx, "up") && joy_vert <= JOY_DEADZONE) {
      level.player.up_priority = 0;
    }

    if (i_binding_up(input_ctx, "down") && joy_vert >= -JOY_DEADZONE) {
      level.player.down_priority = 0;
    }

    if ((i_binding_down(input_ctx, "up") || joy_vert > JOY_DEADZONE) &&
        !level.player.down_priority) {
      vert = 1;
    }

    if ((i_binding_down(input_ctx, "down") || joy_vert < -JOY_DEADZONE) &&
        !level.player.up_priority) {
      level.player.down_priority = 0;
      vert                       = -1;
    }

    level.player.vert = vert;
    move[1]           = -(float)vert;

    if (i_binding_clicked(input_ctx, "left") || joy_hori < -JOY_DEADZONE) {
      level.player.left_priority  = 1;
      level.player.right_priority = 0;
    }

    if (i_binding_clicked(input_ctx, "right") || joy_hori > JOY_DEADZONE) {
      level.player.left_priority  = 0;
      level.player.right_priority = 1;
    }

    if (i_binding_up(input_ctx, "right") && joy_hori <= JOY_DEADZONE) {
      level.player.right_priority = 0;
    }

    if (i_binding_up(input_ctx, "left") && joy_hori >= -JOY_DEADZONE) {
      level.player.left_priority = 0;
    }

    if ((i_binding_down(input_ctx, "left") || joy_hori < -JOY_DEADZONE) &&
        !level.player.right_priority) {
      hori = -1;
    }

    if ((i_binding_down(input_ctx, "right") || joy_hori > JOY_DEADZONE) &&
        !level.player.left_priority) {
      hori = 1;
    }

    level.player.hori = hori;
    move[0]           = (float)hori;

    if (i_binding_clicked(input_ctx, "attack") ||
        i_joy_clicked(input_ctx, JOY_ID, XBOX_A)) {
      if (level.player.attack_timer <= 0.f) {
        r_sprite_anim_play(&level.player.swoosh);
        play_sfx(a_res.s_attack);
        level.player.attack_timer = ATTACK_COOLDOWN;
      }
    }

    int swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);

    if (move[0] > 0.f) {
      level.player.sprite.flip_x = 0;
      level.player.sword.flip_x  = 0;

      if (!swoosh_state)
        level.player.swoosh.flip_x = 0;
    } else if (move[0] < 0.f) {
      level.player.sprite.flip_x = 1;
      level.player.sword.flip_x  = 1;
      if (!swoosh_state)
        level.player.swoosh.flip_x = 1;
    }

    r_anim* player_anim = 0;
    int     change      = 0;
    if (move[0] != 0.f || move[1] != 0.f) {
      if (level.player.is_idle) {
        player_anim          = r_anim_get_name(render_ctx, "player_walk");
        level.player.is_idle = 0;
        change               = 1;
      }
    } else {
      if (!level.player.is_idle) {
        player_anim          = r_anim_get_name(render_ctx, "player_idle");
        level.player.is_idle = 1;
        change               = 1;
      }
    }

    if (change && player_anim) {
      r_sprite_set_anim(&level.player.sprite, player_anim);
      r_sprite_anim_play(&level.player.sprite);
    }

    // move player or something
    if (move[0] != 0.f && move[1] != 0.f) {
      move[0] *= 0.75f;
      move[1] *= 0.75f;
    }

    c_aabb_move(&level.player.aabb, move);
    update_player_sprites();
  }
}

// weight (0 = 100% a, 1 = 100% b, .50 = 50 a 50 b)
static void blend(vec4 dst, vec4 a, vec4 b, float weight) {
  dst[0] = (a[0] * (1.f - weight)) + (b[0] * weight);
  dst[1] = (a[1] * (1.f - weight)) + (b[1] * weight);
  dst[2] = (a[2] * (1.f - weight)) + (b[2] * weight);
}

void update(time_s delta) {
  if (game_state == GAME_PLAY) {
    if (level.player.attack_timer > 0.f) {
      level.player.attack_timer -= delta;
    }

    if (level.to_spawn != 0) {
      level.spawn_timer -= delta;
      if (level.spawn_timer <= 0 &&
          level.enemy_count < level.enemy_capacity - 1) {
        level.spawn_timer = rnd_range((MAX_SPAWN_RATE / 4) * 3, MAX_SPAWN_RATE);
        if (spawn_enemy()) {
          --level.to_spawn;
        }
      }
    }

    if (level.player.take_damage) {
      level.player.damage_timer -= delta;

      float vibration_amount =
          sin((level.player.damage_timer / DAMAGE_DURATION) * 6.28f) * 0.2f;

      if (level.player.damage_timer <= DAMAGE_DURATION * 0.5f) {
        vibration_amount = 0.0f;
      }

      if (level.player.damage_timer <= 0.f) {
        level.player.take_damage = 0;
      }

      i_joy_set_vibration(input_ctx, JOY_ID, vibration_amount,
                          vibration_amount);

      blend(level.player.sprite.color, menu.white, menu.red,
            level.player.damage_timer / DAMAGE_DURATION);
      blend(level.player.sword.color, menu.white, menu.red,
            level.player.damage_timer / DAMAGE_DURATION);
    }

    // update health orbs
    for (int i = 0; i < MAX_HEALTH_ORBS; ++i) {
      health_orb_t* orb = &level.health_orbs[i];
      if (orb->active) {
        orb->timer += delta;

        health_orb_t* orb    = &level.health_orbs[i];
        r_sprite*     sprite = &orb->sprite;

        float remaining = MAX_HEALTH_ORB_LIFE - orb->timer;
        float fade      = remaining / MAX_HEALTH_ORB_LIFE;
        float glow      = 0.7f + (sin(orb->timer / 200.159f) / 3.14f);
        vec2  floating  = {0.f, cos(orb->timer / 400.3f) / 16.f};
        r_sprite_move(sprite, floating);
        sprite->color[0] = glow * fade;
        sprite->color[1] = glow * fade;
        sprite->color[2] = glow * fade;
        sprite->color[3] = glow * fade;

        if (orb->timer >= MAX_HEALTH_ORB_LIFE) {
          orb->active = 0;
          continue;
        }

        float dist = vec2_dist(orb->center, level.player.center);
        if (dist < 30.f && level.player.health < MAX_PLAYER_HEALTH) {
          // give player health if closer than 9 units
          if (dist < 9.f) {
            if (level.player.health < MAX_PLAYER_HEALTH) {
              orb->active = 0;
              ++level.player.health;
            }
            // move towards player
          } else {
            float move_factor = 0.0035f;
            float dist_factor = 1.f / (30.f / (dist));
            vec2  move_amount = {(level.player.center[0] - orb->center[0]) *
                                     delta * dist_factor * move_factor,
                                 (level.player.center[1] - orb->center[1]) *
                                     delta * dist_factor * move_factor};
            vec2_add(orb->center, orb->center, move_amount);
            r_sprite_move(&orb->sprite, move_amount);
          }
        }
      }
    }

    // test player vs enemies
    for (int i = 0; i < level.enemy_capacity; ++i) {
      enemy_t* en = &level.enemies[i];

      if (!en->alive) {
        continue;
      }

      if (en->state != ENEMY_SPAWN) {
        // enemy AI
        float dist = vec2_dist(en->circle.center, level.player.center);
        // if within range, move towards
        if (dist > 7.f) {
          int is_left = level.player.center[0] < en->circle.center[0];
          if (is_left) {
            en->sprite.flip_x = 1;
          } else {
            en->sprite.flip_x = 0;
          }

          float move_factor = 0.005f;
          vec2  move        = {level.player.center[0] - en->circle.center[0],
                               level.player.center[1] - en->circle.center[1]};
          vec2_norm(move, move);
          vec2_scale(move, move, en->move_speed * (delta * move_factor));
          move_enemy(en, move);

          // set animaiton from idle (not walk) to walk
          if (en->state != ENEMY_WALK) {
            en->state = ENEMY_WALK;

            // eyes only have the one animation
            if (en->type != ENEMY_EYE) {
              r_anim* anim = 0;
              switch (en->type) {
                case ENEMY_SLIME:
                  anim = r_anim_get_name(render_ctx, "slime_walk");
                  break;
                case ENEMY_GOBLIN:
                  anim = r_anim_get_name(render_ctx, "goblin_walk");
                  break;
              }

              r_sprite_set_anim(&en->sprite, anim);
              r_sprite_anim_play(&en->sprite);
            }
          }
        } else {
          // set animaiton from walk to idle
          if (en->state == ENEMY_WALK) {
            en->state = ENEMY_IDLE;

            if (en->type != ENEMY_EYE) {
              r_anim* anim = 0;
              switch (en->type) {
                case ENEMY_SLIME:
                  anim = r_anim_get_name(render_ctx, "slime_idle");
                  break;
                case ENEMY_GOBLIN:
                  anim = r_anim_get_name(render_ctx, "goblin_idle");
                  break;
              }

              r_sprite_set_anim(&en->sprite, anim);
              r_sprite_anim_play(&en->sprite);
            }
          }
        }
      } else {
        // move towards initial spawn point
        float dist = vec2_dist(en->circle.center, en->exit_point);
        if (dist <= 4.f) {
          en->state = ENEMY_WALK;
        } else {
          int is_left = level.player.center[0] < en->circle.center[0];
          if (is_left) {
            en->sprite.flip_x = 1;
          } else {
            en->sprite.flip_x = 0;
          }

          // move towards exit spawn point
          float move_factor = 0.005f;
          vec2  move_amount;
          vec2_sub(move_amount, en->exit_point, en->circle.center);
          vec2_norm(move_amount, move_amount);
          vec2_scale(move_amount, move_amount,
                     en->move_speed * (delta * move_factor));
          move_enemy(en, move_amount);
        }
      }

      // animate damaged enemies
      if (en->take_damage) {
        en->damage_timer -= delta;
        if (en->damage_timer < (DAMAGE_DURATION * 0.66f)) {
          en->can_take_damage = 1;
        } else {
          en->can_take_damage = 0;
        }
        blend(en->sprite.color, menu.white, menu.red,
              en->damage_timer / DAMAGE_DURATION);

        // calculate elapsed time
        float elapsed = DAMAGE_DURATION - en->damage_timer;
        if (elapsed < STAGGER_DURATION) {
          float lerp =
              f_invquad(0.f, STAGGER_DURATION, elapsed) / STAGGER_DURATION;

          float stagger_scale = 0.7f;
          vec2  move_amount;
          vec2_scale(move_amount, en->stagger, lerp * stagger_scale);
          move_enemy(en, move_amount);
        }

        if (en->damage_timer <= 0.f) {
          en->take_damage = 0;
          vec2_clear(en->stagger);
        }
      }

      // i frame player when attacking for 3/4ths of cooldown
      if (level.player.attack_timer < ATTACK_COOLDOWN * 0.25f) {
        c_manifold man = c_aabb_vs_circle_man(level.player.aabb, en->circle);
        if (man.distance != 0.f) {
          // take damage I guess :shrug:
          if (!level.player.take_damage) {
            level.player.take_damage = 1;
            --level.player.health;

            vec2 zero = {0.f, 0.f};
            if (level.player.health <= 0) {
              level.player.sprite.visible = 0;
              level.player.swoosh.visible = 0;
              level.player.sword.visible  = 0;
              create_emitter(level.player.center, zero);
            } else {
              play_sfx(a_res.s_player_hit);
            }

            level.player.damage_timer = DAMAGE_DURATION;
          }
        }
      }
      // enemy vs environment
      for (int i = 0; i < level.env_cols; ++i) {
        c_aabb     col = level.env[i];
        c_manifold man = c_aabb_vs_circle_man(col, en->circle);
        if (man.distance != 0.f) {
          vec2 move_amount = {0.f, 0.f};
          vec2_scale(move_amount, man.direction, -1.f * man.distance);
          move_enemy(en, move_amount);
          vec2_clear(en->stagger);
        }
      }
    }

    int swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);
    if (swoosh_state == R_ANIM_PLAY) {
      // test sword vs enemies
      for (int i = 0; i < level.enemy_capacity; ++i) {
        enemy_t* en = &level.enemies[i];

        // only allow damage after a 3rd of the damage timer has elapsed
        if (en->can_take_damage) {
          c_manifold man =
              c_aabb_vs_circle_man(level.player.hitbox, en->circle);
          if (man.distance != 0.f) {
            en->take_damage  = 1;
            en->state        = ENEMY_HIT;
            en->damage_timer = DAMAGE_DURATION;
            en->health -= 1;
            en->can_take_damage = 0;

            vec2 a = {0.f, 0.f};
            vec2_sub(a, en->circle.center, level.player.center);
            vec2_norm(a, a);
            vec2 stagger = {a[0] * KNOCKBACK_AMOUNT, a[1] * KNOCKBACK_AMOUNT};
            vec2_dup(en->stagger, stagger);

            // apply opposite directional force to enemy
            if (en->health <= 0) {
              en->alive = 0;
              // death animation/etc
              create_emitter(en->circle.center, man.direction);

              // health orb
              int to_spawn = rand() % 2;
              if (level.player.health < (MAX_PLAYER_HEALTH / 3) + 1) {
                level.player.allow_spawns = 1;
              }

              if (to_spawn && level.player.allow_spawns &&
                  level.player.health != MAX_PLAYER_HEALTH) {
                spawn_health_orb(en->circle.center);
              }

              // ENEMY RESPAWN
              // to_spawn = rand() % 3;

              play_sfx(a_res.s_enemy_die);
              ++level.player.kill_count;
              --level.enemy_count;
              // if (to_spawn == 2) {
              //  ++level.to_spawn;
              //}
            } else {
              play_sfx(a_res.s_enemy_hit);
            }
          }
        }
      }
    }

    // test player vs environment
    for (int i = 0; i < level.env_cols; ++i) {
      c_manifold man = c_aabb_vs_aabb_man(level.player.aabb, level.env[i]);
      if (man.distance != 0.f) {
        c_aabb_adjust(&level.player.aabb, man);
      }
    }

    // test player vs spawn colliders (prevents out of bounds thru spawn
    // routes)
    for (int i = 0; i < 4; ++i) {
      c_manifold man =
          c_aabb_vs_aabb_man(level.player.aabb, level.spawn_cols[i]);
      if (man.distance != 0.f) {
        c_aabb_adjust(&level.player.aabb, man);
      }
    }

    c_aabb_get_center(level.player.center, level.player.aabb);

    if (level.enemy_count == 0) {
      game_end(1);
    }

    if (level.player.health == 0) {
      game_end(0);
    }
  }
}

static void debug_circle(vec2 px, float radius_px, vec4 color) {
#ifdef DRAW_HITBOXES
  vec2 _px;
  vec2_div(_px, px, camera_size);
  float _radius = radius_px * (window_size[0] / camera_size[0]);
  ui_im_circle_draw(u_ctx, _px, _radius, 1.f, color);
#endif
}

static void debug_box(vec2 center, vec2 size, vec4 color) {
#ifdef DRAW_HITBOXES
  vec2 _center, _size;

  center[0] -= size[0] * 0.5f;
  center[1] -= size[1] * 0.5f;

  vec2_div(_center, center, camera_size);
  vec2_div(_size, size, camera_size);
  ui_im_box_draw(u_ctx, _center, _size, color);
#endif
}

void debug_game(void) {
#ifdef DRAW_HITBOXES
  ui_frame_start(u_ctx);

  vec4 enemy_color, swoosh_color, player_color, env_color, spawn_col_color;
  r_get_color4f(enemy_color, "#0f0");
  r_get_color4f(swoosh_color, "#f0f");
  r_get_color4f(player_color, "#fff");
  r_get_color4f(env_color, "#f00");
  r_get_color4f(spawn_col_color, "#ff0");
  enemy_color[3]     = 0.4f;
  swoosh_color[3]    = 0.4f;
  player_color[3]    = 0.3f;
  env_color[3]       = 0.3f;
  spawn_col_color[3] = 0.3f;

  for (int i = 0; i < level.enemy_capacity; ++i) {
    enemy_t* en = &level.enemies[i];
    if (en->alive)
      debug_circle(en->circle.center, en->circle.radius, enemy_color);
  }

  vec2 center = {0.f, 0.f}, size = {0.f, 0.f};
  if (level.player.swoosh.visible) {
    c_aabb_get_center(center, level.player.hitbox);
    c_aabb_get_size(size, level.player.hitbox);
    debug_box(center, size, swoosh_color);
  }

  for (int i = 0; i < level.env_cols; ++i) {
    c_aabb level_col = level.env[i];
    c_aabb_get_center(center, level_col);
    c_aabb_get_size(size, level_col);
    debug_box(center, size, env_color);
  }

  // spawn hitboxes
  for (int i = 0; i < 4; ++i) {
    c_aabb level_col = level.spawn_cols[i];
    c_aabb_get_center(center, level_col);
    c_aabb_get_size(size, level_col);
    debug_box(center, size, env_color);
  }

  c_aabb_get_center(center, level.player.aabb);
  c_aabb_get_size(size, level.player.aabb);
  debug_box(center, size, player_color);

  ui_frame_end(u_ctx);
#endif
}

void draw_ui(void) {
  r_framebuffer_bind(ui_fbo);
  r_window_clear_color_empty();
  r_window_clear();

  ui_frame_start(u_ctx);

  if (game_state == GAME_PLAY || game_state == GAME_PAUSE) {
    vec2  pos       = {0.0025f, 0.025f};
    float slot_size = 0.03f;

    for (int i = 0; i < (MAX_PLAYER_HEALTH / 2); ++i) {
      pos[0] += slot_size;
      int round = (i + 1) * 2;
      if (round > level.player.health) {
        if (round - 1 == level.player.health) {
          ui_im_img_draw(u_ctx, menu.heart_half, pos, menu.heart_empty.size);
        } else {
          ui_im_img_draw(u_ctx, menu.heart_empty, pos, menu.heart_empty.size);
        }
      } else {
        ui_im_img_draw(u_ctx, menu.heart_full, pos, menu.heart_full.size);
      }
    }

    pos[0]                          = 0.0325f;
    pos[1]                          = 0.1f;
    static char kill_count_text[16] = {0};
    snprintf(kill_count_text, 16, "KILL COUNT: %i", level.player.kill_count);
    ui_im_text_draw(u_ctx, pos, 32.f, menu.font, kill_count_text);
    memset(kill_count_text, 0, sizeof(char) * 16);
    pos[1] = 0.95f;
    snprintf(kill_count_text, 16, "ROUND: %i", cont_round + 1);
    ui_im_text_draw(u_ctx, pos, 32.f, menu.font, kill_count_text);
  }

  if (game_state != GAME_PLAY && game_state != GAME_QUIT) {
    ui_tree_draw(u_ctx, menu.current_page);
  }

  if (game_state == GAME_PLAY || game_state == GAME_PAUSE) {
    debug_game();
  }

  ui_frame_end(u_ctx);
}

void draw_game(time_s delta) {
  r_framebuffer_bind(fbo);
  r_window_clear_color_empty();
  r_window_clear();

  // draw the background world
  r_ctx_update(render_ctx);
  r_baked_sheet_draw(render_ctx, baked, &baked_sheet);

  // draw the player
  if (game_state == GAME_PLAY) {
    r_sprite_update(&level.player.sprite, delta);
    r_sprite_update(&level.player.sword, delta);
    r_sprite_update(&level.player.swoosh, delta);
  }

  // draw (or don't) the swoosh
  int swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);
  if (swoosh_state == R_ANIM_STOP) {
    level.player.swoosh.visible = 0;
  } else {
    level.player.swoosh.visible = 1;
  }

  // draw the character sprites
  r_sprite_draw_batch(render_ctx, &level.player.sprite);
  r_sprite_draw_batch(render_ctx, &level.player.sword);
  r_sprite_draw_batch(render_ctx, &level.player.swoosh);

  for (int i = 0; i < MAX_HEALTH_ORBS; ++i) {
    if (level.health_orbs[i].active) {
      health_orb_t* orb    = &level.health_orbs[i];
      r_sprite*     sprite = &orb->sprite;

      r_sprite_update(sprite, delta);
      r_sprite_draw_batch(render_ctx, sprite);
    }
  }

  // draw the enemies
  for (int i = 0; i < level.enemy_capacity; ++i) {
    if (level.enemies[i].alive) {
      if (game_state == GAME_PLAY)
        r_sprite_update(&level.enemies[i].sprite, delta);

      r_sprite_draw_batch(render_ctx, &level.enemies[i].sprite);
    }
  }

  // draw particle effects
  for (int i = 0; i < MAX_PARTICLE_EMITTERS; ++i) {
    r_particles* emitter = &level.emitters[i];
    if (!r_particles_finished(emitter)) {
      if (game_state == GAME_PLAY || game_state == GAME_LOSE)
        r_particles_update(emitter, delta);
      r_particles_draw(render_ctx, emitter, particle_shader);
    }
  }

  // issue a draw command for anything left in the batches
  r_ctx_draw(render_ctx);
}

void render(time_s delta) {
  if (game_state != GAME_START && game_state != GAME_QUIT) {
    // draw sprites
    draw_game(delta);
  }

  draw_ui();
}

user_preferences load_config(void) {
  user_preferences prefs     = {0};
  asset_t*         raw_table = asset_get(USER_PREFS_FILE);

  if (raw_table) {
    s_table table = s_table_get(raw_table->data);

    const char* value = 0;
    value             = s_table_find(&table, "width");
    int valid_res     = 1;
    if (!value) {
      valid_res = 0;
    }
    prefs.res_width  = (value && valid_res) ? atoi(value) : DEFAULT_WIDTH;
    value            = s_table_find(&table, "height");
    prefs.res_height = (value && valid_res) ? atoi(value) : DEFAULT_HEIGHT;

    value            = s_table_find(&table, "master");
    prefs.master_vol = (value) ? atof(value) : 0.8f;

    value         = s_table_find(&table, "sfx");
    prefs.sfx_vol = (value) ? atof(value) : 1.0f;

    value           = s_table_find(&table, "music");
    prefs.music_vol = (value) ? atof(value) : 1.0f;

    asset_free(raw_table);
    s_table_free(&table);
  } else {
    prefs.master_vol = 0.8f;
    prefs.sfx_vol    = 1.0f;
    prefs.music_vol  = 0.75f;
    prefs.res_width  = DEFAULT_WIDTH;
    prefs.res_height = DEFAULT_HEIGHT;
  }

  return prefs;
}

void save_config(void) {
  s_table table = s_table_create(512, 8, 1);

  s_table_add_float(&table, "master", a_listener_get_gain(audio_ctx));
  s_table_add_float(&table, "sfx",
                    a_layer_get_gain(audio_ctx, a_res.sfx_layer));
  float music_layer = a_layer_get_gain(audio_ctx, a_res.music_layer);
  s_table_add_float(&table, "music",
                    a_layer_get_gain(audio_ctx, a_res.music_layer));
  int32_t width, height;
  r_window_get_size(render_ctx, &width, &height);
  s_table_add_int(&table, "width", width);
  s_table_add_int(&table, "height", height);

  s_table_write(&table, USER_PREFS_FILE);

  s_table_free(&table);
}

int main(void) {
  srand(time(0));

  user_prefs = load_config();

  a_ctx_info ctx_info = (a_ctx_info){
      .device      = 0,
      .max_layers  = 2,
      .max_buffers = 32,
      .max_sfx     = 32,
      .max_fx      = 2,
      .max_filters = 2,
      .pcm_size    = 4096 * 4,
  };

  audio_ctx = a_ctx_create(ctx_info);

  if (!audio_ctx) {
    ASTERA_DBG("Unable to initialize audio context, exiting.\n");
    return 1;
  }

  a_listener_set_gain(audio_ctx, user_prefs.master_vol);
  init_audio();

  r_window_params params = (r_window_params){.width     = user_prefs.res_width,
                                             .height    = user_prefs.res_height,
                                             .x         = 0,
                                             .y         = 0,
                                             .resizable = 0,
                                             .fullscreen   = 0,
                                             .vsync        = 1,
                                             .borderless   = 0,
                                             .refresh_rate = 0,
                                             .gamma        = 1.f,
                                             .title        = "Fighter"};

  window_size[0] = params.width;
  window_size[1] = params.height;

  render_ctx = r_ctx_create(params, 4, 32, 128, 16);
  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    ASTERA_DBG("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(16, 32, 16, 1, 32);

  if (!input_ctx) {
    ASTERA_DBG("Input context failed.\n");
    return 1;
  }

  // setup key bindings
  init_input();

  // setup all the rendering resources
  init_render();

  // setup the render context & input context to talk to eachother
  //        (input callback)
  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  // create the internal timers
  update_timer = s_timer_create();
  render_timer = s_timer_create();

  // setup the user interface
  init_ui();

  // setup in game specific resources
  init_game();

  // setup the collision for everything
  init_collision();

  // while the window isn't requested closed and the game state is still
  // running, loop over everything!
  while (!r_window_should_close(render_ctx) && game_state != GAME_QUIT) {
    time_s delta = s_timer_update(&update_timer);

    i_ctx_update(input_ctx);

    input(delta);

    update(delta);

    if (a_can_play(audio_ctx)) {
      a_ctx_update(audio_ctx);
    }

    if (r_can_render(render_ctx)) {
      time_s render_delta = s_timer_update(&render_timer);
      render(render_delta);

      r_framebuffer_unbind();
      r_window_clear_color("#0a0a0a");
      r_window_clear();

      r_shader_bind(fbo_shader);
      r_set_uniformf(fbo_shader, "use_vig", 1.f);
      r_set_v2(fbo_shader, "resolution", window_size);
      r_set_uniformf(fbo_shader, "vig_intensity", 25.f);
      r_set_uniformf(fbo_shader, "vig_scale", 0.25f);

      r_framebuffer_draw(render_ctx, fbo);

      r_shader_bind(fbo_shader);
      r_set_uniformf(fbo_shader, "use_vig", 0.f);
      r_framebuffer_draw(render_ctx, ui_fbo);

      r_window_swap_buffers(render_ctx);
    }
  }

  save_config();

  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);
  a_ctx_destroy(audio_ctx);
  ui_ctx_destroy(u_ctx);

  return 0;
}

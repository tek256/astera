#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <astera/asset.h>
#include <astera/audio.h>
#include <astera/col.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/input.h>
#include <astera/ui.h>

#define BAKED_SHEET_SIZE  16 * 16
#define BAKED_SHEET_WIDTH 16

#define MAX_ENEMIES 32

#define MAX_PLAYER_HEALTH 10
#define DAMAGE_DURATION   750

// ui stuff

typedef enum {
  MENU_NONE     = 0,
  MENU_MAIN     = 1,
  MENU_SETTINGS = 2,
  MENU_PAUSE    = 3,
} MENU_PAGES;

typedef struct {
  // --- MAIN ---
  ui_img    logo_img;
  ui_text   main_title;
  ui_button play, settings, quit;

  // --- SETTINGS ---
  ui_text   master_label, sfx_label, music_label, settings_title;
  ui_slider master_vol, sfx_vol, music_vol;

  ui_text     res_label;
  ui_dropdown res_dd;

  ui_button back_button;

  // --- PAUSE ---
  ui_text   p_title;
  ui_box    p_bg;
  ui_button p_resume, p_settings, p_quit;

  // --- PAGES ---
  ui_tree  main_page, settings_page, pause_page;
  ui_tree* current_page;
  int      page_number, last_page;

  // --- COLORS ---
  ui_color red, white, black, grey, clear, offwhite, offblack;

  // --- OTHER ---
  ui_font  font;
  asset_t *font_data, *logo_data;
  float    scroll_timer, scroll_duration;
} menu_t;

// audio data
typedef struct {
  int sfx_layer, music_layer;
  int s_attack, s_click, s_back, s_hit, s_die;
} a_resources;

// game data

typedef enum {
  ENEMY_IDLE = 0,
  ENEMY_WALK,
  ENEMY_HIT,
  ENEMY_ATTACK,
  ENEMY_DIE,
  ENEMY_DEAD,
} ENEMY_STATE;

typedef enum {
  ENEMY_EYE    = 0,
  ENEMY_SLIME  = 1,
  ENEMY_GOBLIN = 2,
} ENEMY_TYPES;

typedef struct {
  r_sprite sprite;
  c_circle circle;
  int      health, max_health;
  int      state, state_change;
  int      type, take_damage;
  float    damage_timer;
} enemy_t;

typedef struct {
  vec2     center;
  c_aabb   aabb, hitbox;
  int      health, is_idle, take_damage;
  float    damage_timer, damage_duration;
  r_sprite sprite, sword, swoosh;
} player_t;

typedef struct {
  enemy_t* enemies;
  int      enemy_count, enemy_capacity;

  c_aabb* env;
  int     env_cols;

  player_t player;
} level_t;

static menu_t      menu  = {0};
static a_resources a_res = {0};
static level_t     level = {0};

static vec4 color_red;
static vec4 color_white;

vec2 window_size;

r_shader      shader, baked, fbo_shader, ui_shader, single;
r_baked_sheet baked_sheet;
r_sprite      sprite;
r_sheet       sheet, character_sheet;
r_ctx*        render_ctx;
i_ctx*        input_ctx;
ui_ctx*       u_ctx;
a_ctx*        audio_ctx;

GLFWvidmode* vidmodes;
uint8_t      vidmode_count;

// MENU LAYOUT
// PLAY
// SETTINGS
// QUIT

typedef enum {
  GAME_START = 0,
  GAME_PLAY  = 1,
  GAME_PAUSE = 2,
  GAME_QUIT  = -1,
} GAME_STATE;

static int game_state = GAME_START;

r_framebuffer fbo, ui_fbo;

r_sprite* sprites;

s_timer render_timer;
s_timer update_timer;

int rnd_range(int min, int max) {
  int value = rand() % (max - min);
  return value + min;
}

int rnd_rangef(float min, float max) {
  return min + (((float)rand() / (float)(RAND_MAX)) * (max - min));
}

// check if other point is within radius of point
int within_range_of(vec2 point, float radius, vec2 other_point) {
  vec2 norm = {0.f};
  vec2_sub(norm, point, other_point);
  float dot = vec2_dot(norm, norm);
  return dot < radius * radius;
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

  i_binding_add(input_ctx, "attack", KEY_F, ASTERA_BINDING_KEY);
  i_binding_add_alt(input_ctx, "attack", KEY_SPACE, ASTERA_BINDING_KEY);
}

// set the menu page based on enum
static void menu_set_page(int page) {
  // invalid page type
  if (!(page == MENU_NONE || page == MENU_SETTINGS || page == MENU_PAUSE ||
        page == MENU_MAIN)) {
    printf("Invalid page type.\n");
    return;
  }

  menu.last_page   = menu.page_number;
  menu.page_number = page;

  switch (menu.page_number) {
    case MENU_NONE: {
    } break;
    case MENU_MAIN: {
      ui_tree_reset(&menu.main_page);
    } break;
    case MENU_SETTINGS: {
      ui_tree_reset(&menu.settings_page);
    } break;
    case MENU_PAUSE: {
      ui_tree_reset(&menu.pause_page);
    } break;
    default:
      break;
  }

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
    default:
      break;
  }
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

  menu.font_data = asset_get("resources/fonts/monogram.ttf");
  menu.font      = ui_font_create(u_ctx, menu.font_data->data,
                             menu.font_data->data_length, "monogram");

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
  ui_attrib_setc(u_ctx, UI_BUTTON_BG_HOVER, menu.black);
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

  // -- MAIN MENU --
  menu.main_title =
      ui_text_create(u_ctx, temp, "FIGHTER", 48.f, menu.font, UI_ALIGN_CENTER);

  /* vec4 text_bounds;
  ui_text_bounds(u_ctx, &menu.main_title, text_bounds); */

  // load in the logo data
  menu.logo_data = asset_get("resources/textures/icon.png");

  // square image based on height
  /* vec2 temp3 = {0.1f, 0.1f};
  ui_scale_to_px(u_ctx, temp3, temp3);
  temp3[0] = temp3[1];
  ui_px_to_scale(u_ctx, temp3, temp3);
  temp[0] = text_bounds[0] - temp2[0];
  menu.logo_img =
      ui_img_create(u_ctx, menu.logo_data->data, menu.logo_data->data_length,
                    IMG_NEAREST | IMG_REPEATX | IMG_REPEATY, temp, temp3); */

  ui_element tmp_ele;

  temp[0] = 0.5f;
  temp[1] += 0.25f; // 0.5f, 0.45f;
  menu.play =
      ui_button_create(u_ctx, temp, temp2, "PLAY", UI_ALIGN_CENTER, 32.f);
  tmp_ele = ui_element_get(&menu.play, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  temp[1] += 0.15f; // 0.5f, 0.6f;
  menu.settings =
      ui_button_create(u_ctx, temp, temp2, "SETTINGS", UI_ALIGN_CENTER, 32.f);
  tmp_ele = ui_element_get(&menu.settings, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  temp[1] += 0.15f; // 0.5f, 0.75f
  menu.quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 32.f);
  tmp_ele = ui_element_get(&menu.quit, UI_BUTTON);
  ui_element_center_to(tmp_ele, temp);

  menu.main_page      = ui_tree_create(8);
  menu.main_page.loop = 1;
  ui_tree_add(u_ctx, &menu.main_page, &menu.main_title, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.main_page, &menu.logo_img, UI_IMG, 0, 0, 0);
  ui_tree_add(u_ctx, &menu.main_page, &menu.play, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.main_page, &menu.settings, UI_BUTTON, 1, 1, 1);
  ui_tree_add(u_ctx, &menu.main_page, &menu.quit, UI_BUTTON, 1, 1, 1);

  // SETTINGS MENU
  vec2_clear(temp2);

  temp[0] = 0.5f;
  temp[1] = 0.125f;
  menu.settings_title =
      ui_text_create(u_ctx, temp, "SETTINGS", 32.f, menu.font, UI_ALIGN_CENTER);

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
  menu.master_vol =
      ui_slider_create(u_ctx, temp, temp2, temp3, 1, 0.8f, 0.f, 1.f, 20);

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

  menu.music_vol =
      ui_slider_create(u_ctx, temp, temp2, temp3, 1, 1.f, 0.f, 1.f, 20);

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
  menu.sfx_vol =
      ui_slider_create(u_ctx, temp, temp2, temp3, 1, 1.f, 0.f, 1.f, 20);

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

    int32_t cur_dist = (DEFAULT_WIDTH - cursor->width + DEFAULT_HEIGHT -
                        cursor->height - cursor->refreshRate);

    if (cur_dist < distance) {
      closest  = i;
      distance = cur_dist;
    }
  }

  // dropdown scrolling
  menu.scroll_timer    = 0.f;
  menu.scroll_duration = 1000.f;

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

  menu.settings_page.loop = 0;

  // --- PAUSE MENU ---

  vec2_clear(temp2);
  temp[0] = 0.5f;
  temp[1] = 0.25f;
  menu.p_title =
      ui_text_create(u_ctx, temp, "SETTINGS", 48.f, menu.font, UI_ALIGN_CENTER);

  vec2 zero         = {0.f, 0.f};
  menu.p_bg         = ui_box_create(u_ctx, zero, zero);
  menu.p_bg.size[0] = 1.f;
  menu.p_bg.size[1] = 1.f;
  // ui_box_set_colors(&menu.p_bg, menu.red, 0, 0, 0);
  ui_get_color(menu.p_bg.bg, "000");
  menu.p_bg.bg[3] = 0.2f;

  temp[1] += 0.2f;
  menu.p_resume =
      ui_button_create(u_ctx, temp, temp2, "RESUME", UI_ALIGN_CENTER, 32.f);

  temp[1] += 0.15f;
  menu.p_settings =
      ui_button_create(u_ctx, temp, temp2, "SETTINGS", UI_ALIGN_CENTER, 32.f);

  temp[1] += 0.15f;
  menu.p_quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 32.f);

  menu.pause_page      = ui_tree_create(8);
  menu.pause_page.loop = 1;
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_title, UI_TEXT, 0, 0, 1);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_resume, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_settings, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_quit, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_bg, UI_BOX, 0, 0, 0);

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

void init_audio(a_ctx* ctx) {
  a_res.sfx_layer   = a_layer_create(ctx, "sfx", 16, 0);
  a_res.music_layer = a_layer_create(ctx, "music", 0, 2);

  a_res.s_attack = load_sfx("resources/audio/attack.wav", "attack");
  a_res.s_click  = load_sfx("resources/audio/click.wav", "click");
  a_res.s_back   = load_sfx("resources/audio/back.wav", "back");
  a_res.s_hit    = load_sfx("resources/audio/hit.wav", "hit");
  a_res.s_die    = load_sfx("resources/audio/die.wav", "die");
  // int s_attack, s_click, s_back, s_hit, s_die;
}

void init_collision(void) {
  for (uint16_t i = 0; i < level.enemy_count; ++i) {}

  vec2 level_min = {0.f, 0.f};
  vec2 level_max = {320.f, 180.f};
  vec2 wall_size = {16.f, 16.f};

  level.env_cols = 4;
  level.env      = (c_aabb*)calloc(sizeof(c_aabb), 4);

  // left
  vec2 left_size = {wall_size[0], level_max[1]};
  vec2 left_pos  = {-(wall_size[0] * 0.5f), level_max[1] * 0.5f};
  level.env[0]   = c_aabb_create(left_pos, left_size);

  // right
  vec2 right_pos = {320.f + (wall_size[0] * 0.5f), left_pos[1]};
  level.env[1]   = c_aabb_create(right_pos, left_size);

  // top
  vec2 top_pos  = {level_max[0] * 0.5f, -(wall_size[1] * 0.5f)};
  vec2 top_size = {level_max[0], wall_size[1]};
  level.env[2]  = c_aabb_create(top_pos, top_size);

  // bottom
  vec2 bottom_pos = {top_pos[0], level_max[1] + (wall_size[1] * 0.75f)};
  level.env[3]    = c_aabb_create(bottom_pos, top_size);
}

void init_render(r_ctx* ctx) {
  shader = load_shader("resources/shaders/instanced.vert",
                       "resources/shaders/instanced.frag");

  single = load_shader("resources/shaders/single.vert",
                       "resources/shaders/single.frag");

  baked = load_shader("resources/shaders/simple.vert",
                      "resources/shaders/simple.frag");

  fbo_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  ui_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  fbo    = r_framebuffer_create(DEFAULT_WIDTH, DEFAULT_HEIGHT, fbo_shader, 0);
  ui_fbo = r_framebuffer_create(DEFAULT_WIDTH, DEFAULT_HEIGHT, ui_shader, 0);

  sheet           = load_sheet("resources/textures/tilemap.png", 16, 16);
  character_sheet = load_sheet("resources/textures/spritesheet.png", 16, 16);

  int sheet_width  = (320 / 16) + 1;
  int sheet_height = (180 / 16) + 1;

  r_get_color4f(color_red, "#f00");
  r_get_color4f(color_white, "#fff");

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

    if (x == 0 && y == 0) {
      texture = 0;
    } else if (x >= sheet_width - 1 && y == 0) {
      texture = 0;
      flip_x  = 1;
    } else if (x == 0 && y >= sheet_height - 1) {
      texture = 40;
    } else if (x >= sheet_width - 1 && y >= sheet_height - 1) {
      texture = 40;
      flip_x  = 1;
    } else if (x == 0) {
      texture = 10;
    } else if (x >= sheet_width - 1) {
      texture = 10;
      flip_x  = 1;
    } else if (y == 0) {
      texture = 1;
    } else if (y >= sheet_height - 1) {
      texture = 41;
      // bottom left corner
    } else if (y == sheet_height - 2 && x == 1) {
      texture = 31;
      // top left corner
    } else if (y == 1 && x == 1) {
      texture = 11;
      // top right corner
    } else if (x == sheet_width - 2 && y == 1) {
      texture = 14;
      // bottom right corner
    } else if (x == sheet_width - 2 && y == sheet_height - 2) {
      texture = 34;
      // right edge
    } else if (x == sheet_width - 2 && !(y == 1 || y == sheet_height - 2)) {
      texture = 24;
      flip_y  = rand() % 2;
      // left edge
    } else if (x == 1 && !(y == 1 || y == sheet_height - 2)) {
      texture = 21;
      flip_y  = rand() % 2;
      // top edge
    } else if (y == 1 && !(x == 1 || x == sheet_width - 2)) {
      texture = 13;
      flip_x  = rand() % 2;
      // bottom edge
    } else if (y == sheet_height - 1 && !(x == 1 || x == sheet_width - 2)) {
      texture = 32;
      flip_x  = rand() % 2;
    }

    // left side torches
    if (x == 1 && !(y == 0 || y >= sheet_height - 1) && y % 2 == 1) {
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
               y % 2 == 1) {
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
    } else if (y == 0 && !(x == 0 || x >= sheet_width - 1) && x % 2 == 1) {
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
      load_anim(&character_sheet, "goblin_idle", goblin_idle_frames, 6, 16, 1);

  int     goblin_run_frames[] = {14, 15, 16, 17, 18, 19};
  r_anim* goblin_run =
      load_anim(&character_sheet, "goblin_run", goblin_run_frames, 6, 19, 1);

  int     slime_idle_frames[] = {21, 22, 23, 24, 25, 26};
  r_anim* slime_idle =
      load_anim(&character_sheet, "slime_idle", slime_idle_frames, 6, 16, 1);

  int     slime_bounce_frames[] = {28, 29, 30, 31, 32, 33};
  r_anim* slime_bounce          = load_anim(&character_sheet, "slime_bounce",
                                   slime_bounce_frames, 6, 16, 1);

  int     player_idle_frames[] = {35, 36, 37, 38, 39, 40};
  r_anim* player_idle =
      load_anim(&character_sheet, "player_idle", player_idle_frames, 6, 12, 1);
  if (!player_idle) {
    printf("No player walk\n");
  }

  int     player_walk_frames[] = {42, 43, 44, 45, 46, 47};
  r_anim* player_walk =
      load_anim(&character_sheet, "player_walk", player_walk_frames, 6, 19, 1);
  if (!player_walk) {
    printf("No player walk\n");
  }

  // 16x9 * 20
  vec2 camera_size = {320, 180};
  r_camera_set_size(r_ctx_get_camera(ctx), camera_size);
}

void init_game(void) {
  int width  = 320 / 16.f;
  int height = 180 / 16.f;

  // initialize player stuff _before_ enemies so they're not within range of
  // spawn
  vec2   player_pos      = {152.f, 82.f};
  vec2   player_halfsize = {8.f, 8.f};
  c_aabb player_col      = c_aabb_create(player_pos, player_halfsize);
  vec2   sword_pos       = {player_pos[0] + 12.f, player_pos[1] + 4.f};
  vec2   sword_halfsize  = {10.f, 4.f};
  c_aabb sword_col       = c_aabb_create(sword_pos, sword_halfsize);
  vec2   zero = {0.f, 0.f}, halfsize = {8.f, 8.f};
  vec2   sprite_size = {16.f, 16.f};

  level.player = (player_t){
      .health = MAX_PLAYER_HEALTH, .aabb = player_col, .hitbox = sword_col, 0};
  vec2_dup(level.player.center, player_pos);
  level.player.sprite       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.sprite.layer = 4;
  r_sprite_set_tex(&level.player.sprite, &character_sheet, 42);

  level.player.sword       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.sword.layer = 5;
  r_sprite_set_tex(&level.player.sword, &character_sheet, 13);

  level.player.swoosh       = r_sprite_create(shader, player_pos, sprite_size);
  level.player.swoosh.layer = 6;
  r_anim* sword_swoosh      = r_anim_get_name(render_ctx, "sword_swoosh");
  r_sprite_set_anim(&level.player.swoosh, sword_swoosh);

  level.enemy_count    = rnd_range(16, MAX_ENEMIES);
  level.enemy_capacity = MAX_ENEMIES;
  level.enemies        = (enemy_t*)calloc(sizeof(enemy_t), MAX_ENEMIES);

  // TODO REMOVE (DEBUG)
  r_anim_list_cache(render_ctx);

  for (int i = 0; i < level.enemy_count; ++i) {
    // keep within middle 6/8ths of screen
    int  x        = rnd_range(20, 300);
    int  y        = rnd_range(20, 160);
    vec2 position = {x, y};

    int within_range = 0;
    if ((within_range = within_range_of(level.player.center, 32.f, position))) {
      while (within_range) {
        x            = rnd_range(20, 300);
        y            = rnd_range(20, 160);
        position[0]  = (float)x;
        position[1]  = (float)y;
        within_range = within_range_of(level.player.center, 32.f, position);
      }
    }

    enemy_t* en      = &level.enemies[i];
    en->sprite       = r_sprite_create(shader, position, sprite_size);
    en->sprite.layer = 4;
    en->state        = ENEMY_IDLE;
    en->circle       = c_circle_create(position, 7.f);
    en->state_change = 0;
    en->type         = rand() % 3;

    r_anim* anim = 0;

    switch (en->type) {
      case ENEMY_EYE:
        en->max_health = 2;
        anim           = r_anim_get_name(render_ctx, "eye_idle");
        break;
      case ENEMY_GOBLIN:
        en->max_health = 4;
        anim           = r_anim_get_name(render_ctx, "goblin_idle");
        break;
      case ENEMY_SLIME:
        en->max_health = 3;
        anim           = r_anim_get_name(render_ctx, "slime_idle");
        break;
    }
    en->health = en->max_health;
    r_sprite_set_anim(&en->sprite, anim);
    r_sprite_anim_play(&en->sprite);
  }

  // initialize player data

  // (320 / 2) - (16 / 2), (180 / 2) - (16 / 2)
  // centered position
}

void game_resized_to(vec2 size) {
  r_set_can_render(render_ctx, 0);

  ui_ctx_resize(u_ctx, size);
  r_framebuffer_destroy(fbo);
  r_framebuffer_destroy(ui_fbo);
  fbo            = r_framebuffer_create(size[0], size[1], fbo_shader, 0);
  ui_fbo         = r_framebuffer_create(size[0], size[1], ui_shader, 0);
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
      if (ui_tree_check_event(menu.current_page, menu.play.id) == 1) {
        game_state = GAME_PLAY;
        menu_set_page(MENU_NONE);
        return;
      }

      if (ui_tree_check_event(menu.current_page, menu.settings.id) == 1) {
        menu_set_page(MENU_SETTINGS);
      }

      if (ui_tree_check_event(menu.current_page, menu.quit.id) == 1) {
        // quit game
        menu_set_page(MENU_NONE);
        game_state = GAME_QUIT;
        return;
      }
    } break;
    case MENU_SETTINGS: { //
      if (i_binding_clicked(input_ctx, "right")) {
        if (ui_tree_is_active(u_ctx, menu.current_page, menu.master_vol.id)) {
          ui_slider_next_step(&menu.master_vol);
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.sfx_vol.id)) {
          ui_slider_next_step(&menu.sfx_vol);
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.music_vol.id)) {
          ui_slider_next_step(&menu.music_vol);
        }
      }

      if (i_binding_clicked(input_ctx, "left")) {
        if (ui_tree_is_active(u_ctx, menu.current_page, menu.master_vol.id)) {
          ui_slider_prev_step(&menu.master_vol);
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.sfx_vol.id)) {
          ui_slider_prev_step(&menu.sfx_vol);
        } else if (ui_tree_is_active(u_ctx, menu.current_page,
                                     menu.music_vol.id)) {
          ui_slider_prev_step(&menu.music_vol);
        }
      }

      if (ui_tree_check_event(menu.current_page, menu.back_button.id) == 1) {
        menu_set_page(menu.last_page);
      }

      if (menu.master_vol.holding) {
        //
        a_listener_set_gain(audio_ctx, menu.master_vol.value);
      }

      if (menu.sfx_vol.holding) {
        a_layer_set_gain(audio_ctx, a_res.sfx_layer, menu.sfx_vol.value);
      }
      if (menu.music_vol.holding) {
        a_layer_set_gain(audio_ctx, a_res.music_layer, menu.music_vol.value);
      }

      int32_t event_type = 0;
      if ((event_type =
               ui_tree_check_event(menu.current_page, menu.res_dd.id)) == 1) {
        if (menu.res_dd.showing) {
          menu.res_dd.showing = 0;
        } else {
          menu.res_dd.showing = 1;
        }

        if (ui_dropdown_has_change(&menu.res_dd) && !menu.res_dd.showing) {
          printf("%i\n", menu.res_dd.selected);
          GLFWvidmode* modes = (GLFWvidmode*)menu.res_dd.data;
          r_select_vidmode(render_ctx, modes[menu.res_dd.selected], 0, 1, 0);
          vec2 window_size;
          r_window_get_vsize(render_ctx, window_size);
          game_resized_to(window_size);
        }
      }

    } break;
    case MENU_PAUSE: { //
      if (ui_tree_check_event(menu.current_page, menu.p_settings.id) == 1) {
        menu_set_page(MENU_SETTINGS);
      }

      if (ui_tree_check_event(menu.current_page, menu.p_resume.id) == 1) {
        menu_set_page(MENU_NONE);
        game_state = GAME_PLAY;
      }

      if (ui_tree_check_event(menu.current_page, menu.p_quit.id) == 1) {
        // clear old framebuffer
        r_framebuffer_bind(fbo);
        r_window_clear_color_empty();
        r_window_clear();
        r_framebuffer_unbind();
        menu_set_page(MENU_MAIN);
        game_state = GAME_START;
        return;
      }
    } break;
  }
}

void input(float delta) {
  vec2 mouse_pos = {i_mouse_get_x(input_ctx), i_mouse_get_y(input_ctx)};
  ui_ctx_update(u_ctx, mouse_pos);

  // printf("PAGE NUMBER: %i\n", menu.page_number);
  // printf("PAGE ADDR: %p\n", menu.current_page);
  if (menu.current_page) {
    int32_t active = ui_tree_check(u_ctx, menu.current_page);

    if (i_mouse_clicked(input_ctx, MOUSE_LEFT)) {
      ui_tree_select(u_ctx, menu.current_page, 1, 1);
    }

    if (i_mouse_released(input_ctx, MOUSE_LEFT)) {
      ui_tree_select(u_ctx, menu.current_page, 0, 1);
    }

    if (i_key_clicked(input_ctx, KEY_ESCAPE)) {
      switch (menu.page_number) {
        case MENU_NONE: {
          // do nothing, this case shouldn't be achievable
          printf("How'd you get here?!\n");
        } break;
        case MENU_MAIN: {
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
      }
    }

    if (i_binding_clicked(input_ctx, "down")) {
      ui_tree_next(menu.current_page);
    }

    if (i_binding_clicked(input_ctx, "up")) {
      ui_tree_prev(menu.current_page);
    }

    if (i_binding_clicked(input_ctx, "select")) {
      ui_tree_select(u_ctx, menu.current_page, 1, 0);
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
    if (i_key_clicked(input_ctx, KEY_ESCAPE)) {
      if (game_state == GAME_PLAY) {
        game_state        = GAME_PAUSE;
        menu.current_page = &menu.pause_page;
        menu.page_number  = MENU_PAUSE;
      }
    }

    // Handle User Input normally now
    vec2 move = {0.f};
    if (i_binding_down(input_ctx, "up")) {
      move[1] = -1.f;
    } else if (i_binding_down(input_ctx, "down")) {
      move[1] = 1.f;
    }

    if (i_binding_clicked(input_ctx, "attack")) {
      r_sprite_anim_play(&level.player.swoosh);
    }

    // it's a bit like godmode, but less interesting
    if (i_key_clicked(input_ctx, 'E')) {
      for (int i = 0; i < level.enemy_count; ++i) {
        enemy_t* en      = &level.enemies[i];
        en->take_damage  = 1;
        en->damage_timer = DAMAGE_DURATION;
      }
    }

    if (i_binding_down(input_ctx, "left")) {
      move[0] = -1.f;
    } else if (i_binding_down(input_ctx, "right")) {
      move[0] = 1.f;
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
    vec2 center = {0.f};

    vec2 hitbox_pos = {
        level.player.center[0] + (level.player.sprite.flip_x) ? -20.f : 12.f,
        level.player.center[1] + 4.f};
    vec2 hitbox_halfsize = {10.f, 4.f};
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
}

// weight (0 = 100% a, 1 = 100% b, .50 = 50 a 50 b)
static void blend(vec4 dst, vec4 a, vec4 b, float weight) {
  dst[0] = (a[0] * (1.f - weight)) + (b[0] * weight);
  dst[1] = (a[1] * (1.f - weight)) + (b[1] * weight);
  dst[2] = (a[2] * (1.f - weight)) + (b[2] * weight);
}

void update(time_s delta) {
  if (level.player.take_damage) {
    level.player.damage_timer -= delta;
    if (level.player.damage_timer <= 0.f) {
      level.player.take_damage = 0;
    }

    blend(level.player.sprite.color, color_white, color_red,
          level.player.damage_timer / DAMAGE_DURATION);
    blend(level.player.sword.color, color_white, color_red,
          level.player.damage_timer / DAMAGE_DURATION);
  }

  int swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);
  if (swoosh_state) {
    // test sword vs enemies
    for (int i = 0; i < level.enemy_count; ++i) {
      enemy_t* en = &level.enemies[i];

      // only allow damage after a 3rd of the damage timer has elapsed
      if (!en->damage_timer > (DAMAGE_DURATION * 0.33f)) {
        c_manifold man = c_aabb_vs_circle_man(level.player.hitbox, en->circle);
        if (man.distance != 0.f) {
          // apply opposite directional force to enemy
          en->take_damage  = 1;
          en->damage_timer = DAMAGE_DURATION;
          en->health--;

          if (en->health <= 0) {
            // death animation/etc
          }
        }
      }
    }
  }

  // test player vs enemies
  for (int i = 0; i < level.enemy_count; ++i) {
    enemy_t* en = &level.enemies[i];

    // animate damaged enemies
    if (en->take_damage) {
      en->damage_timer -= delta;
      blend(en->sprite.color, color_white, color_red,
            en->damage_timer / DAMAGE_DURATION);

      if (en->damage_timer <= 0.f) {
        en->take_damage = 0;
      }
    }

    c_manifold man = c_aabb_vs_circle_man(level.player.aabb, en->circle);
    if (man.distance != 0.f) {
      // take damage I guess :shrug:
      if (!level.player.take_damage) {
        level.player.take_damage  = 1;
        level.player.damage_timer = DAMAGE_DURATION;
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
}

void draw_ui(void) {
  r_framebuffer_bind(ui_fbo);
  r_window_clear_color_empty();
  r_window_clear();

  ui_frame_start(u_ctx);
  ui_tree_draw(u_ctx, menu.current_page);
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
  r_sprite_update(&level.player.sprite, delta);
  r_sprite_draw_batch(render_ctx, &level.player.sprite);

  // draw the sword
  r_sprite_update(&level.player.sword, delta);
  r_sprite_draw_batch(render_ctx, &level.player.sword);

  // draw (or don't) the swoosh
  r_sprite_update(&level.player.swoosh, delta);
  int swoosh_state = r_sprite_get_anim_state(&level.player.swoosh);
  if (swoosh_state == R_ANIM_STOP) {
    level.player.swoosh.visible = 0;
  } else {
    level.player.swoosh.visible = 1;
  }

  r_sprite_draw_batch(render_ctx, &level.player.swoosh);

  // draw the enemies
  for (int i = 0; i < level.enemy_count; ++i) {
    r_sprite* sprite = &level.enemies[i].sprite;
    r_sprite_update(&level.enemies[i].sprite, delta);
    r_sprite_draw_batch(render_ctx, &level.enemies[i].sprite);
  }

  // issue a draw command for anything left in the batches
  r_ctx_draw(render_ctx);
}

void render(time_s delta) {
  if (game_state != GAME_START && game_state != GAME_QUIT) {
    // draw sprites
    draw_game(delta);
  }

  static int page_notif         = 0;
  static int page_notif_counter = 0;
  static int ui_change          = 0;
  if (menu.current_page) {
    if (ui_change) {
      ui_change = 0;
    }

    if (page_notif) {
      page_notif = 0;
      ++page_notif_counter;
    }

    // draw ui
    draw_ui();
  } else {
    // check for changes so the framebuffer is cleared. (technically don't
    // have to draw it if we don't have an actual target, but just gluing??
    // this together.
    if (!ui_change) {
      r_framebuffer_bind(ui_fbo);
      r_window_clear_color_empty();
      r_window_clear();

      ui_change = 1;
    }

    if (!page_notif && !(game_state == GAME_PLAY || game_state == GAME_PAUSE)) {
      printf("No current page: %i\n", page_notif_counter);
      page_notif = 1;
    }
  }
}

int main(void) {
  srand(time(0));
  audio_ctx = a_ctx_create(0, 2, 16, 16, 2, 2, 2, 4096 * 4);

  if (!audio_ctx) {
    printf("Unable to initialize audio context, exiting.\n");
    return 1;
  }

  a_listener_set_gain(audio_ctx, 0.8f);
  init_audio(audio_ctx);

  r_window_params params = (r_window_params){.width        = DEFAULT_WIDTH,
                                             .height       = DEFAULT_HEIGHT,
                                             .x            = 0,
                                             .y            = 0,
                                             .resizable    = 0,
                                             .fullscreen   = 0,
                                             .vsync        = 1,
                                             .borderless   = 0,
                                             .refresh_rate = 0,
                                             .gamma        = 1.f,
                                             .title        = "Fighter"};

  window_size[0] = params.width;
  window_size[1] = params.height;

  render_ctx = r_ctx_create(params, 4, 128, 128, 16);
  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    printf("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(16, 32, 16, 1, 32);

  if (!input_ctx) {
    printf("Input context failed.\n");
    return 1;
  }

  init_input();

  init_render(render_ctx);
  r_check_error_loc("Post init");

  asset_t* icon = asset_get("resources/textures/icon.png");
  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  asset_free(icon);

  // Setup the render context & input context to talk to eachother
  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  update_timer = s_timer_create();
  render_timer = s_timer_create();

  init_ui();

  // game -> collision so we have the enemies we need to update
  init_game();

  init_collision();

  while (!r_window_should_close(render_ctx) && game_state != GAME_QUIT) {
    time_s delta = s_timer_update(&update_timer);

    i_ctx_update(input_ctx);

    input(delta);

    update(delta);

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

  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);
  a_ctx_destroy(audio_ctx);

  return 0;
}

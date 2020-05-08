// NOTE: This is being rewritten at the moment

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <astera/linmath.h>
#include <astera/col.h>
#include <astera/ui.h>

typedef struct {
  vec2   position, size;
  c_aabb aabb;

  int lives;
} player_t;

typedef struct {
  int    power;
  c_circ circ;
} bullet_t;

typedef struct {
  enemy_t* enemies;
  int      enemy_count;

  bullet_t* bullets;
  int       bullet_cap;

  player_t player;
} world_t;

typedef enum {
  GAME_MENU  = 0,
  GAME_PLAY  = 1,
  GAME_PAUSE = 2,
} game_state_t;

typedef struct {
  ui_ctx* ctx;

  ui_text   title;
  ui_button start_button;
  ui_button quit_button;
  ui_tree   tree;
  ui_font   font;

  int start_button_id;
  int quit_button_id;

  vec4 white, offwhite;
  vec4 black, gray, grey;
  vec4 red;
} game_ui_t;

typedef enum {
  GAME_MENU  = 0,
  GAME_PLAY  = 1,
  GAME_PAUSE = 2,
} game_state_t;

game_state_t game_state;
game_ui_t    game_ui;

void generate_board() {}

void game_reset() {}

void update(time_s delta) {}

void init() {
  r_window_info window_info;
  window_info.width        = 1280;
  window_info.height       = 720;
  window_info.refresh_rate = 60;
  window_info.resizable    = 1;
  window_info.max_width    = 1920;
  window_info.max_height   = 1080;
  window_info.min_width    = 720;
  window_info.min_height   = 480;
  window_info.vsync        = 1;
  window_info.fullscreen   = 0;
  window_info.borderless   = 0;
  window_info.title        = "Basic Game Example";

  game_ui = (game_ui_t){0};

  vec2 window_size = {window_info.width, window_info.height};
  game_ui.u_ctx    = ui_ctx_create(window_size, 1.f, 1, 1);

  asset_t* font_data = asset_get("resources/fonts/monogram.ttf");
  game_ui.font =
      ui_font_create(u_ctx, "monogram", font_data->data, font_data->data_len);

  vec2 text_pos game_ui.title =
      ui_text_create(u_ctx, text_pos, "ASTERA ENDLESS SHOOTER", 32.f,
                     game_ui.font, UI_ALIGN_LEFT);

  game_ui.start_button =
      ui_button_create(game_ui.ctx, button_pos, button_size, "START",
                       UI_ALIGN_MIDDLE | UI_ALIGN_CENTER, 16.f);

  game_ui.quit_button =
      ui_button_create(game_ui.ctx, button_pos, button_size, "QUIT",
                       UI_ALIGN_MIDDLE | UI_ALIGN_CENTER, 16.f);

  game_ui.start_button_id =
      ui_tree_add(game_ui.tree, game_ui.start_button, 1, 1);

  game_ui.quit_button_id = ui_tree_add(game_ui.tree, game_ui.quit_button, 1, 1);

  ui_text_set_colors();

  game_state = GAME_MENU;
}

void render(time_s delta) {
  if (state == GAME_MENU) {
  } else {
    if (game != GAME_PAUSE) {
    } else {
    }
  }
}

int main(void) {
  printf("Hello world!\n");
  return EXIT_SUCCESS;
}

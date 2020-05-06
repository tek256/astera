// NOTE: At this time the dropdown implementation is buggy
// TODO: Debug Mouse usage in this example, for some reason it just segfaults
//
// NOTE: THIS IS NOT WORKING AT THIS TIME

#include <stdio.h>
#include <astera/debug.h>

#define ASTERA_DEBUG_OUTPUT
#include <astera/asset.h>
#include <astera/input.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/ui.h>

#include <string.h>
#include <assert.h>

#include "game.h"

typedef struct {
  ui_button start_button;
  int       start_button_id;

  ui_button exit_button;
  int       exit_button_id;

  vec4 black, gray, grey;
  vec4 white, off_white;

  ui_tree tree;
} start_screen_t;

static int running = 0;

ui_font        font;
start_screen_t start_screen;
world_t        world;
vec2           window_size;

vec2 zero, one, center;
vec4 pause_color;

r_sheet env_sheet;
// r_sheet anim_sheet;

// TODO Debug Mouse Hover segfault ui.c:1582
void init_ui() {
  start_screen = (start_screen_t){0};
  ui_init(window_size, 1.f, 1);

  // test free after allocation
  asset_t* font_asset = asset_get("resources/fonts/monogram.ttf");

  font = ui_font_create(font_asset->data, font_asset->data_length, "monogram");

  // get the colors for the UI
  ui_get_color(start_screen.black, "000");
  ui_get_color(start_screen.gray, "0a0a0a");
  ui_get_color(start_screen.grey, "212121");
  ui_get_color(start_screen.white, "FFF");
  ui_get_color(start_screen.off_white, "EEE");

  vec2 pos  = {0.0f, 0.0f};
  vec2 size = {0.25f, 0.1f};

  start_screen.start_button = ui_button_create(
      pos, size, "START", UI_ALIGN_CENTER | UI_ALIGN_MIDDLE, 16.f);
  start_screen.start_button.font        = font;
  start_screen.start_button.border_size = 3.f;
  ui_button_set_border_radius(&start_screen.start_button, 5.f);

  start_screen.exit_button = ui_button_create(
      pos, size, "QUIT", UI_ALIGN_CENTER | UI_ALIGN_MIDDLE, 16.f);
  start_screen.exit_button.font        = font;
  start_screen.exit_button.border_size = 3.f;
  ui_button_set_border_radius(&start_screen.exit_button, 5.f);

  // set the button's colors
  ui_button_set_colors(&start_screen.start_button, start_screen.gray,
                       start_screen.grey, start_screen.off_white,
                       start_screen.white, start_screen.black,
                       start_screen.black);

  ui_button_set_colors(&start_screen.exit_button, start_screen.gray,
                       start_screen.grey, start_screen.off_white,
                       start_screen.white, start_screen.black,
                       start_screen.black);

  // center the buttons
  ui_element start_ele = (ui_element){&start_screen.start_button, UI_BUTTON};
  ui_element end_ele   = (ui_element){&start_screen.exit_button, UI_BUTTON};

  vec2 button_center = {0.5f, 0.55f};

  ui_element_center_to(start_ele, button_center);

  button_center[1] = 0.65f;
  ui_element_center_to(end_ele, button_center);

  start_screen.tree      = ui_tree_create(4);
  start_screen.tree.loop = 1;

  start_screen.start_button_id = ui_tree_add(
      &start_screen.tree, &start_screen.start_button, UI_BUTTON, 1, 1);
  start_screen.exit_button_id = ui_tree_add(
      &start_screen.tree, &start_screen.exit_button, UI_BUTTON, 0, 1);

  zero[0] = 0.f;
  zero[1] = 0.f;

  center[0] = 0.5f;
  center[1] = 0.5f;

  one[0] = 1.f;
  one[1] = 1.f;

  ui_get_color(pause_color, "0A0A0A");
  pause_color[3] = 0.5f;
}

void init() {
  r_window_info window_info;
  window_info.width      = 1280;
  window_info.height     = 720;
  window_info.resizable  = 1;
  window_info.max_width  = 1920;
  window_info.max_height = 1080;
  window_info.min_width  = 720;
  window_info.min_height = 480;
  window_info.vsync      = 1;
  window_info.fullscreen = 0;
  window_info.borderless = 0;
  window_info.title      = "Astera 0.01 - Basic Game";

  if (!r_init(window_info)) {
    printf("Error: unable to initialize rendering system.\n");
    return;
  }

  r_window_clear_color("666");

  if (!i_init()) {
    printf("Error: unable to initialize input system.\n");
    return;
  }

  window_size[0] = (float)window_info.width;
  window_size[1] = (float)window_info.height;

  i_create_joy(0);

  init_ui();

  running = 1;
}

void render(time_s delta) {
  r_window_clear();

  if (game_state == GAME_START) {
    ui_frame_start();

    ui_tree* tree = &start_screen.tree;

    ui_tree_draw(tree);

    ui_frame_end();
  } else {
    if (game_state == GAME_PAUSE) {
      ui_frame_start();
      ui_im_box_draw(zero, one, pause_color);
      ui_im_text_draw_aligned(center, 16.f, font,
                              UI_ALIGN_MIDDLE | UI_ALIGN_CENTER, "PAUSED");

      ui_frame_end();
    }
  }

  r_window_swap_buffers();
}

void input(time_s delta) {
  // Update the mouse stuff
  i_update();
  r_poll_events();

  // Mouse Usage
  vec2 mouse_pos = {i_get_mouse_x(), i_get_mouse_y()};
  ui_update(mouse_pos);

  if (game_state == GAME_START) {
    if (i_mouse_clicked(0)) {
      ui_tree_select(&start_screen.tree, 1, 1);
    }

    if (i_key_clicked('D')) {
      ui_tree_select_id(&start_screen.tree, start_screen.start_button_id, 1);
    }

    // Keyboard usage
    if (i_key_clicked('E')) {
      ui_tree_next(&start_screen.tree);
    }

    if (i_key_clicked('Q')) {
      ui_tree_prev(&start_screen.tree);
    }

    if (i_key_clicked(KEY_SPACE)) {
      ui_tree_select(&start_screen.tree, 1, 0);
    }

    // Controller usage
    uint16_t joy_id = i_joy_connected();
    if (joy_id > -1) {
      if (i_joy_button_clicked(XBOX_R1)) {
        ui_tree_next(&start_screen.tree);
      }

      if (i_joy_button_clicked(XBOX_L1)) {
        ui_tree_prev(&start_screen.tree);
      }

      if (i_joy_button_clicked(XBOX_A)) {
        ui_tree_select(&start_screen.tree, 1, 0);
      }
    }

    int32_t event_type = 0;
    if ((event_type = ui_element_event(&start_screen.tree,
                                       start_screen.start_button_id))) {
      printf("Hello from the start button!\n");
      game_start();
    }

    if ((event_type = ui_element_event(&start_screen.tree,
                                       start_screen.exit_button_id))) {
      printf("Hello from the exit button!\n");
      running = 0;
    }
  } else {
    if (game_state == GAME_PAUSE) {
      if (i_key_clicked(KEY_SPACE) || i_joy_button_clicked(XBOX_A) ||
          i_key_clicked(KEY_ESCAPE)) {
        game_state = GAME_GAME;
      }
    } else {
      // normal input state
      float horiz, vert;

      if (i_key_clicked(KEY_ESCAPE)) {
        game_state = GAME_PAUSE;
      }
    }
  }
}

void update(time_s delta) {
  uint32_t active = ui_tree_check(&start_screen.tree);

  if (game_state == GAME_GAME) {
  } else {
  }

  if (r_window_should_close()) {
    running = 0;
  }
}

int main(void) {
  init();

  time_s frame_time = MS_TO_SEC / 60;

  while (running) {
    input(frame_time);
    render(frame_time);
    update(frame_time);
  }

  return 1;
}

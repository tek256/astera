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

typedef struct {
  r_sprite sprite;
  c_aabb   aabb;
  int      health;
} enemy_t;

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

static menu_t menu = (menu_t){0};

vec2 window_size;

r_shader shader, baked, fbo_shader, ui_shader;
r_sprite sprite;
r_sheet  sheet, character_sheet;
r_ctx*   render_ctx;
i_ctx*   input_ctx;
ui_ctx*  u_ctx;
a_ctx*   audio_ctx;

GLFWvidmode* vidmodes;
uint8_t      vidmode_count;

// MENU LAYOUT
// PLAY
// SETTINGS??? ----- mmmmmmaybe
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

r_shader load_shader(const char* vs, const char* fs) {
  asset_t* vs_data = asset_get(vs);
  asset_t* fs_data = asset_get(fs);

  r_shader shader = r_shader_create(vs_data->data, fs_data->data);

  asset_free(vs_data);
  asset_free(fs_data);

  return shader;
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

  switch (page) {
    case MENU_NONE: {
      printf("Woah-1.\n");
      menu.current_page = 0;
    } break;
    case MENU_MAIN: {
      printf("Woah0.\n");
      menu.current_page = &menu.main_page;
    } break;
    case MENU_SETTINGS: {
      printf("Woah1.\n");
      menu.current_page = &menu.settings_page;
    } break;
    case MENU_PAUSE: {
      printf("Woah2.\n");
      menu.current_page = &menu.pause_page;
    } break;
    default:
      printf("Woah.\n");
      break;
  }
}

void init_ui(void) {
  menu = (menu_t){0};

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
  menu.main_title = ui_text_create(u_ctx, temp, "FIGHTERESWEFSDKAJDSF", 48.f,
                                   menu.font, UI_ALIGN_CENTER);

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

  menu.main_page = ui_tree_create(8);
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

  for (uint8_t i = 0; i < vidmode_count; ++i) {
    uint8_t str_len = r_get_vidmode_str_simple(
        &options_buffer[option_index], 1024 - option_index, vidmodes[i]);
    option_list[i] = &options_buffer[option_index];

    option_index += str_len + 1;
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

  menu.settings_page = ui_tree_create(16);
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
      ui_text_create(u_ctx, temp, "SETTINGS", 32.f, menu.font, UI_ALIGN_CENTER);

  vec2 zero         = {0.f, 0.f};
  menu.p_bg         = ui_box_create(u_ctx, zero, zero);
  menu.p_bg.size[0] = 1.f;
  menu.p_bg.size[1] = 1.f;

  ui_get_color(menu.p_bg.bg, "000");
  menu.p_bg.bg[3] = 0.4f;

  temp[1] += 0.2f;
  menu.p_resume =
      ui_button_create(u_ctx, temp, temp2, "RESUME", UI_ALIGN_CENTER, 16.f);

  temp[1] += 0.15f;
  menu.p_settings =
      ui_button_create(u_ctx, temp, temp2, "SETTINGS", UI_ALIGN_CENTER, 16.f);

  temp[1] += 0.15f;
  menu.p_quit =
      ui_button_create(u_ctx, temp, temp2, "QUIT", UI_ALIGN_CENTER, 16.f);

  menu.pause_page = ui_tree_create(8);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_title, UI_TEXT, 0, 0, 1);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_resume, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_settings, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_quit, UI_BUTTON, 1, 1, 2);
  ui_tree_add(u_ctx, &menu.pause_page, &menu.p_bg, UI_BOX, 0, 0, 0);

  menu_set_page(MENU_MAIN);
}

void init_render(r_ctx* ctx) {
  shader = load_shader("resources/shaders/instanced.vert",
                       "resources/shaders/instanced.frag");
  r_shader_cache(ctx, shader, "main");

  baked = load_shader("resources/shaders/simple.vert",
                      "resources/shaders/simple.frag");

  r_shader_cache(ctx, baked, "baked");

  fbo_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  ui_shader =
      load_shader("resources/shaders/fbo.vert", "resources/shaders/fbo.frag");

  vec2 screen_size = {1280.f, 720.f};

  fbo    = r_framebuffer_create(1280, 720, fbo_shader, 0);
  ui_fbo = r_framebuffer_create(1280, 720, ui_shader, 0);

  asset_t* sheet_data = asset_get("resources/textures/Dungeon_Tileset.png");
  sheet = r_sheet_create_tiled(sheet_data->data, sheet_data->data_length, 16,
                               16, 0, 0);

  asset_free(sheet_data);

  // 16x9 * 20
  vec2 camera_size = {320, 180};
  r_camera_set_size(r_ctx_get_camera(ctx), camera_size);
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
      if (i_key_clicked(input_ctx, 'D') ||
          i_key_clicked(input_ctx, KEY_RIGHT)) { // up vol
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

      if (i_key_clicked(input_ctx, 'A') ||
          i_key_clicked(input_ctx, KEY_LEFT)) { // down vol
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
      }

      if (menu.sfx_vol.holding) {
        //
      }
      if (menu.music_vol.holding) {
        // TODO layer handling
      }

      int32_t event_type = 0;
      /*if (event_type = ui_tree_check_event(menu.current_page, menu.res_dd.id))
      { if (menu.res_dd.showing) { ui_dropdown_set_to_cursor(&menu.res_dd);

          menu.res_dd.showing = 0;
        } else {
          menu.res_dd.showing = 1;
        }
      }*/

    } break;
    case MENU_PAUSE: { //
      if (ui_tree_check_event(menu.current_page, menu.p_settings.id) == 1) {
        menu_set_page(MENU_SETTINGS);
      }

      if (ui_tree_check_event(menu.current_page, menu.p_quit.id) == 1) {
        menu_set_page(MENU_NONE);
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
          menu_set_page(MENU_NONE);
          return;
        } break;
        case MENU_SETTINGS: {
          // go back a page
          menu_set_page(menu.last_page);
        } break;
      }
    }

    if (i_key_clicked(input_ctx, 'S') || i_key_clicked(input_ctx, KEY_DOWN)) {
      ui_tree_next(menu.current_page);
    }

    if (i_key_clicked(input_ctx, 'W') || i_key_clicked(input_ctx, KEY_UP)) {
      ui_tree_prev(menu.current_page);
    }

    if (i_key_clicked(input_ctx, KEY_SPACE)) {
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
    if (i_key_down(input_ctx, 'W') || i_key_down(input_ctx, KEY_UP)) {
      move[1] = 1.f;
    }
    if (i_key_down(input_ctx, 'A') || i_key_down(input_ctx, KEY_LEFT)) {
      move[1] = 1.f;
    }
    if (i_key_down(input_ctx, 'S') || i_key_down(input_ctx, KEY_DOWN)) {
      move[1] = -1.f;
    }
    if (i_key_down(input_ctx, 'D') || i_key_down(input_ctx, KEY_RIGHT)) {
      move[0] = -1.f;
    }

    // move player or something
  }
}

void update(time_s delta) {}

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

  r_ctx_update(render_ctx);

  r_ctx_draw(render_ctx);
}

void render(time_s delta) {
  if (game_state != GAME_START && game_state != GAME_QUIT) {
    // draw sprites
    draw_game(delta);
  }

  static int page_notif         = 0;
  static int page_notif_counter = 0;
  if (menu.current_page) {
    if (page_notif) {
      page_notif = 0;
      ++page_notif_counter;
    }

    // draw ui
    draw_ui();
  } else {
    if (!page_notif) {
      printf("No current page: %i\n", page_notif_counter);
      page_notif = 1;
    }
  }
}

int main(void) {
  r_window_params params = (r_window_params){.width        = 1280,
                                             .height       = 720,
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

  render_ctx = r_ctx_create(params, 3, 128, 128, 4);
  r_window_clear_color("#0A0A0A");

  if (!render_ctx) {
    printf("Render context failed.\n");
    return 1;
  }

  input_ctx = i_ctx_create(16, 32, 4, 1, 32);

  if (!input_ctx) {
    printf("Input context failed.\n");
    return 1;
  }

  init_render(render_ctx);

  asset_t* icon = asset_get("resources/textures/icon.png");
  r_window_set_icon(render_ctx, icon->data, icon->data_length);
  asset_free(icon);

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  update_timer = s_timer_create();
  render_timer = s_timer_create();

  init_ui();

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

      r_framebuffer_draw(render_ctx, fbo);
      r_framebuffer_draw(render_ctx, ui_fbo);
      r_window_swap_buffers(render_ctx);
    }
  }

  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);

  return 0;
}

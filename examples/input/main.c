// NOTE: At this time the dropdown implementation is buggy

#include <stdio.h>
#include <stdlib.h>

//#define ASTERA_DEBUG_OUTPUT
//#include <astera/debug.h>

#include <astera/asset.h>
#include <astera/input.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/ui.h>

#include <string.h>
#include <assert.h>

static int running = 0;

vec2 window_size;

r_ctx* render_ctx;
i_ctx* input_ctx;

ui_ctx*     u_ctx;
ui_text     text, text2;
ui_font     test_font;
ui_button   button;
ui_dropdown dropdown;
ui_tree     tree;
ui_line     line;
ui_img      img;
ui_text     typed, explain, tab_msg;

uint32_t button_uid, dropdown_uid;

float text_time, text_rate;
int   text_count;

int option_a, option_b, option_c, option_d, option_e;

#define STRING_BUFFER_SIZE 128
#define STRING_FRAME_SIZE  16

static char string_buffer[STRING_BUFFER_SIZE];
static int  string_count = 0;

static char frame_chars[STRING_FRAME_SIZE];
static int  frame_chars_count = 0;

static char tab_str[16];

int record_string = 0;

vec4 green, darkgreen;
vec4 red, darkred;

asset_t* font_asset;

void init_ui() {
  u_ctx = ui_ctx_create(window_size, 1.f, 1, 1);
  // ui_init(window_size, 1.f, 1);

  vec4 white, off_white, grey2;
  vec4 black;
  vec4 dark, dark2;
  vec4 gray, grey;

  ui_get_color(white, "FFF");
  ui_get_color(off_white, "EEE");
  ui_get_color(grey2, "777");
  ui_get_color(grey, "444");
  ui_get_color(gray, "2D2D2D");
  ui_get_color(dark, "0A0A0A");
  ui_get_color(dark2, "111");
  ui_get_color(black, "000");
  ui_get_color(green, "0ebd24");
  ui_get_color(darkgreen, "0d7f1b");
  ui_get_color(red, "de0c0c");
  ui_get_color(darkred, "af1111");

  vec2 text_pos  = {0.5f, 0.05f};
  vec2 text_pos2 = {0.5f, 0.065f};
  font_asset     = asset_get("resources/fonts/monogram.ttf");

  test_font = ui_font_create(u_ctx, font_asset->data, font_asset->data_length,
                             "monogram");

  vec2 typed_pos = {0.25f, 0.9f};
  typed = ui_text_create(u_ctx, typed_pos, string_buffer, 24.f, test_font,
                         UI_ALIGN_LEFT | UI_ALIGN_BOTTOM);
  ui_text_set_colors(&typed, white, 0);

  vec2 explain_pos = {0.5f, 0.85f};
  explain =
      ui_text_create(u_ctx, explain_pos, "Press tab to start typing a message!",
                     24.f, test_font, UI_ALIGN_CENTER);
  ui_text_set_colors(&explain, white, 0);

  vec2 tab_pos    = {0.f, 1.0f};
  vec2 tab_offset = {15.f, -15.f};
  ui_scale_move_px(u_ctx, tab_pos, tab_pos, tab_offset);

  strcpy(tab_str, "Tab OFF");
  tab_msg = ui_text_create(u_ctx, tab_pos, tab_str, 24.f, test_font,
                           UI_ALIGN_LEFT | UI_ALIGN_BOTTOM);
  ui_text_set_colors(&tab_msg, red, darkred);
  vec4_dup(tab_msg.color, red);
  tab_msg.shadow_size = 10.f;
  tab_msg.use_shadow  = 1;

  text = ui_text_create(u_ctx, text_pos,
                        "I wonder how well this all will scale long term.",
                        32.f, test_font, UI_ALIGN_LEFT);

  text_time        = 0.f;
  text_rate        = 150.f;
  text_count       = 0;
  text.use_reveal  = 0;
  text.use_shadow  = 1;
  text.shadow_size = 25.f;

  vec4 text_shadow_color = {0.f, 0.f, 0.f, 0.7f};
  vec4_dup(text.shadow, text_shadow_color);

  text.use_box = 1;
  vec2_clear(text.bounds);
  text.bounds[0] = window_size[0];

  text.reveal = text.text;

  float max_size = ui_text_max_size(u_ctx, text, window_size, 0);
  text.size      = max_size;

  text2 = ui_text_create(u_ctx, text_pos2, "Astera Input Example", 32.f,
                         test_font, UI_ALIGN_CENTER | UI_ALIGN_BOTTOM);

  vec2 button_pos         = {0.5f, 0.5f};
  vec2 button_size        = {0.25f, 0.1f};
  vec4 button_hover_color = {1.f, 1.f, 0.f, 1.f};
  int  alignment          = UI_ALIGN_MIDDLE | UI_ALIGN_CENTER;

  button      = ui_button_create(u_ctx, button_pos, button_size, "Hello world.",
                            alignment, 32.f);
  button.font = test_font;
  vec4_dup(button.bg, white);
  vec4_dup(button.hover_bg, button_hover_color);
  vec4 button_color = {1.f, 1.f, 1.f, 1.f};
  vec4_dup(button.color, black);
  vec4_dup(button.hover_color, black);

  vec2 dropdown_position = {0.5f, 0.5f};
  vec2 dropdown_size     = {0.20f, 0.075f};

  char* options[] = {
      "Test - A",
      "Test - B",
      "Test - C",
      "Test - D",
  };

  dropdown =
      ui_dropdown_create(u_ctx, dropdown_position, dropdown_size, options, 4);

  dropdown.border_radius = 5.f;
  dropdown.border_size   = 3.f;
  dropdown.use_border    = 1;

  option_e = ui_dropdown_add_option(&dropdown, "Test - E");

  float dropdown_font_size = ui_dropdown_max_font_size(u_ctx, dropdown);
  dropdown.font_size       = dropdown_font_size;

  dropdown.align          = UI_ALIGN_MIDDLE | UI_ALIGN_CENTER;
  dropdown.option_display = 5;

  vec4_dup(dropdown.border_color, black);
  vec4_dup(dropdown.hover_border_color, black);

  vec4_dup(dropdown.bg, dark2);
  vec4_dup(dropdown.hover_bg, grey);
  vec4_dup(dropdown.color, grey2);
  vec4_dup(dropdown.hover_color, white);
  vec4_dup(dropdown.select_color, white);
  vec4_dup(dropdown.hover_select_color, white);

  tree = ui_tree_create(16);

  vec2 line_start = {0.25f, 0.1f};
  vec2 line_end   = {0.75f, 0.1f};
  vec4 line_color = {0.2f, 0.2f, 0.2f, 1.f};
  line = ui_line_create(u_ctx, line_start, line_end, line_color, 3.f);

  asset_t* ui_img_file = asset_get("resources/textures/icon.png");

  vec2 img_pos = {10.f, 10.f};
  vec2 img_size;
  vec2 img_px_size = {75.f, 75.f};

  ui_px_from_scale(img_size, img_px_size, window_size);
  ui_px_to_scale(u_ctx, img_pos, img_pos);
  img =
      ui_img_create(u_ctx, ui_img_file->data, ui_img_file->data_length,
                    IMG_NEAREST | IMG_REPEATX | IMG_REPEATY, img_pos, img_size);

  img.border_size = 3.f;
  // ui_img_set_colors(&img, white, white);
  ui_img_set_border_radius(&img, 25.f);

  asset_free(ui_img_file);
  free(ui_img_file);

  ui_element button_ele   = ui_element_get(&button, UI_BUTTON);
  vec2       center_point = {0.5f, 0.5f};

  ui_element_center_to(button_ele, center_point);

  ui_element dropdown_ele = ui_element_get(&dropdown, UI_DROPDOWN);
  center_point[0]         = 0.5f;
  center_point[1]         = 0.175f;

  ui_element_center_to(dropdown_ele, center_point);

  ui_element line_ele = ui_element_get(&line, UI_LINE);
  center_point[1]     = 0.1f;
  ui_element_center_to(line_ele, center_point);

  ui_tree_add(u_ctx, &tree, &text, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &text2, UI_TEXT, 0, 0, 0);
  dropdown_uid = ui_tree_add(u_ctx, &tree, &dropdown, UI_DROPDOWN, 3, 1, 1);
  button_uid   = ui_tree_add(u_ctx, &tree, &button, UI_BUTTON, 0, 1, 0);
  ui_tree_add(u_ctx, &tree, &line, UI_LINE, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &img, UI_IMG, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &typed, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &explain, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &tab_msg, UI_TEXT, 0, 0, 0);
  ui_tree_print(&tree);
  tree.loop = 0;
}

void init() {
  r_window_params params =
      r_window_params_create(1280, 720, 0, 0, 1, 0, 0, "Input Example");

  input_ctx = i_ctx_create(16, 16, 0, 8, 16, 32);

  // Create a shell of a render context, since we're not using it for actual
  // drawing
  render_ctx = r_ctx_create(params, 0, 0, 0, 0, 0);

  window_size[0] = (float)params.width;
  window_size[1] = (float)params.height;

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  // If only finding happiiness in real life was this simple
  // i_joy_create(input_ctx, 0);

  init_ui();

  running = 1;
}

void render(time_s delta) {
  r_window_clear();

  ui_frame_start(u_ctx);
  ui_tree_draw(u_ctx, &tree);
  ui_frame_end(u_ctx);

  r_window_swap_buffers(render_ctx);
}

void input(time_s delta) {
  i_ctx_update(input_ctx);

  vec2 mouse_pos = {i_mouse_get_x(input_ctx), i_mouse_get_y(input_ctx)};
  ui_ctx_update(u_ctx, mouse_pos);

  /*int16_t joy_id = i_joy_connected(input_ctx);
  if (joy_id > -1) {
    if (i_joy_clicked(input_ctx, XBOX_R1)) {
      ui_tree_next(&tree);
    }

    if (i_joy_clicked(input_ctx, XBOX_L1)) {
      ui_tree_prev(&tree);
    }

    if (i_joy_clicked(input_ctx, XBOX_A)) {
      ui_tree_select(u_ctx, &tree, 1, 0);
    }
  }*/

  int32_t event_type = 0;
  if ((event_type = ui_tree_check_event(&tree, button_uid))) {
    printf("Hello from the button!\n");
  }

  if ((event_type = ui_tree_check_event(&tree, dropdown_uid))) {
    if (dropdown.showing) {
      ui_dropdown_set_to_cursor(&dropdown);
      dropdown.showing = 0;
    } else {
      dropdown.showing = 1;
    }
  }

  if (i_key_clicked(input_ctx, KEY_ESCAPE) ||
      r_window_should_close(render_ctx)) {
    running = 0;
  }

  if (i_key_clicked(input_ctx, KEY_TAB)) {
    record_string = !record_string;
    if (record_string) {
      strcpy(tab_str, "Tab ON");
      ui_text_set_colors(&tab_msg, green, darkgreen);
    } else {
      strcpy(tab_str, "Tab OFF");
      ui_text_set_colors(&tab_msg, red, darkred);
    }
  }

  int char_track = i_get_char_tracking(input_ctx);
  if (record_string) {
    // backspace cooldown (repeating, prevents super fast backspace)
    static time_s bkp_cd = MS_TO_SEC / 8; // 4 backspaces repeating per second
    static time_s bkp_timer = 0;          // time til allowed to backspace

    if (bkp_timer <= 0) {
      if (i_key_clicked(input_ctx, KEY_BACKSPACE)) {
        if (string_count > 0) {
          string_buffer[string_count - 1] = 0;
          --string_count;
          bkp_timer = bkp_cd;
        }
      } else if (i_key_down(input_ctx, KEY_BACKSPACE)) {
        if (string_count > 0) {
          string_buffer[string_count - 1] = 0;
          --string_count;
          bkp_timer = bkp_cd;
        }
      }
    } else {
      bkp_timer -= delta;
    }

    if (!char_track) {
      i_set_char_tracking(input_ctx, 1);
    }

    if (i_get_char_count(input_ctx) > 0) {
      frame_chars_count =
          i_get_chars(input_ctx, frame_chars, STRING_FRAME_SIZE - 1);

      for (int i = 0; i < frame_chars_count; ++i) {
        if (string_count + 1 < STRING_BUFFER_SIZE) {
          string_buffer[string_count] = frame_chars[i];
          ++string_count;
        }

        // clear out this frame's array as we go thru it
        frame_chars[i] = 0;
      }

      i_clear_chars(input_ctx);
    }
  } else {
    if (char_track) {
      i_set_char_tracking(input_ctx, 0);
    }

    if (i_key_clicked(input_ctx, 'E')) {
      ui_tree_next(&tree);
    }

    if (i_key_clicked(input_ctx, 'Q')) {
      ui_tree_prev(&tree);
    }

    if (i_key_clicked(input_ctx, KEY_SPACE)) {
      ui_tree_select(u_ctx, &tree, 1, 0);
    }

    if (i_mouse_clicked(input_ctx, 0)) {
      ui_tree_select(u_ctx, &tree, 1, 1);
    }
  }
}

void update(time_s delta) { uint32_t active = ui_tree_check(u_ctx, &tree); }

int main(void) {
  init();

  time_s frame_time = MS_TO_SEC / 60;

  while (running) {
    input(frame_time);
    render(frame_time);
    update(frame_time);
  }

  i_ctx_destroy(input_ctx);
  ui_ctx_destroy(u_ctx);
  r_ctx_destroy(render_ctx);

  asset_free(font_asset);
  free(input_ctx);
  free(render_ctx);
  free(u_ctx);

  return 0;
}

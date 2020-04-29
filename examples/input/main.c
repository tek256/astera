// NOTE: At this time the dropdown implementation is buggy

#include <stdio.h>

#include <astera/debug.h>

#include <astera/asset.h>
#include <astera/input.h>
#include <astera/render.h>
#include <astera/sys.h>
#define ASTERA_DEBUG_OUTPUT
#include <astera/ui.h>

#include <string.h>
#include <assert.h>

static int running = 0;

vec2 window_size;

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

void init_ui() {
  ui_init(window_size, 1.f, 1);

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

  vec2     text_pos   = {0.5f, 0.05f};
  vec2     text_pos2  = {0.5f, 0.065f};
  asset_t* font_asset = asset_get("resources/fonts/monogram.ttf");

  test_font =
      ui_font_create(font_asset->data, font_asset->data_length, "monogram");

  vec2 typed_pos = {0.25f, 0.9f};
  typed          = ui_text_create(typed_pos, string_buffer, 24.f, test_font,
                         UI_ALIGN_LEFT | UI_ALIGN_BOTTOM);
  ui_text_set_colors(&typed, white, 0);

  vec2 explain_pos = {0.5f, 0.85f};
  explain = ui_text_create(explain_pos, "Press tab to start typing a message!",
                           24.f, test_font, UI_ALIGN_CENTER);
  ui_text_set_colors(&explain, white, 0);

  vec2 tab_pos    = {0.f, 1.0f};
  vec2 tab_offset = {15.f, -15.f};
  ui_scale_move_px(tab_pos, tab_pos, tab_offset);

  strcpy(tab_str, "Tab OFF");
  tab_msg = ui_text_create(tab_pos, tab_str, 24.f, test_font,
                           UI_ALIGN_LEFT | UI_ALIGN_BOTTOM);
  ui_text_set_colors(&tab_msg, red, darkred);
  vec4_dup(tab_msg.color, red);
  tab_msg.shadow_size = 10.f;
  tab_msg.use_shadow  = 1;

  text = ui_text_create(text_pos,
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

  float max_size = ui_text_max_size(text, window_size, 0);
  text.size      = max_size;

  text2 = ui_text_create(text_pos2, "Astera Input Example", 32.f, test_font,
                         UI_ALIGN_CENTER | UI_ALIGN_BOTTOM);

  vec2 button_pos         = {0.5f, 0.5f};
  vec2 button_size        = {0.25f, 0.1f};
  vec4 button_hover_color = {1.f, 1.f, 0.f, 1.f};
  int  alignment          = UI_ALIGN_LEFT | UI_ALIGN_BOTTOM;

  button = ui_button_create(button_pos, button_size, "Hello world.", alignment,
                            16.f);
  button.font = test_font;
  vec4_dup(button.bg, white);
  vec4_dup(button.hover_bg, button_hover_color);
  vec4 button_color = {1.f, 1.f, 1.f, 1.f};
  vec4_dup(button.color, button_color);
  vec4_dup(button.hover_color, button_color);

  vec2 dropdown_position = {0.5f, 0.5f};
  vec2 dropdown_size     = {0.20f, 0.075f};

  char* options[] = {
      "Test - A",
      "Test - B",
      "Test - C",
      "Test - D",
  };

  dropdown = ui_dropdown_create(dropdown_position, dropdown_size, options, 4);

  dropdown.border_radius = 5.f;
  dropdown.border_size   = 3.f;
  dropdown.use_border    = 1;

  option_e = ui_dropdown_add_option(&dropdown, "Test - E");

  float dropdown_font_size = ui_dropdown_max_font_size(dropdown);
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

  tree = ui_tree_create(64);

  vec2 line_start = {0.25f, 0.1f};
  vec2 line_end   = {0.75f, 0.1f};
  vec4 line_color = {0.2f, 0.2f, 0.2f, 1.f};
  line            = ui_line_create(line_start, line_end, line_color, 3.f);

  asset_t* ui_img_file = asset_get("resources/textures/icon.png");

  vec2 img_pos = {10.f, 10.f};
  vec2 img_size;
  vec2 img_px_size = {75.f, 75.f};

  ui_px_from_scale(img_size, img_px_size, window_size);
  ui_px_to_scale(img_pos, img_pos);
  img = ui_image_create(ui_img_file->data, ui_img_file->data_length,
                        IMG_NEAREST | IMG_REPEATX | IMG_REPEATY, img_pos,
                        img_size);

  img.border_size = 3.f;
  // ui_img_set_colors(&img, white, white);
  ui_img_set_border_radius(&img, 25.f);

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

  ui_tree_add(&tree, &text, UI_TEXT, 0, 0);
  ui_tree_add(&tree, &text2, UI_TEXT, 0, 0);
  button_uid   = ui_tree_add(&tree, &button, UI_BUTTON, 0, 1);
  dropdown_uid = ui_tree_add(&tree, &dropdown, UI_DROPDOWN, 3, 1);
  ui_tree_add(&tree, &line, UI_LINE, 0, 0);
  ui_tree_add(&tree, &img, UI_IMAGE, 0, 0);
  ui_tree_add(&tree, &typed, UI_TEXT, 0, 0);
  ui_tree_add(&tree, &explain, UI_TEXT, 0, 0);
  ui_tree_add(&tree, &tab_msg, UI_TEXT, 0, 0);
}

void init() {
  r_window_info window_info;
  window_info.width        = 1280;
  window_info.height       = 720;
  window_info.refresh_rate = 60;
  window_info.vsync        = 1;
  window_info.fullscreen   = 0;
  window_info.borderless   = 0;
  window_info.title        = "Input Example";

  if (!r_init(window_info)) {
    printf("Error: unable to initialize rendering system.\n");
    return;
  }

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

  ui_frame_start();
  ui_tree_draw(tree);

  ui_frame_end();

  r_window_swap_buffers();
}

void input(time_s delta) {
  i_update();
  r_poll_events();

  vec2 mouse_pos = {i_get_mouse_x(), i_get_mouse_y()};
  ui_update(mouse_pos);

  uint16_t joy_id = i_joy_connected();
  if (joy_id > -1) {
    if (i_joy_button_clicked(XBOX_R1)) {
      ui_tree_next(&tree);
    }

    if (i_joy_button_clicked(XBOX_L1)) {
      ui_tree_prev(&tree);
    }

    if (i_joy_button_clicked(XBOX_A)) {
      ui_tree_select(&tree, 1, 0);
    }
  }

  int32_t event_type = 0;
  if ((event_type = ui_element_event(&tree, button_uid))) {
    printf("Hello from the button!\n");
  }

  if ((event_type = ui_element_event(&tree, dropdown_uid))) {
    if (dropdown.showing) {
      ui_dropdown_set_to_cursor(&dropdown);
      dropdown.showing = 0;
    } else {
      dropdown.showing = 1;
    }
  }

  if (i_key_clicked(KEY_ESCAPE) || r_window_should_close()) {
    running = 0;
  }

  if (i_key_clicked(KEY_TAB)) {
    record_string = !record_string;
    if (record_string) {
      strcpy(tab_str, "Tab ON");
      ui_text_set_colors(&tab_msg, green, darkgreen);
    } else {
      strcpy(tab_str, "Tab OFF");
      ui_text_set_colors(&tab_msg, red, darkred);
    }
  }

  int char_track = i_get_char_tracking();
  if (record_string) {
    // backspace cooldown (repeating, prevents super fast backspace)
    static time_s bkp_cd = MS_TO_SEC / 8; // 4 backspaces repeating per second
    static time_s bkp_timer = 0;          // time til allowed to backspace

    if (bkp_timer <= 0) {
      if (i_key_clicked(KEY_BACKSPACE)) {
        if (string_count > 0) {
          string_buffer[string_count - 1] = 0;
          --string_count;
          bkp_timer = bkp_cd;
        }
      } else if (i_key_down(KEY_BACKSPACE)) {
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
      i_set_char_tracking(1);
    }

    if (i_get_char_count() > 0) {
      frame_chars_count = i_get_chars(frame_chars, STRING_FRAME_SIZE - 1);

      for (int i = 0; i < frame_chars_count; ++i) {
        if (string_count + 1 < STRING_BUFFER_SIZE) {
          string_buffer[string_count] = frame_chars[i];
          ++string_count;
        }

        // clear out this frame's array as we go thru it
        frame_chars[i] = 0;
      }

      i_clear_chars();
    }
  } else {
    if (char_track) {
      i_set_char_tracking(0);
    }

    if (i_key_clicked('E')) {
      ui_tree_next(&tree);
    }

    if (i_key_clicked('Q')) {
      ui_tree_prev(&tree);
    }

    if (i_key_clicked(KEY_SPACE)) {
      ui_tree_select(&tree, 1, 0);
    }

    if (i_mouse_clicked(0)) {
      ui_tree_select(&tree, 1, 1);
    }
  }
}

void update(time_s delta) { uint32_t active = ui_tree_check(&tree); }

int main(void) {
  printf("Hello world.\n");
  init();

  time_s frame_time = MS_TO_SEC / 60;

  while (running) {
    input(frame_time);
    render(frame_time);
    update(frame_time);
  }

  return 1;
}

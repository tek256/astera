#include <stdio.h>

#define ASTERA_DEBUG_OUTPUT
#include <astera/debug.h>

#include <astera/asset.h>
#include <astera/col.h>
#include <astera/input.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/ui.h>

#include <assert.h>

static int running = 0;

vec2 window_size;

ui_text     text, text2;
ui_box      box;
ui_font     test_font;
ui_button   button;
ui_dropdown dropdown;
ui_tree     tree;
ui_line     line; // OH, and images
ui_img      img;

uint32_t box_uid, button_uid, dropdown_uid;

float move_speed;

float text_time, text_rate;
int   text_count;

int option_a, option_b, option_c, option_d, option_e;

c_aabb   aabb, aabb2;
c_circle circle, circle2;

vec4 aabb_color, aabb2_color;
vec2 aabb_size, aabb2_size;
vec4 circ_color, circ2_color;

vec2 cam_offset;

void init_ui() {
  ui_init(window_size, 1.f, 1);

  vec2     text_pos   = {0.5f, 0.05f};
  vec2     text_pos2  = {0.5f, 0.065f};
  asset_t* font_asset = asset_get("resources/fonts/monogram.ttf");

  test_font =
      ui_font_create(font_asset->data, font_asset->data_length, "monogram");

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

  text2 = ui_text_create(text_pos2, "Astera Input Example", 24.f, test_font,
                         UI_ALIGN_CENTER | UI_ALIGN_BOTTOM);

  vec2 box_pos          = {0.0f, 0.0f};
  vec2 box_size         = {0.2f, 0.2f};
  vec4 box_color        = {1.f, 0.f, 0.f, 1.f};
  vec4 box_border_color = {0.f, 0.f, 0.f, 1.f};
  vec4 box_hover_color  = {1.f, 0.f, 1.f, 1.f};

  box               = ui_box_create(box_pos, box_size, box_color, 0);
  box.border_radius = 5.f;
  box.border_size   = 3.f;
  vec4_dup(box.border_color, box_border_color);
  vec4_dup(box.hover_bg, box_hover_color);
  box.use_border = 1;

  vec2 button_pos         = {0.25f, 0.25f};
  vec2 button_size        = {0.25f, 0.25f};
  vec4 button_hover_color = {1.f, 1.f, 0.f, 1.f};
  int  alignment          = UI_ALIGN_LEFT | UI_ALIGN_BOTTOM;

  button = ui_button_create(button_pos, button_size, "Hello world.", alignment,
                            16.f);
  button.font = test_font;
  vec4_dup(button.bg, box_color);
  vec4_dup(button.hover_bg, button_hover_color);
  vec4 button_color = {1.f, 1.f, 1.f, 1.f};
  vec4_dup(button.color, button_color);
  vec4_dup(button.hover_color, button_color);

  vec2 dropdown_position = {0.5f, 0.5f};
  vec2 dropdown_size     = {0.20f, 0.075f};

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

  /*  vec4 dropdown_bg           = {0.2f, 0.2f, 0.2f, 1.f};
    vec4 dropdown_hover_bg     = {0.4f, 0.4f, 0.4f, 1.f};
    vec4 dropdown_color        = {0.8f, 0.8f, 0.8f, 0.8f};
    vec4 dropdown_hover_color  = {1.f, 1.f, 1.f, 1.f};
    vec4 dropdown_select_color = {1.f, 1.f, 1.f, 1.f};
    vec4 dropdown_border_color = {0.3f, 0.3f, 0.3f, 0.7f};*/

  dropdown = ui_dropdown_create(dropdown_position, dropdown_size, 0, 0);

  dropdown.border_radius = 5.f;
  dropdown.border_size   = 3.f;
  dropdown.use_border    = 1;

  option_a = ui_dropdown_add_option(&dropdown, "Test - A");
  option_b = ui_dropdown_add_option(&dropdown, "Test - B");
  option_c = ui_dropdown_add_option(&dropdown, "Test - C");
  option_d = ui_dropdown_add_option(&dropdown, "Test - D");
  option_e = ui_dropdown_add_option(&dropdown, "Test - E");
  ui_dropdown_add_option(&dropdown, "Test - F");
  ui_dropdown_add_option(&dropdown, "Test - G");
  ui_dropdown_add_option(&dropdown, "Test - H");
  ui_dropdown_add_option(&dropdown, "Test - I");
  ui_dropdown_add_option(&dropdown, "Test - J");

  float dropdown_font_size = ui_dropdown_max_font_size(dropdown);
  dropdown.font_size       = dropdown_font_size;

  dropdown.align             = UI_ALIGN_MIDDLE | UI_ALIGN_CENTER;
  dropdown.showing           = 0;
  dropdown.option_display    = 5;
  dropdown.top_scroll_pad    = 2;
  dropdown.bottom_scroll_pad = 2;

  vec4_dup(dropdown.border_color, black);
  vec4_dup(dropdown.hover_border_color, black);

  vec4_dup(dropdown.bg, dark2);
  vec4_dup(dropdown.hover_bg, grey);
  vec4_dup(dropdown.color, grey2);
  vec4_dup(dropdown.hover_color, white);
  vec4_dup(dropdown.select_color, white);
  vec4_dup(dropdown.hover_select_color, white);

  /*  vec4_dup(dropdown.border_color, dropdown_border_color);
    vec4_dup(dropdown.hover_border_color, dropdown_border_color);

    vec4_dup(dropdown.bg, dropdown_bg);
    vec4_dup(dropdown.hover_bg, dropdown_hover_bg);
    vec4_dup(dropdown.color, dropdown_color);
    vec4_dup(dropdown.hover_color, dropdown_hover_color);
    vec4_dup(dropdown.select_color, dropdown_select_color);
    vec4_dup(dropdown.hover_select_color, dropdown_select_color);*/

  tree = ui_tree_create(64);

  vec2 line_start = {0.25f, 0.1f};
  vec2 line_end   = {0.75f, 0.1f};
  vec4 line_color = {0.2f, 0.2f, 0.2f, 1.f};
  line            = ui_line_create(line_start, line_end, line_color, 3.f);

  /*asset_t* ui_img_file = asset_get("resources/textures/icon.png");

  if (ui_img_file->data) {
    printf("Loaded img data sizeof %i\n", ui_img_file->data_length);
  }

  vec2 img_pos = {0.25f, 0.05f};
  vec2 img_size;
  vec2 img_px_size = {75.f, 75.f};

  ui_px_from_scale(img_size, img_px_size, window_size);
  img = ui_image_create(ui_img_file->data, ui_img_file->data_length,
                        IMG_NEAREST | IMG_REPEATX | IMG_REPEATY |
                            IMG_GENERATE_MIPMAPS,
                        img_pos, img_size);*/

  ui_tree_add(&tree, &text, UI_TEXT, 0);
  ui_tree_add(&tree, &text2, UI_TEXT, 0);
  // box_uid      = ui_tree_add(&tree, &box, UI_BOX, 1);
  button_uid   = ui_tree_add(&tree, &button, UI_BUTTON, 1);
  dropdown_uid = ui_tree_add(&tree, &dropdown, UI_DROPDOWN, 1);
  ui_tree_add(&tree, &line, UI_LINE, 0);
  // ui_tree_add(&tree, &img, UI_IMAGE, 0);
}

void init_col() {
  move_speed = 100.f;

  vec2  aabb_pos;
  vec2  aabb2_pos;
  vec2  circ_pos, circ2_pos;
  float circ_rad = 5.f, circ2_rad = 25.f;

  aabb_pos[0] = 10.f;
  aabb_pos[1] = 10.f;

  aabb_size[0] = 25.f;
  aabb_size[1] = 25.f;

  aabb2_pos[0] = 200.f;
  aabb2_pos[1] = 200.f;

  aabb2_size[0] = 100.f;
  aabb2_size[1] = 100.f;

  aabb = c_aabb_get(aabb_pos, (vec2){aabb_size[0] * 0.5f, aabb_size[1] * 0.5f});
  aabb2 =
      c_aabb_get(aabb2_pos, (vec2){aabb2_size[0] * 0.5f, aabb2_size[1] * 0.5f});

  circ_pos[0] = 100.f;
  circ_pos[0] = 100.f;

  circ2_pos[0] = 300.f;
  circ2_pos[0] = 100.f;

  circle  = c_circ_get(circ_pos, circ_rad);
  circle2 = c_circ_get(circ2_pos, circ2_rad);

  ui_get_color(circ_color, "EEE");
  printf("%.2f %.2f %.2f %.2f\n", circ_color[0], circ_color[1], circ_color[2],
         circ_color[3]);
  ui_get_color(circ2_color, "666");
  ui_get_color(aabb_color, "333");
  ui_get_color(aabb2_color, "F8671A");
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

  init_col();

  running = 1;
}

void render(time_s delta) {
  r_window_clear();

  ui_frame_start();
  // ui_tree_draw(tree);

  vec2 a_c, a2_c, c_c, c2_c;
  vec2_dup(a_c, aabb.center);
  vec2_dup(a2_c, aabb2.center);
  vec2_dup(c_c, circle.center);
  vec2_dup(c2_c, circle2.center);

  a_c[1] *= -1.f;
  a2_c[1] *= -1.f;
  c_c[1] *= -1.f;
  c2_c[1] *= -1.f;

  vec2_add(a_c, a_c, cam_offset);
  vec2_add(a2_c, a2_c, cam_offset);
  vec2_add(c_c, c_c, cam_offset);
  vec2_add(c2_c, c2_c, cam_offset);

  ui_im_box_draw(a_c, aabb_size, aabb_color);
  ui_im_box_draw(a2_c, aabb2_size, aabb2_color);

  ui_im_circle_draw(c_c, circle.radius, circ_color);
  ui_im_circle_draw(c2_c, circle2.radius, circ2_color);

  // whatever, 2nd circle isn't drawing. YOLO
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
    if (i_joy_exists(joy_id)) {
      // printf("Joy exists!\n");
    }
    if (i_joy_button_clicked(XBOX_R1)) {
      // next
      ui_tree_next(&tree);
    }

    if (i_joy_button_clicked(XBOX_L1)) {
      // prev
      ui_tree_prev(&tree);
    }

    if (i_joy_button_clicked(XBOX_A)) {
      ui_tree_select(&tree, 1, 0);
    }
  }

  int32_t event_type = 0;
  if ((event_type = ui_element_event(&tree, box_uid))) {
    printf("Hello from the box!\n");
  }

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

  if (i_key_clicked('E')) {
    ui_tree_next(&tree);
  }

  if (i_key_clicked('Q')) {
    ui_tree_prev(&tree);
  }

  if (i_key_clicked(KEY_SPACE)) {
    ui_tree_select(&tree, 1, 0);
  }

  if (i_key_clicked('F')) {
    printf("%.2f %.2f\n", aabb.center[0], aabb.center[1]);
  }

  if (i_mouse_clicked(0)) {
    ui_tree_select(&tree, 1, 1);
  }

  if (i_key_clicked(KEY_ESCAPE) || r_window_should_close()) {
    running = 0;
  }

  // now that I think about it, everything is currently being rendered mirrored
  // vertically.
  double norm_delta = delta / MS_TO_SEC;
  if (i_key_down('W')) {
    aabb.center[1] += move_speed * norm_delta;
  } else if (i_key_down('S')) {
    aabb.center[1] -= move_speed * norm_delta;
  }

  if (i_key_down('D')) {
    aabb.center[0] += move_speed * norm_delta;
  } else if (i_key_down('A')) {
    aabb.center[0] -= move_speed * norm_delta;
  }

  if (i_key_down(KEY_RIGHT)) {
    cam_offset[0] += move_speed * norm_delta;
  } else if (i_key_down(KEY_LEFT)) {
    cam_offset[0] -= move_speed * norm_delta;
  }

  if (i_key_down(KEY_UP)) {
    cam_offset[1] += move_speed * norm_delta;
  } else if (i_key_down(KEY_DOWN)) {
    cam_offset[1] -= move_speed * norm_delta;
  }
}

void update(time_s delta) {
  uint32_t active = ui_tree_check(&tree);

  c_man manifold;
  if (c_aabb_vs_aabb(&manifold, aabb, aabb2)) {
    ui_get_color(aabb_color, "1F85DE");
  } else if (c_aabb_vs_circ(&manifold, aabb, circle)) {
    ui_get_color(aabb_color, "1FDE46");
  } else if (c_aabb_vs_circ(&manifold, aabb, circle2)) {
    ui_get_color(aabb_color, "DE831F");
  } else {
    ui_get_color(aabb_color, "555");
  }
}

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

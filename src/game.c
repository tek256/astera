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

#include "ui.h"

#define test_sprite_count 2048

r_sprite sprite, sprite2;
uint32_t *frames;

r_framebuffer fbo;

static int tex_ids, models, flip_x, flip_y;

vec2 window_size;

r_shader screen_shader;

r_sprite test_sprites[test_sprite_count];
a_music *test_music;
a_buf test_buf;
a_req music_req, sfx_req;

ui_text text, text2;
ui_box box;
ui_font test_font;
ui_button button;
ui_dropdown dropdown;
ui_tree tree;
ui_line line; // OH, and images
ui_img img;

uint32_t box_uid, button_uid, dropdown_uid;

float text_time, text_rate;
int text_count;

int g_init(void) {
  r_init_anim_map(32);
  r_init_shader_map(2);
  r_init_batches(4);

  r_cam_set_size(320, 240);

  // Initialize default shader
  asset_t *default_vert = asset_get("sys", "res/shd/main.vert");
  asset_t *default_frag = asset_get("sys", "res/shd/main.frag");
  r_shader default_shader = r_shader_create(default_vert, default_frag);
  asset_free(default_vert);
  asset_free(default_frag);

  // Create the screen's framebuffer shader
  asset_t *screen_vert = asset_get("sys", "res/shd/screen.vert");
  asset_t *screen_frag = asset_get("sys", "res/shd/screen.frag");
  screen_shader = r_shader_create(screen_vert, screen_frag);
  asset_free(screen_vert);
  asset_free(screen_frag);

  // Create the screen's framebuffer proper
  int screen_width, screen_height;
  r_window_get_size(&screen_width, &screen_height);

  window_size[0] = screen_width;
  window_size[1] = screen_height;

  float camera_width, camera_height;
  r_cam_get_size(&camera_width, &camera_height);

  fbo = r_framebuffer_create(screen_width, screen_height, screen_shader);

  // Initialize default texture sheet
  asset_t *default_sheet_file = asset_get("sys", "res/tex/test_sheet.png");
  r_sheet default_sheet = r_sheet_create(default_sheet_file, 16, 16);
  asset_free(default_sheet_file);

  // TODO make this not terrible. (It's gotten better)
  frames = malloc(sizeof(uint32_t) * 4);
  frames[0] = 0;
  frames[1] = 1;
  frames[2] = 2;
  frames[3] = 3;

  // frames = (uint32_t *){0, 1, 2, 3, 4};

  r_anim anim = r_anim_create(default_sheet, frames, 4, 12);
  anim.loop = 1;
  r_anim_cache(anim, "default");

  // Initialize Uniform Arrays
  flip_x = r_shader_setup_array(default_shader, "flip_x", 512, r_int);
  flip_y = r_shader_setup_array(default_shader, "flip_y", 512, r_int);
  models = r_shader_setup_array(default_shader, "models", 512, r_mat);
  tex_ids = r_shader_setup_array(default_shader, "tex_ids", 512, r_int);

  vec2 size, position, position2;
  size[0] = 64.f;
  size[1] = 64.f;

  position[0] = 0.0f;
  position[1] = 0.0f;

  position2[0] = 100.f;
  position2[1] = 0.f;

  r_subtex sub_tex = (r_subtex){default_sheet, 1};
  r_subtex sub_tex2 = (r_subtex){default_sheet, 6};

  sprite = r_sprite_create(default_shader, position, size);
  sprite2 = r_sprite_create(default_shader, position2, size);
  sprite2.layer = 5;
  sprite2.change = 1;

  r_sprite_set_tex(&sprite, sub_tex);
  r_sprite_set_anim(&sprite2, anim);

  r_subtex test_subtex = (r_subtex){default_sheet, 0};

  vec2 test_pos;
  vec2 test_size = {16.f, 16.f};
  for (int i = 0; i < test_sprite_count; ++i) {
    int row = i / 16;
    int col = i % 16;
    test_pos[0] = col * test_size[0];
    test_pos[1] = row * test_size[1];

    test_sprites[i] = r_sprite_create(default_shader, test_pos, test_size);
    test_sprites[i].layer = 0;

    test_subtex.sub_id = (rand() % 4) + 16;

    r_sprite_set_tex(&test_sprites[i], test_subtex);
  }

  // CREATE MUSIC
  music_req = (a_req){0};
  music_req.stop = 0;
  music_req.loop = 0;

  // music_req.layer = 0;
  music_req.gain = 0.2f;
  music_req.range = 10.f;
  music_req.max_range = 100.f;
  asset_t *music_asset = asset_get("sys", "res/snd/test_song.ogg");
  test_music = a_music_create(music_asset, NULL, &music_req);
  a_layer_add_music(1, test_music);

  sfx_req = (a_req){0};
  sfx_req.stop = 0;
  sfx_req.loop = 0;
  sfx_req.gain = 1.f;

  asset_t *sfx_asset = asset_get("sys", "res/snd/powerup.wav");
  test_buf = a_buf_create(sfx_asset);
  asset_free(sfx_asset);

  ui_init(window_size, 1.f, 1);

  vec2 text_pos = {0.0f, 0.1f};
  vec2 text_pos2 = {0.1f, 0.2f};
  asset_t *font_asset = asset_get("sys", "res/fnt/monogram.ttf");
  test_font = ui_font_create(font_asset, "monogram");

  text = ui_text_create(text_pos,
                        "I wonder how well this all will scale long term.",
                        32.f, test_font, UI_ALIGN_LEFT);

  vec2 screen_bounds = {screen_width, screen_height / 3.f};
  text_time = 0.f;
  text_rate = 150.f;
  text_count = 0;
  text.use_reveal = 1;

  text.use_box = 1;
  vec2_clear(text.bounds);
  text.bounds[0] = screen_width;

  text.reveal = text.text;

  float max_size = ui_text_max_size(text, screen_bounds, 0);
  text.size = max_size;

  text2 = ui_text_create(text_pos2, "Yeah, a little bit.", 16.f, test_font,
                         UI_ALIGN_LEFT);

  vec2 box_pos = {0.0f, 0.0f};
  vec2 box_size = {0.2f, 0.2f};
  vec4 box_color = {1.f, 0.f, 0.f, 1.f};
  vec4 box_border_color = {0.f, 0.f, 0.f, 1.f};
  vec4 box_hover_color = {1.f, 0.f, 1.f, 1.f};

  box = ui_box_create(box_pos, box_size, box_color, 0);
  box.border_radius = 5.f;
  box.border_size = 3.f;
  vec4_dup(box.border_color, box_border_color);
  vec4_dup(box.hover_bg, box_hover_color);
  box.use_border = 1;

  vec2 button_pos = {0.25f, 0.25f};
  vec2 button_size = {0.25f, 0.25f};
  vec4 button_hover_color = {1.f, 1.f, 0.f, 1.f};
  int alignment = UI_ALIGN_LEFT | UI_ALIGN_BOTTOM;

  button = ui_button_create(button_pos, button_size, "Hello world.", alignment,
                            16.f);
  button.font = test_font;
  vec4_dup(button.bg, box_color);
  vec4_dup(button.hover_bg, button_hover_color);
  vec4 button_color = {1.f, 1.f, 1.f, 1.f};
  vec4_dup(button.color, button_color);
  vec4_dup(button.hover_color, button_color);

  vec2 dropdown_position = {0.5f, 0.5f};
  vec2 dropdown_size = {0.15f, 0.05f};

  vec4 dropdown_bg = {0.2f, 0.2f, 0.2f, 1.f};
  vec4 dropdown_hover_bg = {0.4f, 0.4f, 0.4f, 1.f};
  vec4 dropdown_color = {0.8f, 0.8f, 0.8f, 0.8f};
  vec4 dropdown_hover_color = {1.f, 1.f, 1.f, 1.f};
  vec4 dropdown_select_color = {1.f, 1.f, 1.f, 1.f};

  dropdown = ui_dropdown_create(dropdown_position, dropdown_size, 0, 0);
  ui_dropdown_add_option(&dropdown, "Test");
  ui_dropdown_add_option(&dropdown, "Test2");
  ui_dropdown_add_option(&dropdown, "Test3");
  ui_dropdown_add_option(&dropdown, "Test4");
  ui_dropdown_add_option(&dropdown, "Test5");

  float dropdown_font_size = ui_dropdown_max_font_size(dropdown);
  dropdown.font_size = dropdown_font_size;

  dropdown.align = UI_ALIGN_MIDDLE | UI_ALIGN_CENTER;
  dropdown.showing = 1;
  dropdown.option_display = 5;

  vec4_dup(dropdown.bg, dropdown_bg);
  vec4_dup(dropdown.hover_bg, dropdown_hover_bg);
  vec4_dup(dropdown.color, dropdown_color);
  vec4_dup(dropdown.hover_color, dropdown_hover_color);
  vec4_dup(dropdown.select_color, dropdown_select_color);
  vec4_dup(dropdown.hover_select_color, dropdown_select_color);

  // Root, MAX_ELEMENTS
  tree = ui_tree_create(64);

  vec2 line_start = {0.2f, 0.2f};
  vec2 line_end = {0.3f, 0.4f};
  vec4 line_color = {1.f, 1.f, 1.f, 1.f};
  line = ui_line_create(line_start, line_end, line_color, 10.f);

  asset_t *ui_img_file = asset_get("sys", "res/tex/icon.png");

  vec2 img_pos = {0.75f, 0.75f};
  vec2 img_size;
  vec2 img_px_size = {75.f, 75.f};
  vec2 screen_scale = {1280.f, 720.f};

  ui_px_from_scale(img_size, img_px_size, screen_scale);
  img = ui_image_create(ui_img_file,
                        IMG_NEAREST | IMG_REPEATX | IMG_REPEATY |
                            IMG_GENERATE_MIPMAPS,
                        img_pos, img_size);

  ui_tree_add(&tree, &text, UI_TEXT, 0);
  ui_tree_add(&tree, &text2, UI_TEXT, 0);
  box_uid = ui_tree_add(&tree, &box, UI_BOX, 1);
  button_uid = ui_tree_add(&tree, &button, UI_BUTTON, 1);
  dropdown_uid = ui_tree_add(&tree, &dropdown, UI_DROPDOWN, 1);
  ui_tree_add(&tree, &line, UI_LINE, 0);
  ui_tree_add(&tree, &img, UI_IMAGE, 0);

  return 1;
}

void g_exit(void) {}

void g_input(time_s delta) {
  if (i_key_clicked(GLFW_KEY_ESCAPE))
    r_window_request_close();

  if (i_key_clicked('P')) {
    a_music_play(test_music);
  } else if (i_key_clicked('O')) {
    a_music_pause(test_music);
  }

  if (i_key_clicked('G')) {
    a_play_sfx(&test_buf, &sfx_req);
  }

  int32_t event_type = 0;
  if ((event_type = ui_element_event(&tree, box_uid))) {
    _l("Box pressed: %i\n", event_type);
  }

  if ((event_type = ui_element_event(&tree, button_uid))) {
    _l("Button pressed: %i\n", event_type);
  }

  event_type = ui_element_event(&tree, dropdown_uid);
  if (event_type != 0) {
    if (dropdown.showing) {
      if (event_type == 1) {
        ui_dropdown_set_to_cursor(&dropdown);
        dropdown.showing = 0;
      }
    }
  }

  if (i_key_clicked(KEY_SPACE)) {
    ui_tree_select(&tree, 1);
  }

  if (i_mouse_clicked(0)) {
    ui_tree_select(&tree, 1);
  }

  if (i_key_clicked('E')) {
    ui_tree_next(&tree);
  } else if (i_key_clicked('Q')) {
    ui_tree_prev(&tree);
  }

  if (i_key_clicked('F')) {
    dropdown.showing = !dropdown.showing;
  }

  vec2 mouse_pos = {i_get_mouse_x(), i_get_mouse_y()};
  ui_update(mouse_pos);

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

  if (dir_x != 0 || dir_y != 0) {
    sprite2.position[0] += dir_x * 0.2f * delta;
    sprite2.position[1] -= dir_y * 0.2f * delta;

    if (dir_x < 0) {
      sprite2.flip_x = 1;
    } else {
      sprite2.flip_x = 0;
    }
    sprite2.change = 1;
    r_sprite_play(&sprite2);
  } else {
    r_sprite_pause(&sprite2);
  }
}

void g_update(time_s delta) {
  text_time += delta;
  if (text_time >= text_rate) {
    text_time -= text_rate;
    ui_text_next(&text);
    //++text.reveal;

    //++text_length;
  }

  r_sprite_update(&sprite, delta);
  r_sprite_update(&sprite2, delta);
  uint32_t id = ui_tree_check(&tree);
}

void g_render(time_s delta) {
  // r_check_error_loc("Sprite Drawcall");
  r_sprite_draw(sprite);

  int tex_id = r_sprite_get_tex_id(sprite);

  // r_check_error_loc("Sprite1 Uniforms");
  r_shader_sprite_uniform(sprite, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite, models, &sprite.model);
  r_shader_sprite_uniform(sprite, flip_x, &sprite.flip_x);
  r_shader_sprite_uniform(sprite, flip_y, &sprite.flip_y);

  tex_id = r_sprite_get_tex_id(sprite2);

  // r_check_error_loc("Sprite2 DrawCall");
  r_sprite_draw(sprite2);
  // r_check_error_loc("Sprite2 Uniforms");
  r_shader_sprite_uniform(sprite2, tex_ids, &tex_id);
  r_shader_sprite_uniform(sprite2, models, &sprite2.model);
  r_shader_sprite_uniform(sprite2, flip_x, &sprite2.flip_x);
  r_shader_sprite_uniform(sprite2, flip_y, &sprite2.flip_y);

  // r_check_error_loc("Array Sprite Draw\n");
  for (int i = 0; i < test_sprite_count; ++i) {
    int tex_id = r_sprite_get_tex_id(test_sprites[i]);

    r_sprite_update(&test_sprites[i], delta);
    r_sprite_draw(test_sprites[i]);
    r_shader_sprite_uniform(test_sprites[i], tex_ids, &tex_id);
    r_shader_sprite_uniform(test_sprites[i], models, &test_sprites[i].model);
    r_shader_sprite_uniform(test_sprites[i], flip_x, &test_sprites[i].flip_x);
    r_shader_sprite_uniform(test_sprites[i], flip_y, &test_sprites[i].flip_y);
  }

  ui_frame_start();

  ui_tree_draw(tree);
  ui_frame_end();
}

void g_frame_start() { //
  r_framebuffer_bind(fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void g_frame_end() { //
  int width, height;
  r_window_get_size(&width, &height);
  glViewport(0, 0, width, height);
  r_framebuffer_draw(fbo);
}

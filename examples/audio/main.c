#include <stdio.h>

// NOTE: outputs internal astera debug based on cmake flags
#include <astera/debug.h>

#include <astera/asset.h>
#include <astera/audio.h>
#include <astera/input.h>
#include <astera/render.h>
#include <astera/sys.h>
#include <astera/ui.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int running = 0;

vec2 window_size;

a_ctx*  audio_ctx;
r_ctx*  render_ctx;
i_ctx*  input_ctx;
ui_ctx* u_ctx;

ui_font   test_font;
ui_slider slider;
ui_tree   tree;
ui_text   explain, timecode, explain2;

uint16_t blip_id;

// Audio resources
uint16_t master_layer;
uint16_t song_id;

a_req    req, sfx_req;
asset_t *font_data, *song_data;

void init_ui() {
  u_ctx = ui_ctx_create(window_size, 1.f, 1, 1);

  vec4 white, off_white, dark, red, clear;

  // Sets it to 0,0,0,0
  vec4_clear(clear);

  ui_get_color(white, "FFF");
  ui_get_color(off_white, "EEE");
  ui_get_color(dark, "0A0A0A");
  ui_get_color(red, "de221f");

  ASTERA_FUNC_DBG("Hello world. %i\n", 4);

  font_data = asset_get("resources/fonts/monogram.ttf");

  test_font = ui_font_create(u_ctx, font_data->data, font_data->data_length,
                             "monogram");

  vec2 explain_pos  = {0.5f, 0.25f};
  vec2 explain2_pos = {0.5f, 0.2f};
  explain =
      ui_text_create(u_ctx, explain_pos,
                     "Press P to play or pause the song, O to stop + reset it!",
                     24.f, test_font, UI_ALIGN_CENTER);

  explain2 =
      ui_text_create(u_ctx, explain2_pos, "Press S to play a sound effect!",
                     24.f, test_font, UI_ALIGN_CENTER);

  ui_text_set_colors(&explain2, white, 0);
  ui_text_set_colors(&explain, white, 0);

  vec2 timecode_pos = {0.5f, 0.375f};

  timecode = ui_text_create(u_ctx, timecode_pos, "0:00 / 1:00", 24.f, test_font,
                            UI_ALIGN_CENTER);
  ui_text_set_colors(&timecode, white, 0);

  ui_element timecode_ele = ui_element_get(&timecode, UI_TEXT);
  ui_element_center_to(timecode_ele, timecode_pos);

  vec2 progress_size = {0.5f, 0.15f};
  vec2 progress_pos  = {0.5f, 0.5f};

  vec2 button_size = {0.025f, 0.15f};
  slider = ui_slider_create(u_ctx, progress_pos, progress_size, button_size, 0,
                            0.f, 0.f, 1.f, 0);

  ui_element slider_ele = ui_element_get(&slider, UI_SLIDER);
  ui_element_center_to(slider_ele, progress_pos);

  ui_slider_set_colors(&slider, clear, clear, red, red, white, white, clear,
                       clear, clear, clear);

  slider.fill_padding  = 10.f;
  slider.border_size   = 5.f;
  slider.border_radius = 5.f;

  tree = ui_tree_create(5);
  ui_tree_add(u_ctx, &tree, &timecode, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &slider, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &tree, &explain, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &explain2, UI_TEXT, 0, 0, 0);
  ui_tree_print(&tree);
  tree.loop = 0;
}

void init_audio() {
  // ASTERA_DBG("Test!.\n");
  /*  uint16_t a_ctx_layer_create(a_ctx * ctx, const char* name, uint16_t
     max_sfx, uint16_t max_songs);*/

  master_layer = a_ctx_layer_create(audio_ctx, "master", 8, 2);
  a_layer_set_gain(audio_ctx, master_layer, 1.0f);

  song_data = asset_get("resources/audio/thingy.ogg");
  song_id   = a_song_create(audio_ctx, song_data->data, song_data->data_length,
                          "test", 32, 4, 4096 * 4);

  asset_t* blip_data = asset_get("resources/audio/blop.wav");
  blip_id = a_buf_create(audio_ctx, blip_data->data, blip_data->data_length,
                         "blip", 0);
  asset_free(blip_data);
  free(blip_data);
}

void init() {
  r_window_params params =
      r_window_params_create(1280, 720, 0, 0, 1, 0, 0, "Audio Example");
  params.vsync = 1;

  // Create a shell of a render context, since we're not using it for actual
  // drawing
  render_ctx = r_ctx_create(params, 0, 0, 0, 0, 0);

  input_ctx = i_ctx_create(16, 16, 32, 8, 16, 32);

  window_size[0] = (float)params.width;
  window_size[1] = (float)params.height;

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  // If only finding happiiness in real life was this simple
  i_joy_create(input_ctx, 0);

  /*a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                    uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                    uint16_t max_filters, uint32_t pcm_size);
*/
  audio_ctx = a_ctx_create(0, 1, 8, 8, 2, 2, 2, 4096 * 4);
  a_listener_set_gain(audio_ctx, 0.1f);

  if (!audio_ctx) {
    return;
  }

  init_audio();

  init_ui();

  running = 1;
}

static char timecode_str[16];

void render(time_s delta) {
  time_s length = a_song_get_length(audio_ctx, song_id);
  time_s time   = a_song_get_time(audio_ctx, song_id);

  int time_min = (int)floor(time / (60 * 1000.f));
  int time_sec = (int)(time - (time_min * 60000.f)) / 1000;

  int len_min = (int)floor(length / (60.f * 1000.f));
  int len_sec = (int)(length - (len_min * 60000.f)) / 1000;

  float prog      = time / length;
  slider.progress = prog;

  memset(timecode_str, 0, sizeof(char) * 16);
  snprintf(timecode_str, 16, "%i:%.2i / %i:%.2i", time_min, time_sec, len_min,
           len_sec);
  timecode.text = timecode_str;

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

  int16_t joy_id = i_joy_connected(input_ctx);
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
  }

  if (i_mouse_clicked(input_ctx, MOUSE_LEFT)) {
    ui_tree_select(u_ctx, &tree, 1, 1);
  }

  if (i_mouse_released(input_ctx, MOUSE_LEFT)) {
    ui_tree_select(u_ctx, &tree, 0, 1);
  }

  if (i_key_clicked(input_ctx, 'S')) {
    vec3 zero = {0.f, 0.f, 0.f};
    sfx_req   = a_req_create(zero, 1.f, 100.f, 0, 0, 0, 0, 0);
    a_sfx_play(audio_ctx, 0, blip_id, &sfx_req);
  }

  if (i_key_clicked(input_ctx, KEY_ESCAPE) ||
      r_window_should_close(render_ctx)) {
    running = 0;
  }

  if (i_key_clicked(input_ctx, 'O')) {
    a_song_reset(audio_ctx, song_id);
  }

  if (i_key_clicked(input_ctx, KEY_P)) {
    ALenum song_state = a_song_get_state(audio_ctx, song_id);
    vec3   song_pos   = {0.f, 0.f, 0.f};
    req               = a_req_create(song_pos, 1.f, 100.f, 1, 0, 0, 0, 0);

    if (song_state == AL_INITIAL || song_state == AL_STOPPED) {
      a_song_play(audio_ctx, master_layer, song_id, &req);
    } else if (song_state == AL_PLAYING) {
      a_song_pause(audio_ctx, song_id);
    } else if (song_state == AL_PAUSED) {
      a_song_resume(audio_ctx, song_id);
    }
  }
}

void update(time_s delta) {
  uint32_t active = ui_tree_check(u_ctx, &tree);
  a_ctx_update(audio_ctx);
}

int main(void) {
  init();

  time_s frame_time = MS_TO_SEC / 60;

  s_timer timer = s_timer_create();
  s_timer_update(&timer);

  while (running) {
    time_s delta = s_timer_update(&timer);
    input(delta);
    render(delta);
    update(delta);
  }

  a_ctx_destroy(audio_ctx);
  r_ctx_destroy(render_ctx);
  i_ctx_destroy(input_ctx);

  free(audio_ctx);
  free(render_ctx);
  free(input_ctx);

  asset_free(song_data);
  asset_free(font_data);
  free(song_data);
  free(font_data);

  return 0;
}


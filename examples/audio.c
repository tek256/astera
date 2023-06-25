#include <stdio.h>

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
ui_progress progress;
ui_slider master_vol, sfx_vol, music_vol;
ui_text   master_vol_label, sfx_vol_label, music_vol_label;
ui_tree   tree;
ui_text   explain, timecode, explain2;

int16_t blip_id;

/// Test reverb
uint16_t    fx_slots[8];
uint16_t    filter_slots[8];
uint8_t     fx_count;
uint8_t     filter_count;
a_fx_reverb reverb;
a_filter*   filter;

uint8_t  use_filter;
uint8_t  use_reverb;
uint32_t reverb_id;

// Audio resources
uint16_t music_layer, sfx_layer;
uint16_t song_id;

a_req    req, sfx_req;
asset_t *font_data, *song_data;

void init_ui() {
  u_ctx = ui_ctx_create(window_size, 1.f, 1, 1, 1);

  ui_color white, off_white, dark, red, clear;

  ui_color_clear(clear);

  ui_get_color(white, "FFF");
  ui_get_color(off_white, "EEE");
  ui_get_color(dark, "0A0A0A");
  ui_get_color(red, "de221f");

  ASTERA_FUNC_DBG("Hello world. %i\n", 4);

  font_data = asset_get("resources/fonts/OpenSans-Regular.ttf");

  test_font = ui_font_create(u_ctx, font_data->data, font_data->data_length,
                             "OpenSans");

  vec2 explain_pos  = {0.5f, 0.1f};
  vec2 explain2_pos = {0.5f, 0.05f};

  explain =
      ui_text_create(u_ctx, explain_pos,
                     "Press P to play or pause the song, O to stop + reset it!",
                     24.f, test_font, UI_ALIGN_CENTER);

  explain2 =
      ui_text_create(u_ctx, explain2_pos, "Press S to play a sound effect!",
                     24.f, test_font, UI_ALIGN_CENTER);

  ui_text_set_colors(&explain2, white, 0);
  ui_text_set_colors(&explain, white, 0);

  vec2 timecode_pos = {0.5f, 0.225f};

  timecode = ui_text_create(u_ctx, timecode_pos, "0:00 / 1:00", 24.f, test_font,
                            UI_ALIGN_CENTER);
  ui_text_set_colors(&timecode, white, 0);

  ui_element timecode_ele = ui_element_get(&timecode, UI_TEXT);
  ui_element_center_to(timecode_ele, timecode_pos);

  vec2 progress_size = {0.5f, 0.15f};
  vec2 progress_pos  = {0.5f, 0.35f};

  vec2 button_size = {0.025f, 0.15f};
  progress = ui_progress_create(u_ctx, progress_pos, progress_size, 0.0f);

  ui_element progress_ele = ui_element_get(&progress, UI_PROGRESS);
  ui_element_center_to(progress_ele, progress_pos);

  ui_progress_set_colors(&progress, clear, clear, red, red, white, white);

  progress.fill_padding  = 6.f;
  progress.border_size   = 2.f;
  progress.border_radius = 5.f;

  float left_pos = 0.5f - (progress_size[0] / 2.f);
  float tmp_pos  = progress_pos[0];

  progress_pos[1] += 0.15f;
  progress_pos[0] = left_pos;

  char* master_vol_label_text = (char*)malloc(sizeof(char) * 12);
  strcpy(master_vol_label_text, "master: 100");

  master_vol_label = ui_text_create(u_ctx, progress_pos, master_vol_label_text,
                                    24.f, test_font, UI_ALIGN_LEFT);

  ui_text_set_colors(&master_vol_label, white, 0);

  progress_pos[0] = tmp_pos;
  progress_pos[1] += 0.05f;
  progress_size[1] = 0.05f;

  master_vol = ui_slider_create(u_ctx, progress_pos, progress_size, button_size,
                                0, 1.f, 0.f, 1.f, 0);

  ui_element slider_ele = ui_element_get(&master_vol, UI_SLIDER);
  ui_element_center_to(slider_ele, progress_pos);

  ui_slider_set_colors(&master_vol, clear, clear, red, red, white, white, clear,
                       clear, clear, clear);

  master_vol.fill_padding  = 6.f;
  master_vol.border_size   = 2.f;
  master_vol.border_radius = 5.f;

  progress_pos[0] = left_pos;
  progress_pos[1] += 0.1f;

  char* sfx_vol_label_text = (char*)malloc(sizeof(char) * 12);
  strcpy(sfx_vol_label_text, "sfx: 100");
  sfx_vol_label = ui_text_create(u_ctx, progress_pos, sfx_vol_label_text, 24.f,
                                 test_font, UI_ALIGN_LEFT);
  ui_text_set_colors(&sfx_vol_label, white, 0);

  progress_pos[0] = tmp_pos;
  progress_pos[1] += 0.05f;

  sfx_vol = ui_slider_create(u_ctx, progress_pos, progress_size, button_size, 0,
                             1.f, 0.f, 1.f, 0);

  slider_ele = ui_element_get(&sfx_vol, UI_SLIDER);
  ui_element_center_to(slider_ele, progress_pos);

  ui_slider_set_colors(&sfx_vol, clear, clear, red, red, white, white, clear,
                       clear, clear, clear);

  sfx_vol.fill_padding  = 6.f;
  sfx_vol.border_size   = 2.f;
  sfx_vol.border_radius = 5.f;

  char* music_vol_label_text = (char*)malloc(sizeof(char) * 12);
  strcpy(music_vol_label_text, "music: 100");

  progress_pos[0] = left_pos;
  progress_pos[1] += 0.1f;

  music_vol_label = ui_text_create(u_ctx, progress_pos, music_vol_label_text,
                                   24.f, test_font, UI_ALIGN_LEFT);

  ui_text_set_colors(&music_vol_label, white, 0);

  progress_pos[0] = tmp_pos;
  progress_pos[1] += 0.05f;
  progress_size[1] = 0.05f;

  music_vol = ui_slider_create(u_ctx, progress_pos, progress_size, button_size,
                               0, 1.f, 0.f, 1.f, 0);

  slider_ele = ui_element_get(&music_vol, UI_SLIDER);
  ui_element_center_to(slider_ele, progress_pos);

  ui_slider_set_colors(&music_vol, clear, clear, red, red, white, white, clear,
                       clear, clear, clear);

  music_vol.fill_padding  = 6.f;
  music_vol.border_size   = 2.f;
  music_vol.border_radius = 5.f;

  progress_pos[0] = left_pos;
  progress_pos[1] += 0.1f;

  // We'll just make it 16 for now
  tree = ui_tree_create(16);
  ui_tree_add(u_ctx, &tree, &timecode, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &progress, UI_PROGRESS, 1, 0, 1);
  ui_tree_add(u_ctx, &tree, &sfx_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &tree, &music_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &tree, &master_vol, UI_SLIDER, 1, 1, 1);
  ui_tree_add(u_ctx, &tree, &sfx_vol_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &master_vol_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &music_vol_label, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &explain, UI_TEXT, 0, 0, 0);
  ui_tree_add(u_ctx, &tree, &explain2, UI_TEXT, 0, 0, 0);
  tree.loop = 0;
}

void init_audio() {
  music_layer = a_layer_create(audio_ctx, "music", 8, 2);
  a_layer_set_gain(audio_ctx, music_layer, 1.0f);

  sfx_layer = a_layer_create(audio_ctx, "sfx", 8, 0);
  a_layer_set_gain(audio_ctx, sfx_layer, 0.8f);

  reverb     = a_fx_reverb_default();
  use_reverb = 1;
  reverb_id  = a_fx_create(audio_ctx, FX_REVERB, &reverb);
  /*uint16_t a_filter_create(a_ctx* ctx, a_filter_type type, float gain, float
     hf, float lf);
  */
  uint16_t filter_id = a_filter_create(audio_ctx, FILTER_LOW, 1.f, 200.f, 0.f);
  filter             = a_filter_get_slot(audio_ctx, filter_id);
  filter_slots[0]    = filter_id;
  ++filter_count;

  fx_slots[0] = reverb_id;
  ++fx_count;

  song_data = asset_get("resources/audio/thingy.ogg");
  song_id   = a_song_create(audio_ctx, song_data->data, song_data->data_length,
                          "test", 32, 4, 4096 * 4);

  if (!song_id) {
    printf("Unable to load song.\n");
  }

  asset_t* blip_data = asset_get("resources/audio/select.wav");
  blip_id = a_buf_create(audio_ctx, blip_data->data, blip_data->data_length,
                         "blip", 0);
  asset_free(blip_data);
}

void init() {
  r_window_params params =
      r_window_params_create(1280, 720, 0, 0, 1, 0, 0, "Audio Example");
  params.vsync = 1;

  // Create a shell of a render context, since we're not using it for actual
  // drawing
  render_ctx = r_ctx_create(params, 0, 0, 0, 0);

  input_ctx = i_ctx_create(16, 16, 32, 5, 32);

  window_size[0] = (float)params.width;
  window_size[1] = (float)params.height;

  r_ctx_make_current(render_ctx);
  r_ctx_set_i_ctx(render_ctx, input_ctx);

  i_joy_create(input_ctx, 0);

  a_ctx_info ctx_info = (a_ctx_info) {
    .device = 0,
    .max_layers = 2,
    .max_buffers = 8,
    .max_sfx = 8,
    .max_fx = 2,
    .max_songs = 1,
    .max_filters = 2,
    .pcm_size = 4096 * 4,
  };

  audio_ctx = a_ctx_create(ctx_info);
  a_listener_set_gain(audio_ctx, 0.5f);

  if (!audio_ctx) {
    return;
  }

  init_audio();

  init_ui();

  running = 1;
}

static char timecode_str[16];

void render(time_s delta) {
  if (song_id) {
    time_s length = a_song_get_length(audio_ctx, song_id);
    time_s time   = a_song_get_time(audio_ctx, song_id);

    int time_min = (int)floor(time / (60 * 1000.f));
    int time_sec = (int)(time - (time_min * 60000.f)) / 1000;

    int len_min = (int)floor(length / (60.f * 1000.f));
    int len_sec = (int)(length - (len_min * 60000.f)) / 1000;

    float prog      = time / length;
    progress.progress = prog;

    memset(timecode_str, 0, sizeof(char) * 16);
    snprintf(timecode_str, 16, "%i:%.2i / %i:%.2i", time_min, time_sec, len_min,
             len_sec);
    timecode.text = timecode_str;
  }

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
    if (i_joy_clicked(input_ctx, 0, XBOX_R1)) {
      ui_tree_next(&tree);
    }

    if (i_joy_clicked(input_ctx, 0, XBOX_L1)) {
      ui_tree_prev(&tree);
    }

    if (i_joy_clicked(input_ctx, 0, XBOX_A)) {
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
    if (use_reverb) {
      sfx_req = a_req_create(zero, 1.f, 100.f, 0, fx_slots, fx_count,
                             filter_slots, filter_count);
    } else {
      sfx_req = a_req_create(zero, 1.f, 100.f, 0, 0, 0, 0, 0);
    }

    a_sfx_play(audio_ctx, sfx_layer, blip_id, &sfx_req);
  }

  if (i_key_clicked(input_ctx, KEY_ESCAPE) ||
      r_window_should_close(render_ctx)) {
    running = 0;
  }

  if (i_key_clicked(input_ctx, 'O') && song_id != 0) {
    a_song_reset(audio_ctx, song_id);
  }

  if (i_key_clicked(input_ctx, KEY_P) && song_id != 0) {
    ALenum song_state = a_song_get_state(audio_ctx, song_id);
    vec3   song_pos   = {0.f, 0.f, 0.f};
    if (use_reverb) {
      req = a_req_create(song_pos, 1.f, 100.f, 1, fx_slots, fx_count,
                         filter_slots, filter_count);
    } else {
      req = a_req_create(song_pos, 1.f, 100.f, 1, 0, 0, 0, 0);
    }

    if (song_state == AL_INITIAL || song_state == AL_STOPPED) {
      a_song_play(audio_ctx, music_layer, song_id, &req);
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

  if (sfx_vol.holding) {
    snprintf(sfx_vol_label.text, 12, "sfx: %i", (int)(sfx_vol.progress * 100));
    a_layer_set_gain(audio_ctx, sfx_layer, sfx_vol.progress);
  }

  if (master_vol.holding) {
    snprintf(master_vol_label.text, 12, "master: %i",
             (int)(master_vol.progress * 100));
    a_listener_set_gain(audio_ctx, master_vol.progress);
  }

  if (music_vol.holding) {
    snprintf(music_vol_label.text, 12, "music: %i",
             (int)(music_vol.progress * 100));
    a_layer_set_gain(audio_ctx, music_layer, music_vol.progress);
  }
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

  asset_free(song_data);
  asset_free(font_data);

  free(sfx_vol_label.text);
  free(master_vol_label.text);
  free(music_vol_label.text);

  return 0;
}


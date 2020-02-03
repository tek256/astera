#include "platform.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio.h"
#include "conf.h"
#include "debug.h"
#include "render.h"

#include "game.h"
#include "input.h"
#include "sys.h"

int target_fps = 120;
int max_fps = 120;

int init_sys() {
  if (!asset_init()) {
    _fatal("Unable to initialize asset system.\n");
    return EXIT_FAILURE;
  }

  if (!i_init()) {
    _fatal("Unable to initialize input system.\n");
    return EXIT_FAILURE;
  }

  c_conf conf;
  r_window_info info;

  char *target_conf_path;

#if defined(CONF_PATH)
  target_conf_path = CONF_PATH;
#else
  target_conf_path = "res/conf.ini";
#endif

  c_table conf_table;

  if (c_has_prefs()) {
    target_conf_path = c_get_pref_p();
  }

  if (target_conf_path) {
    asset_t *conf_file = asset_get("sys", target_conf_path);
    assert(conf_file->data);
    conf_table = c_get_table(conf_file);
    conf = c_parse_table(conf_table);
    asset_free(conf_file);
  } else {
    conf = c_defaults();
  }

  info.width = conf.width;
  info.height = conf.height;
  info.fullscreen = conf.fullscreen;
  info.vsync = conf.vsync;
  info.borderless = conf.borderless;
  info.refreshRate = conf.refreshRate;
  info.icon = conf.icon;
  // info.icon = 0;
  info.gamma = conf.gamma;

#if defined(WINDOW_TITLE)
  info.title = WINDOW_TITLE;
#else
  info.title = "untitled";
#endif

  if (!r_init(info)) {
    _fatal("Unable to initialize rendering system.\n");
    return EXIT_FAILURE;
  }

  max_fps = r_get_refresh_rate();

  if (!a_init(NULL, conf.master, conf.sfx, conf.music)) {
    _fatal("Unable to initialize audio system.\n");
    return EXIT_FAILURE;
  }

  if (!g_init()) {
    _fatal("Unable to initialize game runtime.\n");
    return EXIT_FAILURE;
  }

  c_table_free(conf_table);

  return 1;
}

int main(int argc, char **argv) {
// Free up the extra console initialized with programs in windows
#if defined(__MINGW32__)
#if !defined(DEBUG_OUTPUT)
  FreeConsole();
#endif
#endif
  dbg_enable_log(0, "log.txt");
  c_parse_args(argc, argv);

  init_sys();

  s_timer frame_delta;
  s_timer input_delta;
  s_timer audio_delta;
  s_timer render_delta;
  s_timer game_delta;
  s_timer overall_delta;

  s_timer_create(&frame_delta);
  s_timer_create(&input_delta);
  s_timer_create(&audio_delta);
  s_timer_create(&render_delta);
  s_timer_create(&game_delta);
  s_timer_create(&overall_delta);

  while (!r_window_should_close() && !d_fatal) {
    s_timer_update(&overall_delta);
    s_timer_update(&input_delta);

    i_update();
    glfwPollEvents();
    g_input(input_delta.delta);

    if (a_can_play()) {
      s_timer_update(&audio_delta);
      a_update(audio_delta.delta);
    }

    if (r_allow_render()) {
      s_timer_update(&render_delta);
      g_frame_start(render_delta.delta);

      r_update();
      g_render(render_delta.delta);
      r_end();

      g_frame_end(render_delta.delta);
      r_window_swap_buffers();
    }

    s_timer_update(&game_delta);
    g_update(game_delta.delta);

    s_timer_update(&overall_delta);
    // t_update(&frame_delta);

    if (!r_is_vsync()) {
      //      frame_sample_push(n_time_frame);
      int proj_fps = MS_TO_SEC / overall_delta.delta;
      if (proj_fps >= max_fps) {
        time_s max_timeframe = MS_TO_SEC / max_fps;
        max_timeframe -= overall_delta.delta;

        // Sleep the remainder of the timeframe
        s_sleep(max_timeframe);
      } else if (proj_fps < target_fps && proj_fps > 0) {
        target_fps = proj_fps;
      } else {
        target_fps = 1;
      }
    }
  }

  g_exit();
  r_exit();
  a_exit();

  if (d_fatal) {
    dbg_post_to_err();
  } else {
    dbg_cleanup();
  }

  return EXIT_SUCCESS;
}

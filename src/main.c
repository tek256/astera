#define _POSIX_C_SOURCE 199309L

// Define POSIX features for C99
#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
#if defined(__cplusplus)
#define _XOPEN_SOURCE 700 /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#elif __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 700 /* SUS v4, POSIX 1003.1 2008/13 (POSIX 2008/13) */
#else
#define _XOPEN_SOURCE 500 /* SUS v2, POSIX 1003.1 1997 */
#endif                    /* __STDC_VERSION__ */
#endif                    /* !_XOPEN_SOURCE && !_POSIX_C_SOURCE */

#include "config.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#endif

  c_table conf_table;

  if (c_has_prefs()) {
    target_conf_path = c_get_pref_p();
  }

  if (target_conf_path) {
    asset_t *conf_file = asset_get(0, target_conf_path);
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

#if defined(WINDOW_TITLE)
  info.title = WINDOW_TITLE;
#else
  info.title = "untitled";
#endif

  if (!r_init(info)) {
    _fatal("Unable to initialize rendering system.\n");
    return EXIT_FAILURE;
  }

  if (!a_init(conf.master, conf.sfx, conf.music)) {
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

  double timeframe = MS_PER_SEC / (double)target_fps;
  double curr = t_get_time();
  double last = curr;
  double check;

  double delta;
  double accum = timeframe;
  int vsync = 1;

  while (!r_window_should_close() && !d_fatal) {
    last = curr;
    curr = t_get_time();
    delta = curr - last;
    accum = timeframe;

    i_update();
    glfwPollEvents();
    g_input(delta);

    if (a_allow_play()) {
      a_update(delta);
    }

    if (r_allow_render()) {
      g_frame_start(delta);

      r_update(delta);
      g_render(delta);
      r_end();

      g_frame_end(delta);
      r_window_swap_buffers();
    }

    g_update(delta);

    check = t_get_time();
    accum = (long)(check - curr);

    double n_time_frame = timeframe;
    int t_fps;
    int l_fps = target_fps;

    if (!vsync) {
      if (accum > 0) {
        n_time_frame -= accum;
        t_fps = (int)((double)MS_PER_SEC / n_time_frame);

        if (t_fps > max_fps) {
          t_fps = max_fps;
        } else if (t_fps > 0) {
          target_fps = t_fps;
        }

        timeframe = (double)(MS_PER_SEC / (double)(target_fps));

        struct timespec sleep_req, sleep_rem;
        sleep_req.tv_sec = 0;
        sleep_req.tv_nsec = accum * NS_PER_MS;

        nanosleep(&sleep_req, &sleep_rem);
      } else {
        n_time_frame += accum;
        t_fps = (int)((double)MS_PER_SEC / n_time_frame);

        if (t_fps < max_fps) {
          target_fps = max_fps;
        } else if (t_fps < 0) {
          target_fps = 1;
        } else {
          target_fps = t_fps;
        }

        timeframe = (double)(MS_PER_SEC / (double)(target_fps));
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

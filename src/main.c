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

#if defined(CONF_PATH)
  conf = c_parse_file(CONF_PATH, 1);
#else
  conf = c_defaults();
#endif

  info.width = conf.width;
  info.height = conf.height;
  info.fullscreen = conf.fullscreen;
  info.vsync = conf.vsync;
  info.borderless = conf.borderless;
  info.refreshRate = conf.refreshRate;
#if defined(WINDOW_TITLE)
  info.title = WINDOW_TITLE;
#else
  info.title = "demo";
#endif

  // TODO add icon baking support
  info.icon = NULL;

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
      r_window_clear();
      r_update(delta);
      g_render(delta);
      r_end();
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

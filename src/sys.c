#include "sys.h"

#include <GLFW/glfw3.h>

#include <unistd.h>

#include "platform.h"

#if defined(PLAT_MSFT)
#include <windows.h>
#else
#include <time.h>
#endif

/* Returns time in milliseconds */
time_s s_get_time() { return glfwGetTime() * MS_TO_SEC; }

time_s s_timer_update(s_timer *t) {
  time_s current = s_get_time();
  t->delta = current - t->last;
  t->last = current;
  return t->delta;
}

s_timer s_timer_create() { return (s_timer){s_get_time(), 0}; }

time_s s_sleep(time_s duration) {
#ifdef WIN32
  Sleep(duration * MS_TO_MCS);
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = duration / MS_TO_SEC;
  ts.tv_nsec = fmod(duration, MS_TO_NS);
  nanosleep(&ts, NULL);
#else
  uint32_t sleep_conv = duration / MS_TO_MCS;
  usleep(sleep_conv);
#endif
  return duration;
}

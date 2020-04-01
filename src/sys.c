#include "sys.h"

#include <GLFW/glfw3.h>

#if defined(PLAT_UNIX) || defined(PLAT_LINUX) || defined(PLAT_BSD)
#include <unistd.h>
#endif

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

static char *s_reverse(char *string, int length) {
  int start = 0;
  int end = length - 1;

  char tmp_a, tmp_b;

  while (start < end) {
    tmp_a = string[start];
    tmp_b = string[end];

    string[end] = tmp_a;
    string[start] = tmp_b;

    ++start;
    --end;
  }

  return string;
}

char *s_itoa(int value, char *string, int base) {
  int i = 0;
  int negative = 0;

  if (value == 0) {
    string[i] = '0';
    ++i;
    string[i] = '\0';
    return string;
  }

  if (value < 0 && base == 10) {
    negative = 1;
    value = -value;
  }

  while (value != 0) {
    int remainder = value % base;
    string[i] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
    ++i;
    value = value / base;
  }

  if (negative) {
    string[i] = '-';
    ++i;
  }

  string[i] = '\0';
  s_reverse(string, i);

  return string;
}

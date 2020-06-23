#include <astera/sys.h>
#include <astera/debug.h>

// #include <GLFW / glfw3.h>
#include <math.h>

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || \
    defined(__APPLE__)
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <time.h>
#endif

#if !defined(ASTERA_NO_CONF)
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdint.h>
#if defined(__linux)
#define HAVE_POSIX_TIMER
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif
#elif defined(__APPLE__)
#define HAVE_MACH_TIMER
#include <mach/mach_time.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* Get time in nanoseconds from the Operating System's High performance timer */
static uint64_t s_get_ns() {
  static uint64_t is_init = 0;
#if defined(__APPLE__)
  static mach_timebase_info_data_t info;
  if (0 == is_init) {
    mach_timebase_info(&info);
    is_init = 1;
  }
  uint64_t now;
  now = mach_absolute_time();
  now *= info.numer;
  now /= info.denom;
  return now;
#elif defined(__linux)
  static struct timespec linux_rate;
  if (0 == is_init) {
    clock_getres(CLOCKID, &linux_rate);
    is_init = 1;
  }
  uint64_t        now;
  struct timespec spec;
  clock_gettime(CLOCKID, &spec);
  now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
  return now;
#elif defined(_WIN32)
  static LARGE_INTEGER win_frequency;
  if (0 == is_init) {
    QueryPerformanceFrequency(&win_frequency);
    is_init = 1;
  }
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return (uint64_t)((1e9 * now.QuadPart) / win_frequency.QuadPart);
#endif
}

/* Call the OS's sleep function for given milliseconds */
time_s s_sleep(time_s duration) {
#if defined(_WIN32) || defined(_WIN64)
  Sleep(duration * MS_TO_MCS);
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec  = duration / MS_TO_SEC;
  ts.tv_nsec = fmod(duration, MS_TO_NS);
  nanosleep(&ts, NULL);
#else
  uint32_t sleep_conv = duration / MS_TO_MCS;
  usleep(sleep_conv);
#endif
  return duration;
}

/* Returns time in milliseconds */
time_s s_get_time() { return s_get_ns() / NS_TO_MS; }

/* Update a timer with current time & calculate delta from last update */
time_s s_timer_update(s_timer* t) {
  time_s current = s_get_time();
  t->delta       = current - t->last;
  t->last        = current;
  return t->delta;
}

/* Create the timer structure with current time */
s_timer s_timer_create() { return (s_timer){s_get_time(), 0}; }

/* String reversal */
static char* s_reverse(char* string, uint32_t length) {
  int start = 0;
  int end   = length - 1;

  char tmp_a, tmp_b;

  while (start < end) {
    tmp_a = string[start];
    tmp_b = string[end];

    string[end]   = tmp_a;
    string[start] = tmp_b;

    ++start;
    --end;
  }

  return string;
}

/* Convert int to character */
char* s_itoa(int32_t value, char* string, int8_t base) {
  int i        = 0;
  int negative = 0;

  if (value == 0) {
    string[i] = '0';
    ++i;
    string[i] = '\0';
    return string;
  }

  if (value < 0 && base == 10) {
    negative = 1;
    value    = -value;
  }

  while (value != 0) {
    int remainder = value % base;
    string[i]     = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
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

#if !defined(ASTERA_NO_CONF)
void s_table_free(s_table table) {
  if (table.keys)
    free(table.keys);
  if (table.values)
    free(table.values);
}

static char* s_cleaned_str(const char* str, uint32_t* size, char* str_end) {
  if (!str) {
    ASTERA_DBG("Unable to clean null string.\n");
    return 0;
  }

  int str_size = 0;

  if (str_end) {
    while (isspace(*str) && str < str_end)
      str++;
  } else {
    while (isspace(*str))
      str++;
  }

  if (str == 0)
    return 0;

  const char* start = str;
  if (str_end) {
    while (!isspace(*str) && str < str_end) {
      ++str_size;
      str++;
    }
  } else {
    while (!*str) {
      if (!isspace(*str)) {
        ++str_size;
        str++;
      } else {
        break;
      }
    }
  }

  char* new_str = (char*)malloc(sizeof(char) * (str_size + 1));
  strncpy(new_str, start, str_size);
  new_str[str_size] = '\0';

  if (size)
    *size = str_size;

  return new_str;
}

static void s_kv_get(char* src, char** key, char** value, uint32_t* key_length,
                     uint32_t* value_length) {
  char* split = strstr(src, "=");

  int _key_len, _value_len;
  *key   = s_cleaned_str(src, &_key_len, split);
  *value = s_cleaned_str(split + 1, &_value_len, 0);

  if (key_length) {
    *key_length = _key_len;
  }

  if (value_length) {
    *value_length = _value_len;
  }
}

s_table s_table_get(unsigned char* data, uint32_t length) {
  char* data_ptr = (char*)data;
  char* line     = strtok(data_ptr, "\n");

  const char** keys          = (const char**)malloc(sizeof(char*) * 16);
  const char** values        = (const char**)malloc(sizeof(char*) * 16);
  int          line_capacity = 16;
  int          line_count    = 0;

  while (line != NULL) {
    char *key, *value;

    int key_length   = 0;
    int value_length = 0;

    if (line_count == line_capacity) {
      keys   = realloc(keys, sizeof(char*) * (line_capacity + 8));
      values = realloc(values, sizeof(char*) * (line_capacity + 8));
      line_capacity += 8;
    }

    s_kv_get(line, &key, &value, &key_length, &value_length);

    keys[line_count]   = key;
    values[line_count] = value;

    ++line_count;
    line = strtok(NULL, "\n");
  }

  return (s_table){keys, values, line_count};
}

uint8_t s_table_write(s_table* table, char* filepath) {
  FILE* f = fopen(filepath, "wb");
  if (!f) {
    return 0;
  }

  char   str_buff[128];
  int8_t str_size = 0;

  for (uint32_t i = 0; i < table->count; ++i) {
    str_size =
        snprintf(str_buff, 128, "%s = %s\n", table->keys[i], table->values[i]);
    uint32_t written = fwrite(str_buff, str_size, sizeof(char), f);
    if (written == 0) {
      fclose(f);
      return 0;
    }
    memset(str_buff, 0, sizeof(char) * str_size);
  }

  fwrite("\0", 1, sizeof(char), f);

  fclose(f);
  return 1;
}

uint8_t s_table_write_mem(s_table* table, unsigned char* dst,
                          uint32_t dst_length, uint32_t* write_length) {
  uint32_t size = 0;
  char     str_buff[128];
  uint8_t  str_size;

  for (uint32_t i = 0; i < table->count; ++i) {
    str_size =
        snprintf(str_buff, 128, "%s = %s\n", table->keys[i], table->values[i]);

    if (size + str_size > dst_length - 2) {
      return 0;
    }

    memcpy(&dst[size], str_buff, sizeof(char) * str_size);
    memset(str_buff, 0, sizeof(char) * str_size);
    size += str_size;
  }

  char* end = &dst[size];
  *end      = '\0';

  if (write_length) {
    *write_length = size;
  }

  return 1;
}
#endif

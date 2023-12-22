#include <astera/sys.h>
#include <astera/debug.h>

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

#include <stdlib.h>
#if !defined(ASTERA_NO_CONF)
#include <ctype.h>
#include <stdio.h>
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
time_s s_get_time() {
  return s_get_ns() / NS_TO_MS;
}

/* Update a timer with current time & calculate delta from last update */
time_s s_timer_update(s_timer* t) {
  time_s current = s_get_time();
  t->delta       = current - t->last;
  t->last        = current;
  return t->delta;
}

/* Create the timer structure with current time */
s_timer s_timer_create() {
  return (s_timer){s_get_time(), 0};
}

/* String reversal */
static char* s_reverse(char* string, uint32_t length) {
  uint32_t start = 0;
  uint32_t end   = length - 1;

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
  uint32_t i        = 0;
  uint8_t  negative = 0;

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
void s_table_free(s_table* table) {
  if (table->keys) {
    free(table->keys);
  }
  if (table->values) {
    free(table->values);
  }

  if (table->data && table->data_owned) {
    free((char*)table->data);
  }
}

/* In place clean a string (removes leading & trailing whitespace)
 * str - string to clean
 * n_size - new size
 * str_end - the end of the string
 * returns: new start pointer for string */
static char* s_clean_str(char* str, uint32_t* n_size, char* str_end) {
  if (!str) {
    ASTERA_FUNC_DBG("Unable to clean null string\n");
    return 0;
  }

  uint32_t str_size = 0;
  if (str_end) {
    while (isspace(*str) && str < str_end) {
      str++;
    }
  } else {
    while (isspace(*str)) {
      str++;
    }
  }

  if (str == 0) {
    return 0;
  }

  char* start = str;
  if (str_end) {
    while (!isspace(*str) && str < str_end) {
      ++str_size;
      str++;
    }
  } else {
    while (*str) {
      if (!isspace(*str)) {
        ++str_size;
        str++;
      } else {
        break;
      }
    }
  }

  if (n_size) {
    *n_size = str_size;
  }

  return start;
}

static void s_kv_get(char* src, char** key, char** value, uint32_t* key_length,
                     uint32_t* value_length) {
  uint32_t str_len = (uint32_t)strlen(src);
  char*    split   = strstr(src, "=");

  uint32_t _key_len, _value_len;
  *key   = s_clean_str(src, &_key_len, split);
  *value = s_clean_str(split + 1, &_value_len, src + str_len);

  char* term     = *key;
  term[_key_len] = 0;

  if (key_length) {
    *key_length = _key_len;
  }

  if (value_length) {
    *value_length = _value_len;
  }
}

s_table s_table_get(unsigned char* data) {
  char* data_ptr = (char*)data;
  char* line     = strtok(data_ptr, "\n");

  const char** keys          = (const char**)calloc(16, sizeof(char*));
  const char** values        = (const char**)calloc(16, sizeof(char*));
  uint32_t     line_capacity = 16;
  uint32_t     line_count    = 0;

  while (line != NULL) {
    char *key, *value;

    uint32_t key_length   = 0;
    uint32_t value_length = 0;

    if (line_count == line_capacity) {
      keys = (const char**)realloc(keys, sizeof(char*) * (line_capacity + 8));
      values =
          (const char**)realloc(values, sizeof(char*) * (line_capacity + 8));
      line_capacity += 8;
    }

    s_kv_get(line, &key, &value, &key_length, &value_length);

    keys[line_count]   = key;
    values[line_count] = value;

    ++line_count;
    line = strtok(NULL, "\n");
  }

  return (s_table){.keys          = keys,
                   .values        = values,
                   .count         = line_count,
                   .capacity      = line_capacity,
                   .allow_grow    = 0,
                   .data          = (const char*)data,
                   .data_cursor   = 0,
                   .data_capacity = (uint32_t)strlen((const char*)data),
                   .data_owned    = 0};
}

s_table s_table_create(uint32_t data_cap, uint32_t capacity,
                       uint8_t allow_grow) {
  s_table table = (s_table){0};

  table.data          = (char*)calloc(data_cap, sizeof(char));
  table.keys          = (const char**)calloc(capacity, sizeof(char**));
  table.values        = (const char**)calloc(capacity, sizeof(char**));
  table.count         = 0;
  table.capacity      = capacity;
  table.allow_grow    = allow_grow;
  table.data_cursor   = 0;
  table.data_capacity = data_cap;
  table.data_owned    = 1;

  return table;
}

static char conv_buffer[32] = {0};
uint8_t     s_table_add_float(s_table* table, char* key, float value) {
  memset(conv_buffer, 0, sizeof(char) * 32);
  uint32_t key_len = (uint32_t)strlen(key);
  int      len     = snprintf(conv_buffer, 32, "%.2f", value);
  uint8_t  table_add =
      s_table_add(table, key, key_len, conv_buffer, (uint32_t)len);
  return table_add;
}

uint8_t s_table_add_int(s_table* table, char* key, int value) {
  memset(conv_buffer, 0, sizeof(char) * 32);
  uint32_t key_len = (uint32_t)strlen(key);
  int      len     = snprintf(conv_buffer, 32, "%i", value);
  uint8_t  table_add =
      s_table_add(table, key, key_len, conv_buffer, (uint32_t)len);
  return table_add;
}

uint8_t s_table_add(s_table* table, char* key, uint32_t key_len, char* value,
                    uint32_t value_len) {
  if (key && value) {
    // TODO dynamically rebuild array of string ptrs
    if (table->data_cursor + key_len + value_len > table->data_capacity) {
      return 0;
    }

    if (table->count == table->capacity - 1) {
      if (table->allow_grow) { // at max capacity and can grow
        table->keys = (const char**)realloc(
            table->keys, sizeof(char**) * (table->count + 1));

        table->values = (const char**)realloc(
            table->values, sizeof(char**) * (table->count + 1));

        ++table->capacity;
      } else { // at max capacity and not allowed to grow
        return 0;
      }
    }

    // add to data
    char* n_key   = (char*)&table->data[table->data_cursor];
    char* n_value = n_key + (sizeof(char) * (strlen(key) + 1));

    strncpy(n_key, key, sizeof(char) * key_len);
    n_key[key_len] = 0;
    strncpy(n_value, value, sizeof(char) * value_len);
    n_value[value_len] = 0;

    table->data_cursor += key_len + value_len + 2;

    // add new key/value pair
    table->keys[table->count]   = n_key;
    table->values[table->count] = n_value;

    ++table->count;

    return 1;
  }

  return 0;
}

uint8_t s_table_remove(s_table* table, char* key) {
  uint8_t found = 0;
  for (uint32_t i = 0; i < table->count - 1; ++i) {
    if (!found) {
      if (!strcmp(table->keys[i], key)) {
        found = 1;
      }
    }
    if (found) {
      table->keys[i]   = table->keys[i + 1];
      table->values[i] = table->values[i + 1];
    }
  }

  if (!strcmp(table->keys[table->count], key)) {
    found = 1;
  }

  if (found) {
    table->keys =
        (const char**)realloc(table->keys, sizeof(char**) * (table->count - 1));

    table->values = (const char**)realloc(table->values,
                                          sizeof(char**) * (table->count - 1));
    --table->count;
  }

  return found;
}

const char* s_table_find(s_table* table, char* key) {
  for (uint32_t i = 0; i < table->count; ++i) {
    if (!strcmp(table->keys[i], key)) {
      return table->values[i];
    }
  }
  return 0;
}

uint8_t s_table_write(s_table* table, char* filepath) {
  FILE* f = fopen(filepath, "wb");
  if (!f) {
    return 0;
  }

  char   str_buff[128] = {0};
  int8_t str_size      = 0;

  for (uint32_t i = 0; i < table->count; ++i) {
    // skip empty slots in the table
    if (!table->keys[i] && !table->values[i])
      continue;

    // add new line to it
    str_size =
        snprintf(str_buff, 128, "%s = %s\n", table->keys[i], table->values[i]);
    // write new line
    uint32_t written = (uint32_t)fwrite(str_buff, str_size, sizeof(char), f);

    // if error exit
    if (written == 0) {
      fclose(f);
      return 0;
    }

    // clear out str_buff
    memset(str_buff, 0, sizeof(char) * 128);
  }

  fwrite("\0", 1, sizeof(char), f);

  fclose(f);
  return 1;
}

uint8_t s_table_write_mem(s_table* table, unsigned char* dst,
                          uint32_t dst_length, uint32_t* write_length) {
  uint32_t size = 0;
  char     str_buff[128];
  uint32_t str_size;

  for (uint32_t i = 0; i < table->count; ++i) {
    str_size = (uint32_t)snprintf(str_buff, 128, "%s = %s\n", table->keys[i],
                                  table->values[i]);

    if (size + str_size > dst_length - 2) {
      return 0;
    }

    memcpy(&dst[size], str_buff, sizeof(char) * str_size);
    memset(str_buff, 0, sizeof(char) * str_size);
    size += str_size;
  }

  char* end = (char*)&dst[size];
  *end      = '\0';

  if (write_length) {
    *write_length = size;
  }

  return 1;
}

void s_table_print(s_table* table) {
  for (uint32_t i = 0; i < table->count; ++i) {
    printf("[%i] %s: %s\n", i, table->keys[i], table->values[i]);
  }
}

uint32_t s_strnify(char* dst, uint32_t dst_capacity, const char* src,
                   uint32_t str_length) {
  uint32_t index = 0;

  if (str_length == 0)
    str_length = (uint32_t)strlen(src);

  for (uint32_t i = 0; i < str_length; ++i) {
    if (index >= dst_capacity - 1) {
      break;
    }

    if (isspace(*src)) {
      if (index + 3 >= dst_capacity) {
        break;
      }

      strcat(dst, "%20");
      index += 3;
      dst += 3;
    } else {
      *dst = *src;
      ++index;
      ++dst;
    }
    ++src;
  }

  dst[index] = 0;

  return index;
}

uint32_t s_destrnify(char* dst, uint32_t dst_capacity, const char* src,
                     uint32_t str_length) {
  uint32_t index = 0, length = 0;

  if (str_length == 0)
    str_length = (uint32_t)strlen(src);

  while (*src && index < str_length && length < dst_capacity) {
    if (strstr(src, "%20") == src) {
      *dst = ' ';
      ++dst;
      src += 3;
      index += 3;
    } else {
      *dst = *src;
      ++dst;
      ++src;
      ++index;
    }

    ++length;
  }

  dst[length] = 0;

  return length;
}

s_buffer_t s_buff_create(uint32_t capacity) {
  char* data = (char*)calloc(capacity, sizeof(char));

  return (s_buffer_t){
      .data     = data,
      .cursor   = data,
      .length   = 0,
      .capacity = capacity,
  };
}

s_buffer_t s_buff_read(const char* data) {
  if (!data)
    return (s_buffer_t){0};

  uint32_t length = strlen(data);

  s_buffer_t buff = s_buff_create(length + 1);
  strcpy(buff.data, data);
  buff.length = length;

  return buff;
}

void s_buff_destroy(s_buffer_t* buffer) {
  free(buffer->data);
}

void s_buff_rewind(s_buffer_t* buffer) {
  buffer->cursor = buffer->data;
}

static void s_buffer_to_space(s_buffer_t* buffer) {
  while (*buffer->cursor) {
    if (isspace(*buffer->cursor)) {
      ++buffer->cursor;

      return;
    }
    ++buffer->cursor;
  }
}

void s_buff_move_to(s_buffer_t* buffer, uint32_t index) {
  buffer->cursor = &buffer->data[index];
}

uint8_t s_buff_add_s(s_buffer_t* buffer, const char* string) {
  if (buffer->length == buffer->capacity) {
    return 0;
  }

  uint32_t len = (uint32_t)strlen(string);

  if (buffer->capacity - buffer->length < len) {
    return 0;
  }

  int length = snprintf(buffer->cursor, buffer->capacity - buffer->length,
                        "%s ", string);

  // Move cursor forward
  buffer->cursor += length;
  buffer->length += (uint32_t)length;

  return 1;
}

uint8_t s_buff_add_i(s_buffer_t* buffer, int value) {
  if (buffer->length == buffer->capacity) {
    return 0;
  }

  int length =
      snprintf(buffer->cursor, buffer->capacity - buffer->length, "%i ", value);

  // Move cursor forward
  buffer->cursor += length;
  buffer->length += (uint32_t)length;

  return 1;
}

uint8_t s_buff_add_f(s_buffer_t* buffer, float value) {
  if (buffer->length == buffer->capacity) {
    return 0;
  }

  int length =
      snprintf(buffer->cursor, buffer->capacity - buffer->length, "%f ", value);

  // Move cursor forward
  buffer->cursor += length;
  buffer->length += (uint32_t)length;

  return 1;
}

float s_buff_get_f(s_buffer_t* buffer) {
  if (buffer->length == 0)
    return 0;

  float value;
  if (!sscanf(buffer->cursor, "%f ", &value)) {
    return -1.f;
  }

  s_buffer_to_space(buffer);

  return value;
}

int s_buff_get_i(s_buffer_t* buffer) {
  if (buffer->length == 0)
    return 0;

  int value;
  if (!sscanf(buffer->cursor, "%i ", &value)) {
    return -1;
  }

  s_buffer_to_space(buffer);

  return value;
}

char* s_buff_get_s(s_buffer_t* buffer, char* dst) {
  if (buffer->length == 0)
    return 0;

  if (!sscanf(buffer->cursor, "%s ", dst)) {
    return 0;
  }

  s_buffer_to_space(buffer);

  return dst;
}

#endif

#include <astera/debug.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__linux__) || defined(__unix__) || defined(__FreeBSD__) || \
    defined(__APPLE__)
#include <unistd.h>
#endif

d_ctx* d_ctx_get() { return &_d_ctx; }

void d_set_log(uint8_t log, const char* fp) {
  if (log) {
    _d_ctx.logging = 1;
    if (fp)
      _d_ctx.log_fp = fp;
  } else {
    _d_ctx.logging = 0;
  }
}

void d_set_log_fp(const char* fp) {
  if (fp)
    _d_ctx.log_fp = fp;
}

void d_use_timestamp(uint8_t timestamp) { _d_ctx.timestamp = timestamp; }

uint8_t d_is_logging() { return _d_ctx.logging; }

/* Get the current timestamp and put it in the timebuff */
static void d_get_timestamp() {
  time_t     raw;
  struct tm* ti;
  time(&raw);
  ti = localtime(&raw);
  memset(_d_ctx.time_buff, 0, sizeof(char) * 16);
  strftime(_d_ctx.time_buff, 16, "%d%Om%Y_%H%M", ti);
}

uint8_t d_post_to_err() {
  if (!_d_ctx.logging) {
    return 0;
  }

  // Get the current timestamp
  d_get_timestamp();

  static char err_buff[64];
  snprintf(err_buff, 64, "ERR_%s.txt", _d_ctx.time_buff);

  FILE* o = fopen(err_buff, "wb+");
  FILE* i = fopen(_d_ctx.log_fp, "rb+");

  if (!o || !i) {
    if (!o)
      fprintf(stderr, "Unable to open, %s for error output.\n", err_buff);
    else
      fprintf(stderr, "Unable to open: %s for error output\n", _d_ctx.log_fp);
    fclose(o);
    fclose(i);
    memset(err_buff, 0, sizeof(char) * 64);
    return 0;
  }

  fseek(i, 0, SEEK_END);
  long size = ftell(i);
  long rem  = size;
  rewind(i);

  while (rem > 0) {
    rem -= fread(err_buff, sizeof(char), 64, i);
    fwrite(err_buff, sizeof(char), 64, o);
  }

  // Clear out the data of the error buffer
  memset(err_buff, 0, sizeof(char) * 64);

  fclose(i);
  fclose(o);

  return 1;
}

uint8_t d_cleanup() {
  if (_d_ctx.logging && _d_ctx.log_fp) {
    return remove(_d_ctx.log_fp);
  }

  return 1;
}

// NOTE: Isn't working perfectly with internal astera logging output, try
// ASTERA_DEBUG_STDIO if you want this to work
void _l(const char* format, ...) {
  va_list args;
  va_start(args, format);

  if (!_d_ctx.silent) {
    vfprintf(stdout, format, args);
  }

  if (_d_ctx.logging) {
    FILE* f = fopen(_d_ctx.log_fp, "ab+");

    if (!f) {
      return;
    }

    if (_d_ctx.timestamp) {
      d_get_timestamp();
      fprintf(f, "%s: ", _d_ctx.time_buff);
    }

    vfprintf(f, format, args);
    fclose(f);
  }

  va_end(args);
}

void _e(const char* format, ...) {
  va_list args;
  va_start(args, format);

  if (!_d_ctx.silent) {
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
  }

  if (_d_ctx.logging) {
    FILE* f = fopen(_d_ctx.log_fp, "ab+");

    if (!f) {
      return;
    }

    if (_d_ctx.timestamp) {
      d_get_timestamp();
      fprintf(f, "%s ", _d_ctx.time_buff);
    }

    vfprintf(f, format, args);
  }

  va_end(args);
}

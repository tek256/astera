#ifndef ASTERA_DEBUG_HEADER
#define ASTERA_DEBUG_HEADER

#define ASTERA_DEBUG_INCLUDED

/* Macro Options
 * ASTERA_DEBUG_OUTPUT - Enable the debug output macro (configured with
 *                       cmake build type) */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
  uint8_t     silent, logging, timestamp;
  const char* log_fp;
  char        time_buff[16];
} d_ctx;

// Create the global context to use, and initialize it
static d_ctx _d_ctx;

/* Get a pointer to the debug context paramters
 * returns: pointer to the debug context */
d_ctx* d_ctx_get();

/* Disable / Enable & set usage of outputting the log to file
 *  log - 1 = use log, 0 = don't
 *  fp - the filepath (optional if disabling) */
void d_set_log(uint8_t log, const char* fp);

/* Set the file to use for log output
 * fp - the file path*/
void d_set_log_fp(const char* fp);

/* Enable / disable usage of the timestamp
 * timestamp - use timestamp, 0 = don't */
void d_use_timestamp(uint8_t timestamp);

/* If we're outputting the log to file
 * returns: logging = 1, not logging = 0 */
uint8_t d_is_logging();

/* Post the output file to a unique timestamped error file
 * returns: success = 1, failure = 0 */
uint8_t d_post_to_err();

/* Remove any files created by runtime (except for error files)
 * returns: success = 1, failure = 0 */
uint8_t d_cleanup();

void _l(const char* format, ...);
void _e(const char* format, ...);

// TODO fix this
#if defined(ASTERA_DEBUG_OUTPUT) && !defined(_MSC_VER)
#define ASTERA_DBG(fmt, ...) _l(fmt, ##__VA_ARGS__)

#define ASTERA_FUNC_DBG(fmt, ...) \
  _l("%s: ", __FUNCTION__);       \
  _l(fmt, ##__VA_ARGS__);
#else
#define ASTERA_DBG(fmt, ...)
#define ASTERA_FUNC_DBG(fmt, ...)
#endif

// EOF
#endif

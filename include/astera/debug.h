#ifndef ASTERA_DEBUG_HEADER
#define ASTERA_DEBUG_HEADER

#define ASTERA_DEBUG_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/export.h>

int d_fatal;

ASTERA_API void dbg_enable_log(int log, const char* fp);
ASTERA_API void dbg_set_log_fp(const char* fp);

ASTERA_API void dbg_set_timestamp(int enabled);
ASTERA_API int  dbg_is_logging();
ASTERA_API int  dbg_post_to_err();

ASTERA_API int dbg_cleanup();

ASTERA_API void _fatal(const char* format, ...);
ASTERA_API void _l(const char* format, ...);
ASTERA_API void _e(const char* format, ...);

#ifdef __cplusplus
}
#endif
#endif


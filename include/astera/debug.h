#ifndef ASTERA_DEBUG_HEADER
#define ASTERA_DEBUG_HEADER

#define ASTERA_DEBUG_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int d_fatal;

void dbg_enable_log(int log, const char* fp);
void dbg_set_log_fp(const char* fp);

void dbg_set_timestamp(int enabled);
int  dbg_is_logging();
int  dbg_post_to_err();

int dbg_cleanup();

void _fatal(const char* format, ...);
void _l(const char* format, ...);
void _e(const char* format, ...);

#ifdef __cplusplus
}
#endif
#endif


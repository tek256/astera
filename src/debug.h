#ifndef DEBUG_H
#define DEBUG_H

#define DBG_VAR_LEN 16 

#include "types.h"

typedef struct {
    const char name[DBG_VAR_LEN];
    char* value;
    int r, c;
} dbg_var;

int d_fatal;

void dbg_enable_log(int log, const char* fp);
void dbg_set_log_fp(const char* fp);

void dbg_set_timestamp(int enabled);
int  dbg_is_logging();
int  dbg_post_to_err();

int  dbg_cleanup();

void _fatal(const char* format, ...);
void _l(const char* format, ...);
void _e(const char* format, ...);

#endif

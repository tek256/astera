#ifndef DEBUG_H
#define DEBUG_H

#define DBG_VAR_LEN 16 

typedef struct {
    const char name[DBG_VAR_LEN];
    char* value;
    int r, c;
} dbg_var;

int  dbg_cleanup();
void dbg_enable_log(int log);
int  dbg_get_logging();
void dbg_set_log_fp(const char* fp);
int dbg_log(const char* format, ...);
int dbg_post_to_err();
#endif

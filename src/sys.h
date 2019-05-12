#ifndef SYS_H
#define SYS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define TIMER_NAME_LENGTH 8
#define TIMER_MAX_COUNT 32

#define MS_PER_SEC 1e3
#define NS_PER_MS  1e6
#define NS_PER_SEC 1e9

#define SEC_PER_MS 1e-3
#define MS_PER_NS  1e-6
#define SEC_PER_NS 1e-9

typedef double time_s;

typedef struct timer_s {
    char name[TIMER_NAME_LENGTH];
    time_s timestamp;
} timer_s;

static timer_s t_timers[TIMER_MAX_COUNT];
static int     t_timer_count = 0;

void sys_log(const char* format, ...);
void sys_err(const char* format, ...);
void sys_warn(const char* format, ...); 

time_s t_get_time();
time_s t_get_time_since(time_s t);
time_s t_get_time_until(time_s t);
time_s t_get_time_diff(time_s t);
int    t_is_time_before(time_s a, time_s b);
int    t_is_time_after(time_s a,time_s b);

int   t_cmp_timer_names(const char a[TIMER_NAME_LENGTH], const char b[TIMER_NAME_LENGTH]);
void   t_cpy_timer_names(char dst[TIMER_NAME_LENGTH], const char src[TIMER_NAME_LENGTH]);

timer_s* t_get_timer(const char name[TIMER_NAME_LENGTH]);
void     t_end_timer(const char name[TIMER_NAME_LENGTH]);
void     t_mark_timer(const char name[TIMER_NAME_LENGTH]);
void     t_mark_ttimer(const char name[TIMER_NAME_LENGTH], time_s timestamp);
time_s   t_get_timer_since(const char name[TIMER_NAME_LENGTH]);
time_s   t_get_timer_until(const char name[TIMER_NAME_LENGTH]);
time_s   t_get_timer_diff(const char name[TIMER_NAME_LENGTH]);

inline time_s t_get_timer_time(const timer_s* t){
    return t->timestamp;
}

void     t_end_timert(timer_s* t);
void     t_mark_timert(timer_s* t);
void     t_mark_ttimert(timer_s* t, time_s timestamp);
time_s   t_get_timert_since(timer_s* t);
time_s   t_get_timert_until(timer_s* t);
time_s   t_get_timert_diff(timer_s* t);

#endif

#ifndef SYS_H
#define SYS_H

#include <stdio.h>

#include "config.h"
#include "platform.h"

#define TIMER_NAME_LENGTH 8
#define TIMER_MAX_COUNT 32

#define MS_PER_MIN MS_PER_SEC * 60
#define MS_PER_HR MS_PER_MIN * 60

#define MS_PER_SEC 1e3
#define NS_PER_MS 1e6
#define NS_PER_SEC 1e9

#define SEC_PER_MS 1e-3
#define MS_PER_NS 1e-6
#define SEC_PER_NS 1e-9

typedef double time_s;

typedef struct {
  char name[TIMER_NAME_LENGTH];
  time_s timestamp;
} timer_s;

static timer_s t_timers[TIMER_MAX_COUNT];
static s32 t_timer_count = 0;

time_s t_get_time();
time_s t_get_time_since(time_s t);
time_s t_get_time_until(time_s t);
time_s t_get_time_diff(time_s t);
s32 t_is_time_before(time_s a, time_s b);
s32 t_is_time_after(time_s a, time_s b);

timer_s *t_get_timer(const char name[TIMER_NAME_LENGTH]);
void t_end_timer(const char name[TIMER_NAME_LENGTH]);
void t_mark_timer(const char name[TIMER_NAME_LENGTH]);
void t_mark_ttimer(const char name[TIMER_NAME_LENGTH], time_s timestamp);
time_s t_get_timer_since(const char name[TIMER_NAME_LENGTH]);
time_s t_get_timer_until(const char name[TIMER_NAME_LENGTH]);
time_s t_get_timer_diff(const char name[TIMER_NAME_LENGTH]);

inline time_s t_get_timer_time(const timer_s *t) { return t->timestamp; }

void t_end_timert(timer_s *t);
void t_mark_timert(timer_s *t);
void t_mark_ttimert(timer_s *t, time_s timestamp);
time_s t_get_timert_since(timer_s *t);
time_s t_get_timert_until(timer_s *t);
time_s t_get_timert_diff(timer_s *t);

#endif

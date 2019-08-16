#ifndef SYS_H
#define SYS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <pthread.h>

#include "platform.h"

#define TIMER_NAME_LENGTH 8
#define TIMER_MAX_COUNT 32

#define MS_PER_SEC 1e3
#define NS_PER_MS  1e6
#define NS_PER_SEC 1e9

#define SEC_PER_MS 1e-3
#define MS_PER_NS  1e-6
#define SEC_PER_NS 1e-9

#define S_MAX_THREADS 6

typedef double time_s;

typedef struct  {
    char name[TIMER_NAME_LENGTH];
    time_s timestamp;
} timer_s;

typedef struct s_thread_state s_thread_state;
struct s_thread_state {
	u32 uid;
	char desc[8];
	u32 state;
	u32 running : 1;
	u32 await : 1;
	void (*func_ptr)(void*);
	pthread_t thread;
};

static s_thread_state s_threads[S_MAX_THREADS];
static u32 s_thread_count;

static timer_s t_timers[TIMER_MAX_COUNT];
static s32     t_timer_count = 0;

s_thread_state* s_spin_thread(void(*function)(void*), char desc[8]); 
s32 s_stop_thread(u32 uid);
s32 s_force_quit_thread(u32 uid);

time_s t_get_time();
time_s t_get_time_since(time_s t);
time_s t_get_time_until(time_s t);
time_s t_get_time_diff(time_s t);
s32    t_is_time_before(time_s a, time_s b);
s32    t_is_time_after(time_s a,time_s b);

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

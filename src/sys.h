#ifndef SYS_H
#define SYS_H

#define SEC_TO_NS 1e-9
#define SEC_TO_MCS 1e-6
#define SEC_TO_MS 1e-6
#define SEC_TO_MIN 60
#define SEC_TO_HOUR 3600

#define MS_TO_NS 1e-6
#define MS_TO_MCS 1e-3
#define MS_TO_SEC 1e3
#define MS_TO_MIN MS_TO_SEC * 60
#define MS_TO_HOUR MS_TO_MIN * 60

#define MCS_TO_NS 1e-3
#define MCS_TO_MS 1e3
#define MCS_TO_SEC 1e6
#define MCS_TO_MIN MCS_TO_SEC * 60
#define MCS_TO_HOUR MCS_TO_MIN * 60

#define NS_TO_MCS 1e3
#define NS_TO_MS 1e6
#define NS_TO_SEC 1e9
#define NS_TO_MIN NS_TO_SEC * 60
#define NS_TO_HOUR NS_TO_MIN * 60

typedef double time_s;

typedef struct {
  time_s last;
  time_s delta;
} s_timer;

/* Returns time in Milliseconds
 */
time_s s_get_time();

time_s s_timer_update(s_timer *t);
s_timer s_timer_create();

time_s s_sleep(time_s duration);

#endif

#ifndef ASTERA_SYS_HEADER
#define ASTERA_SYS_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/export.h>

#define SEC_TO_NS   1e-9
#define SEC_TO_MCS  1e-6
#define SEC_TO_MS   1e-6
#define SEC_TO_MIN  60
#define SEC_TO_HOUR 3600

#define MS_TO_NS   1e-6
#define MS_TO_MCS  1e-3
#define MS_TO_SEC  1e3
#define MS_TO_MIN  MS_TO_SEC * 60
#define MS_TO_HOUR MS_TO_MIN * 60

#define MCS_TO_NS   1e-3
#define MCS_TO_MS   1e3
#define MCS_TO_SEC  1e6
#define MCS_TO_MIN  MCS_TO_SEC * 60
#define MCS_TO_HOUR MCS_TO_MIN * 60

#define NS_TO_MCS  1e3
#define NS_TO_MS   1e6
#define NS_TO_SEC  1e9
#define NS_TO_MIN  NS_TO_SEC * 60
#define NS_TO_HOUR NS_TO_MIN * 60

#if !defined(ASTERA_SYS_LOWP_TIME)
typedef double time_s;
#else
typedef float time_s;
#endif

typedef struct {
  time_s last;
  time_s delta;
} s_timer;

/* Returns time in Milliseconds */
ASTERA_API time_s s_get_time();

/* Update the mark & delta for timer */
ASTERA_API time_s s_timer_update(s_timer* t);

/* Create a timer that is up to date */
ASTERA_API s_timer s_timer_create();

/* Sleep for time in Milliseconds */
ASTERA_API time_s s_sleep(time_s duration);

/* Convert integer to String */
ASTERA_API char* s_itoa(int value, char* string, int base);

#ifdef __cplusplus
}
#endif
#endif

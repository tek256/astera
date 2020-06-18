#ifndef ASTERA_SYS_HEADER
#define ASTERA_SYS_HEADER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
  /* last - the last time this struct was updated
     delta - the time between the last update and the update before that */
  time_s last;
  time_s delta;
} s_timer;

#if !defined(ASTERA_NO_CONF)
typedef struct {
  /* keys - an array of the key strings
     values - an array of the value strings
     count - the number of strings (length of each array individually) */
  const char** keys;
  const char** values;
  int          count;
} s_table;

/* Parse data as a psuedo-ini loader
   data - the data of the file
   length - the length of the data array
   returns: formatted table struct */
s_table s_table_get(unsigned char* data, uint32_t length);

/* Free a table struct's contents
   table - the table you want to free */
void s_table_free(s_table table);

/* Write a psuedo INI file from a table
   table - the table to write to file
   filepath - the path of the file to write
   returns: 1 = success, 0 = fail */
uint8_t s_table_write(s_table* table, char* filepath);

/* Write a psuedo INI table to memory
   table - the table to write to file
   filepath - the path of the file to write
   returns: 1 = success, 0 = fail */
uint8_t s_table_write_mem(s_table* table, unsigned char* data,
                          uint32_t dst_length, uint32_t* write_length);

#endif

/* Get the current time in milliseconds */
time_s s_get_time();

/* Update the mark & delta for timer
   t - the timer to update
   returns: the delta from last update of the time */
time_s s_timer_update(s_timer* t);

/* Create a timer that is up to date
   returns: a formatted s_timer marked with current time */
s_timer s_timer_create();

/* Sleep for time in Milliseconds
   duration - the time in milliseconds to sleep
   returns: time actually slept */
time_s s_sleep(time_s duration);

/* Convert integer to String
   value - the value to convert to string
   string - the storage for the string
   base - the base to set the value to
   returns: pointer to the string */
char* s_itoa(int32_t value, char* string, int8_t base);

#ifdef __cplusplus
}
#endif
#endif

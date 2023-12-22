// TODO:
// - Multi iterator to parse duplicate keys
// - Threading
// - System Info

/* MACROS:
 * ASTERA_NO_CONF - Remove INI Loading functionality
 * ASTERA_LOWP_TIME - Use single precision floats for time */

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

/* String based data input/output*/
typedef struct {
  char *   data, *cursor;
  uint32_t length, capacity;
} s_buffer_t;

#if !defined(ASTERA_NO_CONF)
typedef struct {
  /* data - the raw data array/string
     keys - an array of the key strings
     values - an array of the value strings
     count - the number of strings (length of each array individually)
     capacity - the max number of strings in the arrays
     allow_grow - if to allow for array growth
     data_owned - if we own the allocated data (have to free it)*/
  const char*  data;
  const char** keys;
  const char** values;
  uint32_t     count, capacity, data_capacity, data_cursor;
  uint8_t      allow_grow, data_owned;
} s_table;

/* Parse data as a psuedo-ini loader
   data - the data of the file
   returns: formatted table struct */
s_table s_table_get(unsigned char* data);

/* Create a table with count
 * data_capacity - the max number of characteres to store
 * capacity - the max number of key/value pairs in array
 * allow_grow - if to allow for the table to auto grow when adding at capacity
 * returns: basic table structure */
s_table s_table_create(uint32_t data_capacity, uint32_t capacity,
                       uint8_t allow_grow);

/* Add a key/(int) value to a table
 * returns: 1 = success, 0 = fail */
uint8_t s_table_add_int(s_table* table, char* key, int value);

/* Add a key/(float) value to a table
 * returns: 1 = success, 0 = fail */
uint8_t s_table_add_float(s_table* table, char* key, float value);

/* Add a key/value to a table
 * returns: 1 = success, 0 = fail */
uint8_t s_table_add(s_table* table, char* key, uint32_t key_len, char* value,
                    uint32_t value_len);

/* Remove the first key matched from table
 * returns: 1 = success, 0 = fail */
uint8_t s_table_remove(s_table* table, char* key);

/* Returns value of a key if found
 * returns: value, 0 = fail */
const char* s_table_find(s_table* table, char* key);

/* Free a table struct's contents
   table - the table you want to free */
void s_table_free(s_table* table);

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

/* Output a table to stdout */
void s_table_print(s_table* table);

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

/* Convert a string to parsable string
 * dst - the destination to store the string
 * dst_capacity - the max amount of characters stored
 * src - the source string
 * str_length - the length of the source string
 * returns: length of string */
uint32_t s_strnify(char* dst, uint32_t dst_capacity, const char* src,
                   uint32_t str_length);

/* Convert a string back to regular non-strnified style
 * dst - the destination to store the string
 * dst_capacity - the max amount of characters stored
 * src - the source string
 * str_length - the length of the source string
 * returns: new length of string */
uint32_t s_destrnify(char* dst, uint32_t dst_capacity, const char* src,
                     uint32_t str_length);

/* Create string buffer with a size of capacity
 * capacity - the capacity of the buffer in characters
 * returns: formatted s_buffer_t struct */
s_buffer_t s_buff_create(uint32_t capacity);

/* Create a string buffer matching already existing data
 * data - the data to mirror
 * returns: formatted s_buffer_t struct */
s_buffer_t s_buff_read(const char* data);

/* Destroy a string buffer and all of its contents
 * buff - the buffer to destroy */
void s_buff_destroy(s_buffer_t* buffer);

/* Rewind a string buffer's cursor
 * buff - the buffer to affect */
void s_buff_rewind(s_buffer_t* buffer);

/* Move a buffer's cursor to index
 * buff - the buffer to affect
 * index - the index to move to */
void s_buff_move_to(s_buffer_t* buffer, uint32_t index);

/* Add a string to a string buffer
 * buff - the buffer to add a string to
 * string - the string to add
 * returns: success = 1, fail = 0 */
uint8_t s_buff_add_s(s_buffer_t* buffer, const char* string);

/* Add an integer to a string buffer
 * buff - the buffer to add an integer to
 * value - the integer value to add
 * returns: success = 1, fail = 0 */
uint8_t s_buff_add_i(s_buffer_t* buffer, int value);

/* Add a float to a string buffer
 * buff - the buffer to add a float to
 * value - the float value to add
 * returns: success = 1, fail = 0 */
uint8_t s_buff_add_f(s_buffer_t* buffer, float value);

/* Get a float value from a string buffer at cursor point
 * buff - the string buffer to check
 * return: the float value */
float s_buff_get_f(s_buffer_t* buffer);

/* Get an integer value from a string buffer at cursor point
 * buff - the string buffer to check
 * return: the integer value */
int s_buff_get_i(s_buffer_t* buffer);

/* Get a string value from a string buffer at cursor point
 * buff - the string buffer to check
 * dst - the destination to place the string
 * return: the string value */
char* s_buff_get_s(s_buffer_t* buffer, char* dst);

#ifdef __cplusplus
}
#endif
#endif

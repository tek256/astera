Sys Reference
=============

This is reference is meant to be a programmer's companion when using astera!
NOTE: This is still under construction, feedback is welcome!

Macros
^^^^^^

**General**

| ``ASTERA_SYS_LOWP_TIME`` - Typedef's ``time_s`` to single precision ``float`` instead of the default ``double``.
| ``ASTERA_NO_CONF`` - Prevents configuration utilities from being included & defined

**Timing**

| ``SEC_TO_NS`` - Convert seconds to nanoseconds 
| ``SEC_TO_MCS`` - Convert seconds to microseconds
| ``SEC_TO_MS`` - Convert seconds to milliseconds
| ``SEC_TO_MIN`` - Convert seconds to minutes
| ``SEC_TO_HOUR`` - Convert seconds to hours
| ``MS_TO_NS`` - Convert milliseconds to nanoseconds
| ``MS_TO_MCS`` - Convert milliseconds to microseconds
| ``MS_TO_SEC`` - Convert milliseconds to seconds
| ``MS_TO_MIN`` - Convert milliseconds to minutes
| ``MS_TO_HOUR`` - Convert milliseconds to hours
| ``MCS_TO_NS`` - Convert microseconds to nanoseconds
| ``MCS_TO_MS`` - Convert microseconds to milliseconds
| ``MCS_TO_SEC`` - Convert microseconds to seconds
| ``MCS_TO_MIN`` - Convert microseconds to minutes
| ``MCS_TO_HOUR`` - Convert microseconds to hours
| ``NS_TO_MCS`` - Convert nanoseconds to microseconds
| ``NS_TO_MS`` - Convert nanoseconds to milliseconds
| ``NS_TO_SEC`` - Convert nanoseconds to seconds
| ``NS_TO_MIN`` - Convert nanoseconds to minutes
| ``NS_TO_HOUR`` - Convert nanoseconds to hours

Types
^^^^^

**Configuration**

| ``s_table`` - The struct representation of a configuration file

**Timing**

| ``time_s`` - The representation of time (double by default, float optional)
| ``s_timer`` - A timer type for tracking time deltas

Functions 
^^^^^^^^^

**Configuration**

| ``s_table s_table_get(unsigned char* data, uint32_t length);`` - Process a file's contents as a Psuedo-INI File
| ``void s_table_free(s_table table);`` - Free the contents of an ``s_table`` type.
| ``uint8_t s_table_write(s_table* table, char* filepath);`` - Write an ``s_table`` type to filepath in psuedo-ini format.
| ``uint8_t s_table_write_mem();`` - Write an ``s_table`` type to memory

**Timing**

| ``time_s s_get_time();`` - Get the current time
| ``time_s s_timer_update(s_timer* t);`` - Update the mark & delta for timer
| ``s_timer s_timer_create();`` - Create an updated ``s_timer`` type
| ``time_s s_sleep(time_s duration);`` - Sleep for time in milliseconds


**Other**

| ``char* s_itoa(int32_t value, char* string, int8_t base);`` - Convert an integer to string

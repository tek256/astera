Sys Guide
=============

Usage 
^^^^^

Sys is the ``system`` utility within astera. Sys comprises 2 main parts, timing and configuration. 

However if you don't want the configuration functions in your project you can define ``ASTERA_NO_CONF`` before including the sys header to exclude it. 

Timing
^^^^^^

The timing system in astera is defined in ``milliseconds``.

There are defines to convert nanoseconds, microseconds, milliseconds, and seconds to and from each of the units. For example ``SEC_TO_MS`` is just defined as ``1e-6`` which is just 1 / 1000. 

Sys.h also includes two typedefs for timing:

| ``s_timer`` which holds the last update, and the delta to this update.
| and ``time_s`` which is defined as a ``double``, or optionally ``float`` if the macro ``ASTERA_SYS_LOWP_TIME``

The timing functions within sys.h also include:

| ``time_s s_get_time();`` - Returns the current time in milliseconds
| ``time_s s_timer_update(s_timer* timer);`` - Updates a timer & returns the calculated delta
| ``s_timer s_timer_create();`` - Create a timer struct with current time
| ``time_s s_sleep(time_s duration);`` - Sleep for time (milliseconds), return time slept

Configuration
^^^^^^^^^^^^^

The configuration portion of sys is essentially a barebone (and unreliable) INI File loader. Keep in mind that this loader works off of new line, splits by equals, and doesn't support tables. 

**Reading**

The loader essentially splits each line by the ``=`` sign and reallocates the data into an ``s_table`` for usage. It can prove useful, but isn't the most reliable of configuration methods.

In order to use this loader you'll want to call ``s_table s_table_get(unsigned char* data, uint32_t length);``. The data passed being the raw data of the file (you can use the asset system to load this data). 

Once you're done with the table, you can call ``s_table_free(s_table table)`` to free the arrays within it.

**Writing**

If you construct one of these tables and want to write it to file you can do so with ``uint8_t s_table_write(s_table* table, char* filepath);`` or ``uint8_t s_table_write_mem(void* data, uint32_t dst_length, s_table* table, uint32_t* write_length);``. 

#ifndef INPUT_H
#define INPUT_H

#include "config.h"
#include "platform.h"
#include <misc/linmath.h>

#define BINDING_KEY 1
#define BINDING_MB 2
#define BINDING_JOYA 3
#define BINDING_JOYB 4

#define MAX_KEYS 16
#define MAX_CHARS 16
#define MAX_MOUSE_BUTTONS 16

#define MAX_KEY_BINDINGS 32

#define MAX_JOY_AXES 12
#define MAX_JOY_BUTTONS 16

#define XBOX_360_PAD 0
#define XBOX_ONE_PAD 1
#define PS3_PAD 2
#define PS4_PAD 3
#define GENERIC_PAD 4

#if defined(PLAT_MSFT) || defined(PLAT_LINUX) || defined(PLAT_BSD)

#define XBOX_A 0
#define XBOX_B 1
#define XBOX_X 2
#define XBOX_Y 3
#define XBOX_L1 4
#define XBOX_R1 5
#define XBOX_SELECT 6
#define XBOX_START 7
#define XBOX_LEFT_STICK 8
#define XBOX_RIGHT_STICK 9

#elif defined(PLAT_APPLE)

#define XBOX_A 16
#define XBOX_B 17
#define XBOX_X 18
#define XBOX_Y 19
#define XBOX_L1 13
#define XBOX_R1 14
#define XBOX_SELECT 10
#define XBOX_START 9
#define XBOX_LEFT_STICK 11
#define XBOX_RIGHT_STICK 12

#endif

#if defined(PLAT_MSFT)

#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 2
#define XBOX_R_Y 3
#define XBOX_D_X 6
#define XBOX_D_Y 7

#elif defined(PLAT_LINUX) || defined(PLAT_BSD)

#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 4
#define XBOX_R_Y 5
#define XBOX_D_X 7
#define XBOX_D_Y 8

#define XBOX_R_T 6
#define XBOX_L_T 3

#elif defined(PLAT_APPLE)

#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 3
#define XBOX_R_Y 4
#define XBOX_R_T 5
#define XBOX_L_T 6

#endif

typedef struct {
  double x, y;
  double dx, dy;
} i_positions;

typedef struct {
  u16 *prev;
  u16 *curr;

  u16 curr_count;
  u16 prev_count;
  u16 capacity;
} i_states;

typedef struct {
  f32 *prev;
  f32 *curr;

  u16 curr_count;
  u16 prev_count;
  u16 capacity;
} i_statesf;

typedef struct {
  char name[8];
  u16 uid;
  u8 state;

  u16 value;
  u16 alt;

  u8 type;
  u8 alt_type;

  int used : 1;
} key_binding;

static u16 key_binding_track = 0;

u16 i_init(void);
void i_exit(void);

u16 i_contains(u16 val, u16 *arr, int count);

i_positions i_create_p(void);
i_statesf i_create_sf(u16 size);
i_states i_create_s(u16 size);

void i_create_joy(u16 joy);
void i_destroy_joy(u16 joy);

f32 i_joy_axis(u16 axis);

u16 i_joy_button_down(u16 button);
u16 i_joy_button_up(u16 button);
u16 i_joy_button_clicked(u16 button);
u16 i_joy_button_released(u16 button);
void i_get_joy_buttons(u16 *dst, int count);
const char *i_get_joy_name(u16 joy);
u16 i_get_joy_type(u16 joy);

f32 i_joy_axis_delta(u16 joy);

void i_key_callback(int key, int scancode, int toggle);
u16 i_key_down(u16 key);
u16 i_key_up(u16 key);
u16 i_key_clicked(u16 key);
u16 i_key_released(u16 key);

u16 i_key_binding_track(void);

void i_set_screensize(u32 width, u32 height);
void i_set_char_tracking(int tracking);
void i_char_callback(u32 c);
void i_get_chars(char *dst, u16 count);

void i_set_mouse_grab(int grabbed);
int i_get_mouse_grab(void);

void i_mouse_button_callback(u16 button);
void i_mouse_pos_callback(double x, double y);
void i_mouse_scroll_callback(double sx, double sy);

void i_get_scroll(double *x, double *y);
double i_get_scroll_x(void);
double i_get_scroll_y(void);

u16 i_mouse_down(u16 button);
u16 i_mouse_up(u16 button);
u16 i_mouse_clicked(u16 button);
u16 i_mouse_released(u16 button);

u16 i_mouse_within(vec2 start, vec2 end);

void i_get_mouse_pos(double *x, double *y);
double i_get_mouse_x(void);
double i_get_mouse_y(void);

void i_get_moues_delta(double *x, double *y);
double i_get_delta_x(void);
double i_get_delta_y(void);

void i_add_binding(const char *name, int value, int type);
void i_enable_binding_track(const char *key_binding);
u16 i_binding_count(void);

void i_binding_track_callback(int value, int type);
u16 i_get_binding_type(const char *key_binding);
u16 i_get_binding_alt_type(const char *key_bindg);
u16 i_binding_clicked(const char *key_binding);
u16 i_binding_released(const char *key_binding);
u16 i_binding_down(const char *key_binding);
u16 i_binding_up(const char *key_binding);
f32 i_binding_val(const char *key_binding); // gets the value
u16 i_binding_defined(const char *key_binding);

f32 i_opposing(const char *prim, const char *sec);

void i_default_bindings(void);

void i_update(void);

#endif

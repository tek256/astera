#ifndef INPUT_H
#define INPUT_H

#include "platform.h"

#define BINDING_KEY 1
#define BINDING_MB 2
#define BINDING_JOYA 3
#define BINDING_JOYB 4

#define MAX_KEYS 512
#define MAX_CHARS 512
#define MAX_MOUSE_BUTTONS 32

#define MAX_KEY_BINDINGS 64

#define MAX_JOY_AXES 12
#define MAX_JOY_BUTTONS 64

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
#define XBOX_START  7
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
#define XBOX_START  9
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

static int key_binding_track = 0;

void i_create_joy(int joyId);
void i_destroy_joy(int joyId);

static void i_add_joy_button(int button);
static void i_rm_joy_button(int button);

float i_joy_axis(int axis);

int   i_joy_button_down(int button);
int   i_joy_button_up(int button);
int   i_joy_button_clicked(int button);
int   i_joy_button_released(int button);
void  i_get_joy_buttons(int* dst, int count);
const char* i_get_joy_name(int joy);
int   i_get_joy_type(int joy);
float i_joy_axis_delta(int joy);

static void i_rm_concurrent_key(int index);
void        i_key_callback(int key, int scancode, int toggle);
int         i_key_down(int key);
int         i_key_up(int key);
int         i_key_clicked(int key);
int         i_key_released(int key);

void   i_set_char_tracking(int tracking);
void   i_char_callback(unsigned int c);
char*  i_get_chars();

void   i_set_mouse_grab(int grabbed);
int    i_get_mouse_grab();

void   i_mouse_button_callback(int button);
void   i_mouse_pos_callback(double x, double y);
void   i_mouse_scroll_callback(double sx, double sy);

void   i_get_scroll(double* x, double* y);
double i_get_scroll_x();
double i_get_scroll_y();

int   i_mouse_down(int button);
int   i_mouse_up(int button);
int   i_mouse_clicked(int button);
int   i_mouse_released(int button);

void   i_get_mouse_pos(double* x, double* y);
double i_get_mouse_x();
double i_get_mouse_y();

void   i_get_moues_delta(double* x, double* y);
double i_get_delta_x();
double i_get_delta_y();

void  i_add_binding(const char* name, int value, int type);
void  i_enable_binding_track(const char* key_binding);
int   i_binding_track();
void  i_binding_track_callback(int value, int type);
int   i_get_binding_type(const char* key_binding);
int   i_get_binding_alt_type(const char* key_bindg);
int   i_binding_clicked(const char* key_binding);
int   i_binding_released(const char* key_binding);
int   i_binding_down(const char* key_binding);
int   i_binding_up(const char* key_binding);
float i_binding_val(const char* key_binding); //gets the value
int   i_binding_defined(const char* key_binding);

float i_opposing(const char* prim, const char* sec);

void  i_default_bindings();

void  i_update();

#endif

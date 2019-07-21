#ifndef INPUT_H
#define INPUT_H

#define KEY_BINDING_KEY 1
#define KEY_BINDING_MOUSE_BUTTON 2
#define KEY_BINDING_JOY_AXIS 3
#define KEY_BINDING_JOY_BUTTON 4

#define MAX_KEYS 512
#define MAX_CHARS 512
#define MAX_MOUSE_BUTTONS 32

#define MAX_KEY_BINDINGS 64

#define MAX_JOY_AXES 12
#define MAX_JOY_BUTTONS 64

static int key_binding_track = 0;

void i_create_joy(int joyId);
void i_destroy_joy(int joyId);

static void i_add_joy_button(int button);
static void i_rm_joy_button(int button);

float i_get_joy_axis(int axis);

int   i_joy_button_down(int button);
int   i_joy_button_up(int button);
int   i_joy_button_clicked(int button);
int   i_joy_button_released(int button);

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
int   i_binding_clickedi(const char* key_binding);
int   i_bining_releasedi(const char* key_binding);
int   i_binding_downi(const char* key_binding);
int   i_binding_upi(const char* key_binding);
float i_binding_val(const char* key_binding); //gets the value
int   i_binding_defined(const char* key_binding);

float i_opposing(const char* prim, const char* sec);

void  i_default_bindings();

void  i_update();

#endif

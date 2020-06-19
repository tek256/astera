// TODO: Multi joy support
// Refactor joystick to support all button sizes not fixed
// Find out what the designated max size of `key` is for keyboard input &
// refactor i_states; to that

#ifndef ASTERA_INPUT_HEADER
#define ASTERA_INPUT_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if !defined ASTERA_KB_NAMELEN
#define ASTERA_KB_NAMELEN 8
#endif

#if !defined ASTERA_BINDING_KEY
#define ASTERA_BINDING_KEY 1
#endif

#if !defined ASTERA_BINDING_MB
#define ASTERA_BINDING_MB 2
#endif

#if !defined ASTERA_BINDING_JOYA
#define ASTERA_BINDING_JOYA 3
#endif

#if !defined ASTERA_BINDING_JOYB
#define ASTERA_BINDING_JOYB 4
#endif

#include <GLFW/glfw3.h>

#define XBOX_360_PAD 0
#define XBOX_ONE_PAD 1
#define PS3_PAD      2
#define PS4_PAD      3
#define GENERIC_PAD  4

// Useful keybindings
#if !defined(ASTERA_NO_GLFW_KEYBINDINGS)
#define KEY_SPACE       GLFW_KEY_SPACE
#define KEY_BACKSPACE   GLFW_KEY_BACKSPACE
#define KEY_DELETE      GLFW_KEY_DELETE
#define KEY_UP          GLFW_KEY_UP
#define KEY_DOWN        GLFW_KEY_DOWN
#define KEY_RIGHT       GLFW_KEY_RIGHT
#define KEY_LEFT        GLFW_KEY_LEFT
#define KEY_HOME        GLFW_KEY_HOME
#define KEY_TAB         GLFW_KEY_TAB
#define KEY_ESC         GLFW_KEY_ESCAPE
#define KEY_ESCAPE      GLFW_KEY_ESCAPE
#define KEY_LEFT_SHIFT  GLFW_KEY_LEFT_SHIFT
#define KEY_RIGHT_SHIFT GLFW_KEY_RIGHT_SHIFT
#define KEY_ENTER       GLFW_KEY_ENTER
#define KEY_LEFT_CTRL   GLFW_KEY_LEFT_CONTROL
#define KEY_RIGHT_CTRL  GLFW_KEY_RIGHT_CONTROL
#define KEY_LEFT_ALT    GLFW_KEY_LEFT_ALT
#define KEY_RIGHT_ALT   GLFW_KEY_RIGHT_ALT
#define KEY_LEFT_SUPER  GLFW_KEY_LEFT_SUPER
#define KEY_RIGHT_SUPER GLFW_KEY_RIGHT_SUPER

#define KEY_A GLFW_KEY_A
#define KEY_B GLFW_KEY_B
#define KEY_C GLFW_KEY_C
#define KEY_D GLFW_KEY_D
#define KEY_E GLFW_KEY_E
#define KEY_F GLFW_KEY_F
#define KEY_G GLFW_KEY_G
#define KEY_H GLFW_KEY_H
#define KEY_I GLFW_KEY_I
#define KEY_J GLFW_KEY_J
#define KEY_K GLFW_KEY_K
#define KEY_L GLFW_KEY_L
#define KEY_M GLFW_KEY_M
#define KEY_N GLFW_KEY_N
#define KEY_O GLFW_KEY_O
#define KEY_P GLFW_KEY_P
#define KEY_Q GLFW_KEY_Q
#define KEY_R GLFW_KEY_R
#define KEY_S GLFW_KEY_S
#define KEY_T GLFW_KEY_T
#define KEY_U GLFW_KEY_U
#define KEY_V GLFW_KEY_V
#define KEY_W GLFW_KEY_W
#define KEY_X GLFW_KEY_X
#define KEY_Y GLFW_KEY_Y
#define KEY_Z GLFW_KEY_Z
#define KEY_0 GLFW_KEY_0
#define KEY_1 GLFW_KEY_1
#define KEY_2 GLFW_KEY_2
#define KEY_3 GLFW_KEY_3
#define KEY_4 GLFW_KEY_4
#define KEY_5 GLFW_KEY_5
#define KEY_6 GLFW_KEY_6
#define KEY_7 GLFW_KEY_7
#define KEY_8 GLFW_KEY_8
#define KEY_9 GLFW_KEY_9

#define MOUSE_LEFT   GLFW_MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT  GLFW_MOUSE_BUTTON_RIGHT
#define MOUSE_MIDDLE GLFW_MOUSE_BUTTON_MIDDLE
#endif

#if defined _WIN32 || defined __linux__ || defined __unix__ || \
    defined                                        __FreeBSD__
#define XBOX_A           0
#define XBOX_B           1
#define XBOX_X           2
#define XBOX_Y           3
#define XBOX_L1          4
#define XBOX_R1          5
#define XBOX_SELECT      6
#define XBOX_START       7
#define XBOX_LEFT_STICK  8
#define XBOX_RIGHT_STICK 9

#elif defined __APPLE__

#define XBOX_A           16
#define XBOX_B           17
#define XBOX_X           18
#define XBOX_Y           19
#define XBOX_L1          13
#define XBOX_R1          14
#define XBOX_SELECT      10
#define XBOX_START       9
#define XBOX_LEFT_STICK  11
#define XBOX_RIGHT_STICK 12

#endif

#if defined _WIN32
#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 2
#define XBOX_R_Y 3
#define XBOX_D_X 6
#define XBOX_D_Y 7

#elif defined __linux__ || defined __unix__ || defined __FreeBSD__

#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 4
#define XBOX_R_Y 5
#define XBOX_D_X 7
#define XBOX_D_Y 8

#define XBOX_R_T 6
#define XBOX_L_T 3

#elif defined __APPLE__

#define XBOX_L_X 0
#define XBOX_L_Y 1
#define XBOX_R_X 3
#define XBOX_R_Y 4
#define XBOX_R_T 5
#define XBOX_L_T 6

#endif

typedef struct i_ctx i_ctx;

i_ctx* i_ctx_create(uint16_t max_mouse_buttons, uint16_t max_keys,
                    uint16_t max_bindings, uint16_t max_joy_axes,
                    uint16_t max_joy_buttons, uint16_t max_chars);
void   i_ctx_destroy(i_ctx* ctx);
void   i_ctx_update(i_ctx* ctx);

void i_poll_events();

void i_joy_create(i_ctx* ctx, uint16_t joy);
void i_joy_destroy(i_ctx* ctx, uint16_t joy);

float i_joy_axis(i_ctx* ctx, uint16_t axis);

int         i_joy_connected(i_ctx* ctx);
uint16_t    i_joy_down(i_ctx* ctx, uint16_t button);
uint16_t    i_joy_up(i_ctx* ctx, uint16_t button);
uint16_t    i_joy_clicked(i_ctx* ctx, uint16_t button);
uint16_t    i_joy_released(i_ctx* ctx, uint16_t button);
void        i_get_joy_buttons(i_ctx* ctx, uint16_t* dst, int count);
const char* i_get_joy_name(uint16_t joy);
uint16_t    i_get_joy_type(uint16_t joy);

float i_joy_axis_delta(i_ctx* ctx, uint16_t joy);

void     i_key_callback(i_ctx* ctx, int key, int scancode, int toggle);
uint16_t i_key_down(i_ctx* ctx, uint16_t key);
uint16_t i_key_up(i_ctx* ctx, uint16_t key);
uint16_t i_key_clicked(i_ctx* ctx, uint16_t key);
uint16_t i_key_released(i_ctx* ctx, uint16_t key);

uint16_t i_key_binding_track(i_ctx* ctx);

void i_set_char_tracking(i_ctx* ctx, int tracking);
int  i_get_char_tracking(i_ctx* ctx);
void i_char_callback(i_ctx* ctx, uint32_t c);
int  i_get_chars(i_ctx* ctx, char* dst, uint16_t count);
int  i_get_char_count(i_ctx* ctx);
void i_clear_chars(i_ctx* ctx);

void i_mouse_grab_set(GLFWwindow* window, int grabbed);
int  i_mouse_grab_get(GLFWwindow* window);

void i_mouse_button_callback(i_ctx* ctx, uint16_t button, int8_t toggle);
void i_mouse_pos_callback(i_ctx* ctx, double x, double y);
void i_mouse_scroll_callback(i_ctx* ctx, double sx, double sy);

void   i_scroll_get(i_ctx* ctx, double* x, double* y);
double i_scroll_get_x(i_ctx* ctx);
double i_scroll_get_y(i_ctx* ctx);
void   i_scroll_reset(i_ctx* ctx);

uint16_t i_mouse_down(i_ctx* ctx, uint16_t button);
uint16_t i_mouse_up(i_ctx* ctx, uint16_t button);
uint16_t i_mouse_clicked(i_ctx* ctx, uint16_t button);
uint16_t i_mouse_released(i_ctx* ctx, uint16_t button);

void   i_mouse_get_pos(i_ctx* ctx, double* x, double* y);
double i_mouse_get_x(i_ctx* ctx);
double i_mouse_get_y(i_ctx* ctx);

void   i_mouse_get_delta(i_ctx* ctx, double* x, double* y);
double i_mouse_get_dx(i_ctx* ctx);
double i_mouse_get_dy(i_ctx* ctx);

int i_any_event(i_ctx* ctx);

void i_binding_add(i_ctx* ctx, const char* name, int value, int type);
void i_binding_add_alt(i_ctx* ctx, const char* name, int value, int type);

void i_enable_binding_track(i_ctx* ctx, const char* key_binding, uint8_t alt);
uint16_t i_binding_count(i_ctx* ctx);

void     i_binding_track_callback(i_ctx* ctx, int value, int type);
uint16_t i_binding_get_type(i_ctx* ctx, const char* key_binding);
uint16_t i_binding_get_alt_type(i_ctx* ctx, const char* key_bindg);
uint16_t i_binding_clicked(i_ctx* ctx, const char* key_binding);
uint16_t i_binding_released(i_ctx* ctx, const char* key_binding);
uint16_t i_binding_down(i_ctx* ctx, const char* key_binding);
uint16_t i_binding_up(i_ctx* ctx, const char* key_binding);
float    i_binding_val(i_ctx*      ctx,
                       const char* key_binding); // gets the value
uint16_t i_binding_defined(i_ctx* ctx, const char* key_binding);

#ifdef __cplusplus
}
#endif
#endif


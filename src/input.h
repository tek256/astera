#ifndef ASTERA_INPUT_HEADER
#define ASTERA_INPUT_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <linmath.h>
#include <stdint.h>

#include <GLFW/glfw3.h>

#define BINDING_KEY  1
#define BINDING_MB   2
#define BINDING_JOYA 3
#define BINDING_JOYB 4

#define MAX_KEYS          16
#define MAX_CHARS         16
#define MAX_MOUSE_BUTTONS 16

#define MAX_KEY_BINDINGS 32

#define MAX_JOY_AXES    12
#define MAX_JOY_BUTTONS 16

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
#endif

#if defined(PLAT_MSFT) || defined(PLAT_LINUX) || defined(PLAT_BSD)

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

#elif defined(PLAT_APPLE)

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
  uint16_t* prev;
  uint16_t* curr;

  uint16_t curr_count;
  uint16_t prev_count;
  uint16_t capacity;
} i_states;

typedef struct {
  float* prev;
  float* curr;

  uint16_t curr_count;
  uint16_t prev_count;
  uint16_t capacity;
} i_statesf;

typedef struct {
  char     name[8];
  uint16_t uid;
  uint8_t  state;

  uint16_t value;
  uint16_t alt;

  uint8_t type;
  uint8_t alt_type;

  int used : 1;
} key_binding;

static uint16_t key_binding_track = 0;

uint16_t i_init(void);
void     i_exit(void);

uint16_t i_contains(uint16_t val, uint16_t* arr, int count);

i_positions i_create_p(void);
i_statesf   i_create_sf(uint16_t size);
i_states    i_create_s(uint16_t size);

void i_create_joy(uint16_t joy);
void i_destroy_joy(uint16_t joy);

float i_joy_axis(uint16_t axis);

uint16_t    i_joy_button_down(uint16_t button);
uint16_t    i_joy_button_up(uint16_t button);
uint16_t    i_joy_button_clicked(uint16_t button);
uint16_t    i_joy_button_released(uint16_t button);
void        i_get_joy_buttons(uint16_t* dst, int count);
const char* i_get_joy_name(uint16_t joy);
uint16_t    i_get_joy_type(uint16_t joy);

float i_joy_axis_delta(uint16_t joy);

void     i_key_callback(int key, int scancode, int toggle);
uint16_t i_key_down(uint16_t key);
uint16_t i_key_up(uint16_t key);
uint16_t i_key_clicked(uint16_t key);
uint16_t i_key_released(uint16_t key);

uint16_t i_key_binding_track(void);

void i_set_screensize(uint32_t width, uint32_t height);
void i_set_char_tracking(int tracking);
void i_char_callback(uint32_t c);
void i_get_chars(char* dst, uint16_t count);

void i_set_mouse_grab(GLFWwindow* window, int grabbed);
int  i_get_mouse_grab(GLFWwindow* window);

void i_mouse_button_callback(uint16_t button);
void i_mouse_pos_callback(double x, double y);
void i_mouse_scroll_callback(double sx, double sy);

void   i_get_scroll(double* x, double* y);
double i_get_scroll_x(void);
double i_get_scroll_y(void);

uint16_t i_mouse_down(uint16_t button);
uint16_t i_mouse_up(uint16_t button);
uint16_t i_mouse_clicked(uint16_t button);
uint16_t i_mouse_released(uint16_t button);

uint16_t i_mouse_within(vec2 start, vec2 end);

void   i_get_mouse_pos(double* x, double* y);
double i_get_mouse_x(void);
double i_get_mouse_y(void);

void   i_get_moues_delta(double* x, double* y);
double i_get_delta_x(void);
double i_get_delta_y(void);

void     i_add_binding(const char* name, int value, int type);
void     i_enable_binding_track(const char* key_binding);
uint16_t i_binding_count(void);

void     i_binding_track_callback(int value, int type);
uint16_t i_get_binding_type(const char* key_binding);
uint16_t i_get_binding_alt_type(const char* key_bindg);
uint16_t i_binding_clicked(const char* key_binding);
uint16_t i_binding_released(const char* key_binding);
uint16_t i_binding_down(const char* key_binding);
uint16_t i_binding_up(const char* key_binding);
float    i_binding_val(const char* key_binding); // gets the value
uint16_t i_binding_defined(const char* key_binding);

float i_opposing(const char* prim, const char* sec);

void i_update(void);

#ifdef __cplusplus
}
#endif
#endif


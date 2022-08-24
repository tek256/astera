#ifndef ASTERA_INPUT_HEADER
#define ASTERA_INPUT_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

/* Create an input context
 * max_mouse_buttons - the max number of tracked mouse buttons
 * max_keys - the max number of keyboard keys to track
 * max_bindings - the max number of bindings to track
 * max_joys - the max number of joysticks to track
 * max_chars - the max numbers of characters to track for string input */
i_ctx* i_ctx_create(uint16_t max_mouse_buttons, uint16_t max_keys,
                    uint16_t max_bindings, uint8_t max_joys,
                    uint16_t max_chars);

/* Get the current number of keys pressed */
uint32_t i_ctx_current_keys(i_ctx* ctx);

/* DEBUG ONLY TODO REMOVE
 * output current keyboard states */
void i_ctx_debug_out(i_ctx* ctx);

/* Destroy an input context and all of its resources
 * ctx - the context to destroy */
void i_ctx_destroy(i_ctx* ctx);

/* Update an input context (generally once a frame)
 * ctx - the context to update */
void i_ctx_update(i_ctx* ctx);

/* Poll input events */
void i_poll_events();

/* Setup a joystick in the input context
 * ctx - context to setup joy in
 * joy - the joystick id
 * returns: context's joy ID for joystick created, 0 = fail */
int8_t i_joy_create(i_ctx* ctx, uint8_t joy);

/* Check if a joystick is a gamepad or not
 * ctx - the input context
 * joy - the joystick
 * returns 1 = yes, 0 = no, -1 joy not found */
int8_t i_joy_is_gamepad(i_ctx* ctx, uint8_t joy);

/* Remove a joystick from the input context
 * ctx - the input context
 * joy - the joy id returned on creation */
void i_joy_destroy(i_ctx* ctx, uint8_t joy);

/* Get a joystick axis value
 * ctx - the input context
 * joy_id - the joystick id returned on creation
 * axis - the axis to check */
float i_joy_axis(i_ctx* ctx, uint8_t joy_id, uint8_t axis);

/* Check if any joysticks are connected (Gamepads included)
 * ctx - the context to check
 * returns: success = 1, fail = 0 */
uint8_t i_joy_connected(i_ctx* ctx);

/* Get the first joystick connected in the list
 * ctx - the context to checl
 * returns: non-zero joystick ID, fail = 0 */
uint8_t i_joy_first(i_ctx* ctx);

/* Check if a specific joystick exists by ID
 * ctx - the context to check
 * joy_id - the ID of a joystick
 * return: success = 1, fail = 0 */
uint8_t i_joy_exists(i_ctx* ctx, uint8_t joy_id);

/* Check if a joystick button is down currently
 * ctx - the context to check
 * joy_id - the ID of the joystick (returned on create/check)
 * button - the ID of the button
 * returns: yes = 1, no = 0 */
uint8_t i_joy_down(i_ctx* ctx, uint8_t joy_id, uint8_t button);

/* Check if a joystick button is up currently
 * ctx - the context to check
 * joy_id - the ID of the joystick (returned on create/check)
 * button - the ID of the button
 * returns: yes = 1, no = 0 */
uint8_t i_joy_up(i_ctx* ctx, uint8_t joy_id, uint8_t button);

/* Check if a joystick button is clicked this frame
 * ctx - the context to check
 * joy_id - the ID of the joystick (returned on create/check)
 * button - the ID of the button
 * returns: yes = 1, no = 0 */
uint8_t i_joy_clicked(i_ctx* ctx, uint8_t joy_id, uint8_t button);

/* Check if a joystick button is released this frame
 * ctx - the context to check
 * joy_id - the ID of the joystick (returned on create/check)
 * button - the ID of the button
 * returns: yes = 1, no = 0 */
uint8_t i_joy_released(i_ctx* ctx, uint8_t joy_id, uint8_t button);

/* Get the device name of a joystick by ID
 * joy - the ID of the joystick
 * returns: device name string */
const char* i_get_joy_name(uint8_t joy);

/* Get the type of a joystick device
 * joy - the ID of the joystick
 * returns: XBOX_360_PAD, PS3_PAD, GENERIC_PAD, etc */
uint16_t i_get_joy_type(uint8_t joy);

/* Printf all the stuff in a joy lol*/
void i_joy_debug_output(i_ctx* ctx, uint8_t joy);

/* Get the delta of a joystick axis in the last frame
 * ctx - the context to check
 * joy_id - the ID of the joystick
 * axis - the axis of the controller to check
 * returns: change in axis between current and last frame */
float i_joy_axis_delta(i_ctx* ctx, uint8_t joy_id, uint8_t axis);

/* Set controller vibration
 * ctx - the context to update
 * joy_id - the ID of the joystick to modify
 * left - the amount of vibration to apply (0.0-1.0f) for the left motor
 * right - the amount of vibration to apply (0.0-1.0f for the right motor */
void i_joy_set_vibration(i_ctx* ctx, uint8_t joy_id, float left, float right);

/* the glfw key callback
 * ctx - context to update
 * key - the key ID
 * scancode - unused
 * toggle - if pressed or released */
void i_key_callback(i_ctx* ctx, int key, int scancode, int toggle);

/* Check if a key is currently down this frame
 * ctx - context to check
 * key - the key id/char
 * returns: 1 = true, 0 = false */
uint16_t i_key_down(i_ctx* ctx, uint16_t key);

/* Check if a key is not currently down this frame
 * ctx - context to check
 * key - the key id/char
 * returns: 1 = true, 0 = false */
uint16_t i_key_up(i_ctx* ctx, uint16_t key);

/* Check if a key was pressed starting this frame
 * ctx - context to check
 * key - the key id/char
 * returns: 1 = true, 0 = false */
uint16_t i_key_clicked(i_ctx* ctx, uint16_t key);

/* Check if a key was released starting this frame
 * ctx - context to check
 * key - the key id/char
 * returns: 1 = true, 0 = false */
uint16_t i_key_released(i_ctx* ctx, uint16_t key);

/* Check if key binding tracking is enabled
 * ctx - context to check
 * returns: 1 = true, 0 = false */
uint16_t i_key_binding_track(i_ctx* ctx);

/* Enable/Disable character tracking (string input)
 * ctx - the context to use
 * tracking - if to use char tracking or not */
void i_set_char_tracking(i_ctx* ctx, int tracking);

/* Check if character tracking is enabled (string input)
 * ctx - the context to check
 * returns: 1 - tracking enabled, 0 - no tracking */
int i_get_char_tracking(i_ctx* ctx);

/* The GLFW callback for character tracking
 * ctx - the context to use
 * c - the character in 32bit int format */
void i_char_callback(i_ctx* ctx, uint32_t c);

/* Get the data from the char tracking buffer (string input)
 * dst - the buffer to place the data in
 * count - the max size of dst
 * returns: the number of characters placed */
int i_get_chars(i_ctx* ctx, char* dst, uint16_t count);

/* Get the number of characters in the char tracking buffer (string input)
 * ctx - the context to check
 * returns: number of characters in char tracking buffer */
int i_get_char_count(i_ctx* ctx);

/* Clear out all the data from the char tracking buffer (string input)
 * ctx - the context to clear */
void i_clear_chars(i_ctx* ctx);

/* Set if the mouse should be grabbed or not
 * window - the GLFW window handle from r_window_get_glfw
 * grabbed - if the mouse should be set to grabbed or not */
void i_mouse_grab_set(GLFWwindow* window, int grabbed);

/* Check if the mosue is grabbed or not
 * window - the GLFW window handle from r_window_get_glfw
 * returns: if the mouse is grabbed or not */
int i_mouse_grab_get(GLFWwindow* window);

void i_mouse_button_callback(i_ctx* ctx, uint16_t button, int8_t toggle);
void i_mouse_pos_callback(i_ctx* ctx, double x, double y);
void i_mouse_scroll_callback(i_ctx* ctx, double sx, double sy);

void i_scroll_get(i_ctx* ctx, double* x, double* y);
void i_scroll_get_d(i_ctx* ctx, double* x, double* y);

/* Get mouse scroll x position
 * ctx - context to check
 * returns: scroll position on x axis */
double i_scroll_get_x(i_ctx* ctx);

/* Get mouse scroll y position
 * ctx - context to check
 * returns: scroll position on y axis */
double i_scroll_get_y(i_ctx* ctx);

/* Get mouse scroll delta on the x axis
 * ctx - context to check
 * returns: scroll delta on x axis */
double i_scroll_get_dx(i_ctx* ctx);

/* Get mouse scroll delta on the y axis
 * ctx - context to check
 * returns: scroll delta on y axis */
double i_scroll_get_dy(i_ctx* ctx);

/* Reset the mouse scroll position to 0
 * ctx - context to modify */
void i_scroll_reset(i_ctx* ctx);

/* Check if a mouse button is down this frame
 * ctx - context to check
 * button - the mouse button index
 * returns: 1 = true, 0 = false */
uint16_t i_mouse_down(i_ctx* ctx, uint16_t button);

/* Check if a mouse button isn't down this frame
 * ctx - context to check
 * button - the mouse button index
 * returns: 1 = true, 0 = false */
uint16_t i_mouse_up(i_ctx* ctx, uint16_t button);

/* Check if a mouse button is down starting this frame
 * ctx - context to check
 * button - the mouse button index
 * returns: 1 = true, 0 = false */
uint16_t i_mouse_clicked(i_ctx* ctx, uint16_t button);

/* Check if a mouse button is up starting this frame
 * ctx - context to check
 * button - the mouse button index
 * returns: 1 = true, 0 = false */
uint16_t i_mouse_released(i_ctx* ctx, uint16_t button);

/* Get the mouse position
 * ctx - context to check
 * x - double pointer to set the x axis value
 * y - double pointer to set the y axis value
 * NOTE: null pointers can be passed */
void i_mouse_get_pos(i_ctx* ctx, double* x, double* y);

/* Get the mouse x position
 * ctx - context to check
 * returns: mouse position on the x axis */
double i_mouse_get_x(i_ctx* ctx);

/* Get the mouse y position
 * ctx - context to check
 * returns: mouse position on the y axis */
double i_mouse_get_y(i_ctx* ctx);

/* Get the mouse's delta this frame
 * ctx - context to check
 * x - double pointer to set the delta x value
 * y - double pointer to set the delta y value
 * NOTE: null pointers can be passed */
void i_mouse_get_delta(i_ctx* ctx, double* x, double* y);

/* Get the mouse's delta on the x axis this frame
 * ctx - context to check
 * returns: mouse delta on the x axis */
double i_mouse_get_dx(i_ctx* ctx);

/* Get the mouse's delta on the y axis this frame
 * ctx - context to check
 * returns: mouse delta on the y axis */
double i_mouse_get_dy(i_ctx* ctx);

/* Check if any input events happened this frame
 * ctx - the context to check
 * returns: 1 - any event happened, 0 - no events*/
int i_any_event(i_ctx* ctx);

/* Add a key binding to an input context
 * name - the name of the key binding (max length = 7 chars)
 * value - the action required to trigger the binding
 * type - the type (ASTERA_BINDING_KEY, ASTERA_BINDING_JOYA,
 *                  ASTERA_BINDING_JOYB, ASTERA_BINDING_MOUSEB)
 *
 * returns: binding ID, 0 = failure */
uint16_t i_binding_add(i_ctx* ctx, const char* name, int value, int type);

/* Add an alternate key binding to an input context (doesn't override
 * original) name - the name of the key binding value - the action required to
 * trigger the binding type - the type (ASTERA_BINDING_KEY,
 * ASTERA_BINDING_JOYA, ASTERA_BINDING_JOYB, ASTERA_BINDING_MOUSEB)
 *
 * returns: binding ID, 0 = failure */
uint16_t i_binding_add_alt(i_ctx* ctx, const char* name, int value, int type);

/* Enable key binding track
 * ctx - the context to use
 * key_binding - the name of the key binding
 * alt - if to modify the alt value
 * returns: binding ID, 0 = failure */
uint16_t i_enable_binding_track(i_ctx* ctx, const char* key_binding,
                                uint8_t alt);

/* Enable key binding track by binding ID
 * ctx - the context to use
 * binding_id - the ID of the key binding
 * alt - if to modify the alt value
 * returns: binding ID, 0 = failure */
uint16_t i_enable_bindingi_track(i_ctx* ctx, uint16_t binding_id, uint8_t alt);

/* Add an alternate key binding to an input context (doesn't override
 * original) binding_id - the ID of the key binding value - the action
 * required to trigger the binding type - the type (ASTERA_BINDING_KEY,
 * ASTERA_BINDING_JOYA, ASTERA_BINDING_JOYB, ASTERA_BINDING_MOUSEB) returns:
 * binding ID, 0 = failure */
uint16_t i_bindingi_add_alt(i_ctx* ctx, uint16_t binding_id, int value,
                            int type);

/* Get the ID of a key binding by name
 * ctx - the context to search
 * name- the name to search for
 * returns: binding ID, 0 = fail/undefined */
uint16_t i_binding_get_id(i_ctx* ctx, const char* name);

/* Get the number of key bindings in the context
 * ctx - the context to check
 * returns: the number of key bindings */
uint16_t i_binding_count(i_ctx* ctx);

/* Set the value for a tracked binding with this callback
 * ctx - the context to update
 * source - the type of device (joystick, keyboard, mouse, etc)
 * value - the value/index of button
 * type - the type of input from that device (i.e joystick axis vs button) */
void i_binding_track_callback(i_ctx* ctx, int source, int value, int type);

/* Get the (device) type a binding uses
 * ctx - context to check
 * key_binding - the name of the binding
 * returns: (device) type */
uint16_t i_binding_get_type(i_ctx* ctx, const char* key_binding);

/* Get the alt value (device) type a binding uses
 * ctx - context to check
 * key_binding - the name of the binding
 * returns: alt (device) type */
uint16_t i_binding_get_alt_type(i_ctx* ctx, const char* key_bindg);

/* Check if the binding is down starting this frame
 * NOTE: if not a button type, this will always return 0
 * ctx - context to check
 * key_binding - the name of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_binding_clicked(i_ctx* ctx, const char* key_binding);

/* Check if the binding is up starting this frame
 * NOTE: if not a button type, this will always return 0
 * ctx - context to check
 * key_binding - the name of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_binding_released(i_ctx* ctx, const char* key_binding);

/* Check if the binding is down this frame
 * NOTE: if type is joystick axis this will check if it's negative value (<
 * 0.f) ctx - context to check key_binding - the name of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_binding_down(i_ctx* ctx, const char* key_binding);

/* Check if the binding is up this frame
 * NOTE: if type is joystick axis this will check if it's positive value (>
 * 0.f) ctx - context to check key_binding - the name of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_binding_up(i_ctx* ctx, const char* key_binding);

/* Get the value of a key binding (psuedo axis)
 * ctx - context to check
 * key_binding - the binding name
 * returns: 1 = down, 0 = up; joy axis: joy axis value */
float i_binding_val(i_ctx* ctx, const char* key_binding);

/* Check if a key binding is defined
 * ctx - context to check
 * key_binding - the binding name
 * returns: binding_id, 0 = false (undefined) */
uint8_t i_binding_defined(i_ctx* ctx, const char* key_binding);

/* Get the (device) type a binding uses
 * ctx - context to check
 * binding_id - the id of the binding
 * returns: (device) type */
uint16_t i_bindingi_get_type(i_ctx* ctx, uint16_t binding_id);

/* Get the alt value (device) type a binding uses
 * ctx - context to check
 * binding_id - the id of the binding
 * returns: alt (device) type */
uint16_t i_bindingi_get_alt_type(i_ctx* ctx, uint16_t binding_id);

/* Check if the binding is down starting this frame
 * NOTE: if not a button type, this will always return 0
 * ctx - context to check
 * binding_id - the id of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_bindingi_clicked(i_ctx* ctx, uint16_t binding_id);

/* Check if the binding is up starting this frame
 * NOTE: if not a button type, this will always return 0
 * ctx - context to check
 * binding_id - the name of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_bindingi_released(i_ctx* ctx, uint16_t binding_id);

/* Check if the binding is down this frame
 * NOTE: if type is joystick axis this will check if it's negative value (<
 * 0.f) ctx - context to check binding_id - the id of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_bindingi_down(i_ctx* ctx, uint16_t binding_id);

/* Check if the binding is up this frame
 * NOTE: if type is joystick axis this will check if it's positive value (>
 * 0.f) ctx - context to check binding_id - the id of the key binding
 * returns: 1 = true, 0 = false */
uint8_t i_bindingi_up(i_ctx* ctx, uint16_t binding_id);

/* Get the value of a key binding (psuedo axis)
 * ctx - context to check
 * binding_id - the binding id
 * returns: 1 = down, 0 = up; joy axis: joy axis value */
float i_bindingi_val(i_ctx* ctx, uint16_t binding_id);

/* Check if a key binding is defined
 * ctx - context to check
 * key_binding - the binding id
 * returns: 1 = true (defined), 0 = false (undefined) */
uint8_t i_bindingi_defined(i_ctx* ctx, uint16_t binding_id);

#ifdef __cplusplus
}
#endif
#endif

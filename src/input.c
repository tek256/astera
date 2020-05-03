#ifndef INPUT_C
#define INPUT_C

#include <astera/input.h>

#include <stdlib.h>
#include <string.h>

#if !defined DBG_E
#if defined  ASTERA_DEBUG_OUTPUT
#if defined  ASTERA_DEBUG_INCLUDED
#define DBG_E(fmt, ...) _l(fmt, __VA_ARGS__)
#else
#include <stdio.h>
#define DBG_E(fmt, ...) printf(fmt, __VA_ARGS__)
#endif
#else
#define DBG_E(fmt, ...)
#endif
#endif

static char     chars[ASTERA_MAX_CHARS];
static uint16_t char_count = 0;
static char     char_track = 0;

static unsigned char joystick_id = 0;
static unsigned char joy_exists  = 0;

i_states joy_b;
i_states mouse_b;
i_states keyboard;

uint32_t screen_width, screen_height;

uint16_t* current_keys;
uint16_t  current_key_count;

i_statesf joy_a;

// current
i_positions mouse_p;
i_positions mouse_l;
// scroll
i_positions mouse_s;

key_binding* key_bindings;
key_binding* tracked_key_binding;
int          key_binding_count;

uint16_t i_init(void) {
  mouse_b = i_create_s(ASTERA_MAX_MOUSE_BUTTONS);
  if (!mouse_b.curr || !mouse_b.prev) {
    DBG_E("Unable to malloc space for mouse.\n");
    i_exit();
    return 0;
  }

  keyboard = i_create_s(ASTERA_MAX_KEYS);
  if (!keyboard.curr || !keyboard.prev) {
    DBG_E("Unable to malloc space for keyboard.\n");
    i_exit();
    return 0;
  }

  current_keys = malloc(sizeof(uint16_t) * ASTERA_MAX_KEYS);
  if (!current_keys) {
    DBG_E("Unable to malloc space for keyboard.\n");
    i_exit();
    return 0;
  }

  current_key_count = 0;

  mouse_p = i_create_p();
  mouse_s = i_create_p();

  key_binding_count   = 0;
  tracked_key_binding = 0;
  key_bindings        = malloc(sizeof(key_binding) * ASTERA_MAX_KEY_BINDINGS);
  memset(key_bindings, 0, sizeof(key_binding) * ASTERA_MAX_KEY_BINDINGS);
  if (!key_bindings) {
    DBG_E("Unable to malloc space for key bindings.\n");
    i_exit();
    return 0;
  }
  return 1;
}

i_positions i_create_p(void) { return (i_positions){0, 0, 0, 0}; }

i_states i_create_s(uint16_t size) {
  void *a, *b;
  a = malloc(size * sizeof(uint16_t));
  b = malloc(size * sizeof(uint16_t));

  memset(a, 0, sizeof(uint16_t) * size);
  memset(b, 0, sizeof(uint16_t) * size);

  return (i_states){a, b, 0, 0, size};
}

i_statesf i_create_sf(uint16_t size) {
  void *a, *b;
  a = malloc(size * sizeof(float));
  b = malloc(size * sizeof(float));

  memset(a, 0, sizeof(float) * size);
  memset(b, 0, sizeof(float) * size);

  return (i_statesf){a, b, 0, 0, size};
}

void i_exit(void) {
  if (mouse_b.curr)
    free(mouse_b.curr);
  if (mouse_b.prev)
    free(mouse_b.prev);
  if (keyboard.curr)
    free(keyboard.curr);
  if (keyboard.prev)
    free(keyboard.prev);
  if (current_keys)
    free(current_keys);

  if (joy_exists) {
    if (joy_b.curr)
      free(joy_b.curr);
    if (joy_b.prev)
      free(joy_b.prev);
    if (joy_a.curr)
      free(joy_a.curr);
    if (joy_a.prev)
      free(joy_a.prev);
  }
}

uint16_t i_contains(uint16_t val, uint16_t* arr, int count) {
  for (int i = 0; i < count; ++i) {
    if (arr[i] == val) {
      return 1;
    }
  }
  return 0;
}

void i_create_joy(uint16_t joy_id) {
  if (!joy_exists) {
    int present = glfwJoystickPresent(joy_id);

    if (!present) {
      return;
    }

    joy_a = i_create_sf(ASTERA_MAX_JOY_AXES);
    joy_b = i_create_s(ASTERA_MAX_JOY_BUTTONS);

    joystick_id = joy_id;
  }
}

int8_t i_joy_exists(uint16_t joy) { return glfwJoystickPresent(joy); }

uint8_t i_joy_connected() { return joystick_id; }

float i_joy_axis_delta(uint16_t axis) {
  if (!joy_exists)
    return 0;
  return joy_a.curr[axis] - joy_a.prev[axis];
}

void i_get_joy_buttons(uint16_t* dst, int count) {
  if (!joy_exists)
    return;
  int cpy_count =
      (count > ASTERA_MAX_JOY_BUTTONS) ? ASTERA_MAX_JOY_BUTTONS : count;
  memcpy(dst, joy_b.curr, cpy_count * sizeof(uint16_t));
}

void i_get_joy_axes(float* dst, int count) {
  if (!joy_exists)
    return;
  int cpy_count = (count > ASTERA_MAX_JOY_AXES) ? ASTERA_MAX_JOY_AXES : count;
  memcpy(dst, joy_a.curr, cpy_count * sizeof(float));
}

const char* i_get_joy_name(uint16_t joy) { return glfwGetJoystickName(joy); }

uint16_t i_get_joy_type(uint16_t joy) {
  if (!glfwJoystickPresent(joy)) {
    return -1;
  }

  const char* name = i_get_joy_name(joy);
  if (strstr(name, "Microsoft")) {
    if (strstr(name, "360")) {
      return XBOX_360_PAD;
    } else if (strstr(name, "One")) {
      return XBOX_ONE_PAD;
    }
  } else if (strstr(name, "Sony")) {
    if (strstr(name, "3")) {
      return PS3_PAD;
    } else if (strstr(name, "4")) {
      return PS4_PAD;
    }
  } else {
    return GENERIC_PAD;
  }

  return 0;
}

void i_destroy_joy(uint16_t joy_id) {
  if (joy_id == joystick_id) {
    if (joy_exists) {
      joy_exists = 0;
    }
  }
}

float i_joy_axis(uint16_t axis) {
  if (!joy_exists)
    return 0.f;
  return joy_a.curr[axis];
}

uint16_t i_joy_button_down(uint16_t button) {
  if (!joy_exists)
    return 0;
  return joy_b.curr[button];
}

uint16_t i_joy_button_up(uint16_t button) {
  if (!joy_exists)
    return 0;
  return !joy_b.curr[button];
}

uint16_t i_joy_button_clicked(uint16_t button) {
  if (!joy_exists)
    return 0;
  return i_joy_button_down(button) && !joy_b.prev[button];
}

uint16_t i_joy_button_released(uint16_t button) {
  if (!joy_exists)
    return 0;
  return !joy_b.curr[button] && joy_b.prev[button];
}

void i_rm_key(uint16_t key) {
  int index = -1;
  for (int i = 0; i < current_key_count; ++i) {
    if (current_keys[i] == key) {
      index = i;
      break;
    }
  }

  int cap = (current_key_count >= ASTERA_MAX_KEYS - 1) ? ASTERA_MAX_KEYS
                                                       : current_key_count;

  for (int i = index; i < cap - 1; ++i) {
    current_keys[i] = current_keys[i + 1];
  }

  current_keys[cap - 1] = 0;
  --current_key_count;
}

void i_key_callback(int key, int scancode, int toggle) {
  if (toggle) {
    if (key_binding_track) {
      i_binding_track_callback(key, ASTERA_BINDING_KEY);
    }

    if (keyboard.curr_count == keyboard.capacity) {
      return;
    }

    if (i_contains((uint16_t)key, keyboard.curr, keyboard.curr_count)) {
      return;
    }

    keyboard.curr[keyboard.curr_count] = key;
    ++keyboard.curr_count;

    if (i_contains((uint16_t)key, current_keys, current_key_count) &&
        current_key_count < ASTERA_MAX_KEYS) {
      return;
    }

    current_keys[current_key_count] = key;
    ++current_key_count;
  } else {
    i_rm_key((uint16_t)key);
  }
}

void i_set_screensize(uint32_t width, uint32_t height) {
  screen_width  = width;
  screen_height = height;
}

void i_set_char_tracking(int tracking) { char_track = tracking; }

int i_get_char_tracking(int tracking) { return char_track; }

void i_char_callback(uint32_t c) {
  if (!char_track) {
    return;
  }

  chars[char_count] = c;
  char_count++;
}

int i_get_chars(char* dst, uint16_t count) {
  if (!char_count || !count) {
    return 0;
  }
  uint16_t cpy_count = (count > char_count) ? char_count : count;
  memcpy(dst, chars, sizeof(char) * cpy_count);
  return cpy_count;
}

int i_get_char_count() { return char_count; }

void i_clear_chars() {
  memset(chars, 0, sizeof(char) * ASTERA_MAX_CHARS);
  char_count = 0;
}

void i_set_mouse_grab(GLFWwindow* window, int grab) {
  if (grab) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

int i_get_mouse_grab(GLFWwindow* window) {
  int value = glfwGetInputMode(window, GLFW_CURSOR);

  if (value == GLFW_CURSOR_DISABLED) {
    return 1;
  } else if (value == GLFW_CURSOR_NORMAL) {
    return 0;
  } else {
    DBG_E("Error: Invalid input mode for GLFW_CURSOR: %i\n", value);
    return 0;
  }
}

void i_mouse_button_callback(uint16_t button) {
  if (key_binding_track) {
    i_binding_track_callback(button, ASTERA_BINDING_MB);
  }
  mouse_b.curr[mouse_b.curr_count] = button;
  ++mouse_b.curr_count;
}

void i_mouse_pos_callback(double x, double y) {
  mouse_p.x = x;
  mouse_p.y = y;
}

void i_mouse_scroll_callback(double sx, double sy) {
  mouse_s.x += sx;
  mouse_s.y += sy;
  mouse_s.dx = sx;
  mouse_s.dy = sy;
}

void i_get_scroll(double* x, double* y) {
  *x = mouse_s.dx;
  *y = mouse_s.dy;
}

uint16_t i_key_binding_track(void) { return key_binding_track; }

uint16_t i_mouse_down(uint16_t button) {
  return i_contains(button, mouse_b.curr, mouse_b.curr_count);
}

uint16_t i_mouse_up(uint16_t button) {
  return !i_contains(button, mouse_b.curr, mouse_b.curr_count);
}

uint16_t i_mouse_clicked(uint16_t button) {
  return i_mouse_down(button) &&
         !i_contains(button, mouse_b.prev, mouse_b.prev_count);
}

uint16_t i_mouse_released(uint16_t button) {
  return i_mouse_up(button) &&
         !i_contains(button, mouse_b.prev, mouse_b.prev_count);
}

double i_get_scroll_x(void) { return mouse_s.x; }

double i_get_scroll_y(void) { return mouse_s.y; }

void i_get_mouse_pos(double* x, double* y) {
  *x = mouse_p.x;
  *y = mouse_p.y;
}

double i_get_mouse_x(void) { return mouse_p.x; }

double i_get_mouse_y(void) { return mouse_p.y; }

void i_get_mouse_delta(double* x, double* y) {
  *x = mouse_p.dx;
  *y = mouse_p.dy;
}

double i_get_delta_x(void) { return mouse_p.dx; }

double i_get_delta_y(void) { return mouse_p.dy; }

uint16_t i_key_down(uint16_t key) {
  return i_contains(key, current_keys, current_key_count);
}

uint16_t i_key_up(uint16_t key) {
  return !i_contains(key, current_keys, current_key_count);
}

uint16_t i_key_clicked(uint16_t key) {
  return i_contains(key, keyboard.curr, keyboard.curr_count) &&
         !i_contains(key, keyboard.prev, keyboard.prev_count);
}

uint16_t i_key_released(uint16_t key) {
  return i_key_up(key) && i_contains(key, keyboard.prev, keyboard.prev_count);
}

int i_any_event(void) {
  for (int i = 0; i < keyboard.curr_count; ++i) {
    if (keyboard.curr[i] != 0) {
      return 1;
    }
  }

  for (int i = 0; i < mouse_b.curr_count; ++i) {
    if (mouse_b.curr[i] != 0) {
      return 1;
    }
  }

  if (joy_exists) {
    for (int i = 0; i < joy_b.curr_count; ++i) {
      if (joy_b.curr[i] != 0) {
        return 1;
      }
    }
  }

  return 0;
}

void i_add_binding(const char* name, int value, int type) {
  if (key_binding_count > 0) {
    for (int i = 0; i < key_binding_count; ++i) {
      if (strncmp(key_bindings[i].name, name, ASTERA_KB_NAMELEN) == 0) {
        key_bindings[i].value = value;
        key_bindings[i].type  = type;
        return;
      }
    }

    if (key_binding_count == ASTERA_MAX_KEY_BINDINGS) {
      DBG_E("Unable to add more key bindings.\n");
      return;
    }
  }

  strncpy(key_bindings[key_binding_count].name, name, ASTERA_KB_NAMELEN);
  key_bindings[key_binding_count].value = value;
  key_bindings[key_binding_count].type  = type;
  ++key_binding_count;
}

void i_add_binding_alt(const char* name, int value, int type) {
  for (int i = 0; i < key_binding_count; ++i) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (strncmp(name, key_bindings[i].name, len) == 0) {
      key_bindings[i].alt      = value;
      key_bindings[i].alt_type = type;
      return;
    }
  }
}

void i_enable_binding_track(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, 8)) {
      tracked_key_binding = &key_bindings[i];
      break;
    }
  }

  if (tracked_key_binding != NULL) {
    key_binding_track = 1;
  }
}

uint16_t i_binding_track(void) { return key_binding_track; }

void i_binding_track_callback(int value, int type) {
  if (tracked_key_binding != NULL) {
    tracked_key_binding->value = value;
    tracked_key_binding->type  = type;
  }

  key_binding_track = 0;
}

uint16_t i_get_binding_type(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, 8)) {
      return key_bindings[i].type;
    }
  }
  return 0;
}

uint16_t i_get_binding_alt_type(const char* key_binding) {
  for (int i = 0; i < key_binding_count; ++i) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, 8)) {
      return key_bindings[i].alt_type;
    }
  }
  return 0;
}

uint16_t i_binding_clicked(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, len)) {
      int val;
      switch (key_bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_clicked(key_bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(key_bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(key_bindings[i].value);
          break;
      }

      if (val) {
        return 1;
      }

      switch (key_bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_clicked(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(key_bindings[i].alt);
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_released(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, len)) {
      if (key_bindings[i].type == ASTERA_BINDING_JOYA) {
        return 0;
      }

      int val;
      switch (key_bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_released(key_bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(key_bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(key_bindings[i].value);
          break;
      }

      if (val) {
        return 1;
      }

      switch (key_bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_released(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(key_bindings[i].alt);
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_down(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, len)) {
      if (key_bindings[i].type == ASTERA_BINDING_JOYA) {
        return 0;
      }

      int val;
      switch (key_bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_down(key_bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(key_bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(key_bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(key_bindings[i].value) < 0.f) ? 1 : 0;
          break;
      }
      if (val) {
        return 1;
      }

      switch (key_bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_down(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(key_bindings[i].alt) < 0.f) ? 1 : 0;
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_up(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, len)) {
      int val;

      switch (key_bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_up(key_bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_up(key_bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_up(key_bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(key_bindings[i].value) > 0.f) ? 1 : 0;
          break;
      }

      if (val) {
        return 1;
      }

      switch (key_bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_button_up(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_up(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_up(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(key_bindings[i].value) > 0.f) ? 1 : 0;
          break;
      }

      return val;
    }
  }
  return 0;
}

float i_binding_val(const char* key_binding) {
  for (int i = 0; i < key_binding_count; i++) {
    int len = strlen(key_bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(key_bindings[i].name, key_binding, len)) {
      float val;
      switch (key_bindings[i].type) {
        case ASTERA_BINDING_MB:
          val = (i_mouse_down(key_bindings[i].value)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_KEY:
          val = (i_key_down(key_bindings[i].value)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_JOYA:
          val = i_joy_axis(key_bindings[i].value);
          break;
        case ASTERA_BINDING_JOYB:
          val = (i_joy_button_down(key_bindings[i].value)) ? 1.0f : 0.0f;
          break;
      }

      if (val != 0.f) {
        return val;
      }

      switch (key_bindings[i].alt_type) {
        case ASTERA_BINDING_MB:
          val = (i_mouse_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_KEY:
          val = (i_key_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_JOYA:
          val = i_joy_axis(key_bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYB:
          val = (i_joy_button_down(key_bindings[i].alt)) ? 1.0f : 0.0f;
          break;
      }

      return val;
    }
  }

  return 0.0f;
}

uint16_t i_binding_defined(const char* key_binding) {
  if (!key_binding) {
    return 0;
  }

  if (key_binding_count > 0) {
    for (int i = 0; i < key_binding_count; ++i) {
      int len = strlen(key_bindings[i].name);
      if (len != 0) {
        len = (len > 8) ? 8 : len;
        if (!strncmp(key_bindings[i].name, key_binding, len)) {
          return 1;
        }
      }
    }
  }
  return 0;
}

// TODO test code for removal
float i_opposing(const char* prim, const char* sec) {
  float prim_f = i_binding_down(prim) ? 2.f : 0.f;
  float sec_f  = i_binding_down(sec) ? -1.f : 0.f;
  return prim_f + sec_f;
}

void i_update(void) {
  mouse_p.dx = mouse_p.x - mouse_l.x;
  mouse_p.dy = mouse_p.y - mouse_l.y;

  mouse_l.x = mouse_p.x;
  mouse_l.y = mouse_p.y;

  if (!joy_exists) {
    for (int i = 0; i < 16; ++i) {
      if (glfwJoystickPresent(i)) {
        i_create_joy(i);
        DBG_E("Joystick [%i] found.\n", i);
        joy_exists = 1;
        break;
      }
    }
  }

  int overlap = (keyboard.curr_count > keyboard.prev_count)
                    ? keyboard.curr_count
                    : keyboard.prev_count;
  memset(keyboard.prev, 0, sizeof(uint16_t) * keyboard.prev_count);
  memcpy(keyboard.prev, keyboard.curr, sizeof(uint16_t) * keyboard.curr_count);
  keyboard.prev_count = keyboard.curr_count;

  memset(keyboard.curr, 0, keyboard.curr_count * sizeof(uint16_t));
  memcpy(keyboard.curr, current_keys, current_key_count * sizeof(uint16_t));
  keyboard.curr_count = current_key_count;

  overlap = (mouse_b.curr_count > mouse_b.prev_count) ? mouse_b.curr_count
                                                      : mouse_b.prev_count;
  memcpy(mouse_b.prev, mouse_b.curr, sizeof(uint16_t) * overlap);
  mouse_b.prev_count = mouse_b.curr_count;
  memset(mouse_b.curr, 0, sizeof(uint16_t) * mouse_b.curr_count);
  mouse_b.curr_count = 0;

  if (joy_exists) {
    int          count;
    const float* axes = glfwGetJoystickAxes(joystick_id, &count);

    // Joy axes in variable
    memcpy(joy_a.prev, joy_a.curr, sizeof(float) * joy_a.curr_count);
    memset(joy_a.curr, 0, sizeof(float) * joy_a.capacity);

    overlap = (count > joy_a.capacity) ? joy_a.capacity : count;
    memcpy(joy_a.curr, axes, sizeof(float) * overlap);

    for (int i = 0; i < joy_a.capacity; ++i) {
      if (joy_a.curr[i] != 0.f) {
        i_binding_track_callback(i, ASTERA_BINDING_JOYA);
      }
    }

    // Really, not needed just for safety.
    joy_a.prev_count = joy_a.curr_count;
    joy_a.curr_count = count;

    const unsigned char* buttons = glfwGetJoystickButtons(joystick_id, &count);

    overlap = (joy_b.curr_count > joy_b.prev_count) ? joy_b.curr_count
                                                    : joy_b.prev_count;
    memcpy(joy_b.prev, joy_b.curr, sizeof(uint16_t) * overlap);
    memset(joy_b.curr, 0, sizeof(uint16_t) * joy_b.capacity);

    for (int i = 0; i < count; ++i) {
      if (i >= joy_b.capacity) {
        break;
      }

      if (buttons[i] == GLFW_PRESS || buttons[i] == GLFW_REPEAT) {
        if (key_binding_track) {
          i_binding_track_callback(i, ASTERA_BINDING_JOYB);
        }

        joy_b.curr[i] = 1;
      } else {
        joy_b.curr[i] = 0;
      }
    }

    joy_b.prev_count = joy_b.curr_count;
    joy_b.curr_count = count;
  }
}

#endif

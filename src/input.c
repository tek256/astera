#include <astera/input.h>
#include <astera/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For controller vibration
#if defined(WIN32)
#include <windows.h>
#include <xinput.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <unistd.h>
#endif

typedef struct {
  double x, y;
  double dx, dy;
} i_positions;

typedef struct {
  int* prev;
  int* curr;
  int* concurrent;

  uint16_t concurrent_count;
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
  int32_t          id;
  GLFWgamepadstate curr, prev;

  // if we should use gamepad functions
  uint8_t is_gamepad;
} i_joy;

typedef struct {
  char     name[8];
  uint16_t uid;
  uint8_t  state;

  uint16_t source, alt_source;

  uint16_t value;
  uint16_t alt;

  uint8_t type;
  uint8_t alt_type;

  int used : 1;
} i_binding;

struct i_ctx {
  char*    chars;
  uint16_t char_count;

  i_states mouse_b, keyboard;

  // mouse pointer, mouse_last(frame), mouse scrollwheel
  i_positions mouse_p, mouse_l, mouse_s;

  i_binding* bindings;
  i_binding* tracked_binding;

  i_joy*   joys;
  uint16_t joy_count, joy_capacity;

  uint16_t joystick_id;
  uint16_t track_count;
  uint16_t track_cap;
  uint16_t binding_count;
  uint16_t binding_cap;

  uint16_t max_mouse_buttons, max_keys, max_bindings;
  uint16_t max_joy_axes, max_joy_buttons, max_chars;

  uint8_t char_track, joy_exists, binding_track, track_alt;
};

static inline uint8_t i_contains(int val, int* arr, uint16_t count) {
  for (uint16_t i = 0; i < count; ++i) {
    if (arr[i] == val) {
      return 1;
    }
  }
  return 0;
}

static void i_create_s(i_states* states, uint16_t size) {
  states->curr = (int*)calloc(size, sizeof(int));

  if (!states->curr) {
    goto fail;
  }

  states->prev = (int*)calloc(size, sizeof(int));

  if (!states->prev) {
    goto fail;
  }

  states->concurrent = (int*)calloc(size, sizeof(int));

  if (!states->concurrent) {
    goto fail;
  }

  states->curr_count       = 0;
  states->prev_count       = 0;
  states->concurrent_count = 0;
  states->capacity         = size;

  return;
fail:
  if (states->curr)
    free(states->curr);
  if (states->prev)
    free(states->prev);
  if (states->concurrent)
    free(states->concurrent);

  states->curr_count       = 0;
  states->prev_count       = 0;
  states->concurrent_count = 0;
  states->capacity         = 0;
}

static inline i_positions i_create_p() {
  return (i_positions){.dx = 0.0, .dy = 0.0, .x = 0.0, .y = 0.0};
}

i_ctx* i_ctx_create(uint16_t max_mouse_buttons, uint16_t max_keys,
                    uint16_t max_bindings, uint8_t max_joys,
                    uint16_t max_chars) {
  i_ctx* ctx = (i_ctx*)calloc(1, sizeof(i_ctx));

  i_create_s(&ctx->mouse_b, max_mouse_buttons);
  i_create_s(&ctx->keyboard, max_keys);

  ctx->joy_exists = 0;
  ctx->char_track = 0;

  ctx->max_mouse_buttons = max_mouse_buttons;
  ctx->max_keys          = max_keys;

  ctx->mouse_p = i_create_p();
  ctx->mouse_s = i_create_p();

  ctx->binding_count   = 0;
  ctx->tracked_binding = 0;
  ctx->bindings        = (i_binding*)calloc(max_bindings, sizeof(i_binding));
  ctx->max_bindings    = max_bindings;

  ctx->joy_capacity = max_joys;
  ctx->joy_count    = 0;

  if (ctx->joy_capacity != 0) {
    ctx->joys = (i_joy*)malloc(sizeof(i_joy) * ctx->joy_capacity);
    for (uint16_t i = 0; i < ctx->joy_capacity; ++i) {
      ctx->joys[i].id         = -1;
      ctx->joys[i].is_gamepad = 0;
    }
  } else {
    ctx->joys = 0;
  }

  ctx->chars      = (char*)calloc(max_chars, sizeof(char));
  ctx->char_count = 0;

  ctx->max_chars = max_chars;

  if (!ctx->bindings) {
    ASTERA_FUNC_DBG("Unable to alloc space for key bindings.\n");
    i_ctx_destroy(ctx);
    return 0;
  }

  return ctx;
}

uint32_t i_ctx_current_keys(i_ctx* ctx) {
  return ctx->keyboard.curr_count;
}

void i_ctx_debug_out(i_ctx* ctx) {
  printf("Curr:\n");
  for (int i = 0; i < ctx->keyboard.curr_count; ++i) {
    printf("%i ", ctx->keyboard.curr[i]);
  }
  printf("\nConcurr:\n");
  for (int i = 0; i < ctx->keyboard.concurrent_count; ++i) {
    printf("%i ", ctx->keyboard.concurrent[i]);
  }
  printf("\n");
}

void i_ctx_update(i_ctx* ctx) {
  glfwPollEvents();

  ctx->mouse_p.dx = ctx->mouse_p.x - ctx->mouse_l.x;
  ctx->mouse_p.dy = ctx->mouse_p.y - ctx->mouse_l.y;

  ctx->mouse_l.x = ctx->mouse_p.x;
  ctx->mouse_l.y = ctx->mouse_p.y;

  memset(ctx->keyboard.prev, 0, sizeof(int) * ctx->keyboard.prev_count);
  memcpy(ctx->keyboard.prev, ctx->keyboard.curr,
         sizeof(int) * ctx->keyboard.curr_count);
  ctx->keyboard.prev_count = ctx->keyboard.curr_count;

  memset(ctx->keyboard.curr, 0, ctx->keyboard.curr_count * sizeof(int));
  memcpy(ctx->keyboard.curr, ctx->keyboard.concurrent,
         sizeof(int) * ctx->keyboard.concurrent_count);
  ctx->keyboard.curr_count = ctx->keyboard.concurrent_count;

  memset(ctx->mouse_b.prev, 0, sizeof(int) * ctx->mouse_b.prev_count);
  memcpy(ctx->mouse_b.prev, ctx->mouse_b.curr,
         sizeof(int) * ctx->mouse_b.prev_count);
  ctx->mouse_b.prev_count = ctx->mouse_b.curr_count;

  memset(ctx->mouse_b.curr, 0, sizeof(int) * ctx->mouse_b.curr_count);
  memcpy(ctx->mouse_b.curr, ctx->mouse_b.concurrent,
         ctx->mouse_b.concurrent_count * sizeof(int));
  ctx->mouse_b.curr_count = ctx->mouse_b.concurrent_count;

  if (ctx->joy_capacity > 0) {
    if (ctx->joy_capacity != ctx->joy_count) {
      for (uint8_t i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (ctx->joy_count == ctx->joy_capacity)
          break;

        if (ctx->joy_capacity == 0)
          break;

        uint8_t present = (uint8_t)glfwJoystickPresent(i);

        if (present) {
          int8_t open = -1;
          for (uint8_t j = 0; j < ctx->joy_capacity; ++j) {
            if (ctx->joys[j].id == i) {
              open = -1;
              break;
            } else if (ctx->joys[j].id == -1 && !open) {
              open = j;
            }
          }

          if (open > -1) {
            ctx->joys[open].id         = i;
            ctx->joys[open].is_gamepad = glfwJoystickIsGamepad(i);

            ++ctx->joy_count;
          }
        }
      }
    }

    for (uint8_t i = 0; i < ctx->joy_capacity; ++i) {
      i_joy* joy = &ctx->joys[i];

      if (joy->id == -1)
        continue;

      if (joy->is_gamepad) {
        memcpy(joy->prev.buttons, joy->curr.buttons,
               sizeof(unsigned char) * 15);
        memcpy(joy->prev.axes, joy->curr.axes, sizeof(float) * 6);

        glfwGetGamepadState(joy->id, &joy->curr);
      } else {
        memcpy(joy->prev.buttons, joy->curr.buttons,
               sizeof(unsigned char) * 15);
        memcpy(joy->prev.axes, joy->curr.axes, sizeof(float) * 6);

        int          count;
        const float* axes = glfwGetJoystickAxes(joy->id, &count);
        memcpy(joy->curr.axes, axes, ((count > 6) ? 6 : count) * sizeof(float));

        const unsigned char* buttons = glfwGetJoystickButtons(joy->id, &count);
        memcpy(joy->curr.buttons, buttons,
               ((count > 15) ? 15 : count) * sizeof(unsigned char));
      }

      for (uint8_t j = 0; j < 15; ++j) {
        if (joy->curr.buttons[j] != 0.f) {
          i_binding_track_callback(ctx, i, j, ASTERA_BINDING_JOYB);
        }
      }

      for (uint8_t j = 0; j < 6; ++j) {
        if (joy->curr.axes[j] != 0.f) {
          i_binding_track_callback(ctx, i, j, ASTERA_BINDING_JOYA);
        }
      }
    }
  }
}

void i_poll_events() {
  glfwPollEvents();
}

void i_ctx_destroy(i_ctx* ctx) {
  free(ctx->mouse_b.curr);
  free(ctx->mouse_b.prev);
  free(ctx->mouse_b.concurrent);
  free(ctx->keyboard.curr);
  free(ctx->keyboard.prev);
  free(ctx->keyboard.concurrent);

  if (ctx->bindings)
    free(ctx->bindings);

  if (ctx->chars)
    free(ctx->chars);

  if (ctx->joy_capacity) {
    free(ctx->joys);
  }

  free(ctx);
}

int8_t i_joy_create(i_ctx* ctx, uint8_t joy_id) {
  if (ctx->joy_count == ctx->joy_capacity)
    return -1;

  int8_t present = glfwJoystickPresent(joy_id);

  if (!present) {
    return -1;
  }

  int8_t open = -1;
  for (uint8_t i = 0; i < ctx->joy_capacity; ++i) {
    i_joy* joy = &ctx->joys[i];
    if (joy->id == (int8_t)joy_id) {
      return i;
    } else if (joy->id == -1) {
      open = i;
    }
  }

  if (open == -1)
    return -1;

  ctx->joys[open].id         = joy_id;
  ctx->joys[open].is_gamepad = glfwJoystickIsGamepad(joy_id);
  ++ctx->joy_count;
  ctx->joy_exists = 1;

  return open;
}

int8_t i_joy_is_gamepad(i_ctx* ctx, uint8_t joy_id) {
  if (joy_id > ctx->joy_capacity || joy_id == 0)
    return -1;
  return ctx->joys[joy_id - 1].is_gamepad;
}

uint8_t i_joy_connected(i_ctx* ctx) {
  return ctx->joy_count > 0;
}

uint8_t i_joy_first(i_ctx* ctx) {
  if (ctx->joy_count == 0)
    return 0;

  for (uint8_t i = 0; i < ctx->joy_capacity; ++i) {
    if (ctx->joys[i].id != -1) {
      return i + 1;
    }
  }

  return 0;
}

uint8_t i_joy_exists(i_ctx* ctx, uint8_t joy_id) {
  if (joy_id > ctx->joy_capacity || ctx->joy_count == 0) {
    return 0;
  }
  return ctx->joys[joy_id - 1].id != -1;
}

float i_joy_axis_delta(i_ctx* ctx, uint8_t joy_id, uint8_t axis) {
  if (!ctx->joy_exists)
    return 0;
  return ctx->joys[joy_id].curr.axes[axis] - ctx->joys[joy_id].prev.axes[axis];
}

void i_joy_set_vibration(i_ctx* ctx, uint8_t joy_id, float left, float right) {
#if defined(WIN32)
  XINPUT_VIBRATION effect;

  memset(&effect, 0, sizeof(XINPUT_VIBRATION));

  effect.wLeftMotorSpeed  = (WORD)(65535.0f * left);
  effect.wRightMotorSpeed = (WORD)(65535.0f * right);

  (int)(XInputSetState(joy_id, &effect));
#endif
}

const char* i_get_joy_name(uint8_t joy) {
  return glfwGetJoystickName(joy);
}

uint16_t i_get_joy_type(uint8_t joy) {
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

void i_joy_debug_output(i_ctx* ctx, uint8_t joy_id) {
  if (!ctx->joy_exists)
    return;
  i_joy* joy = &ctx->joys[joy_id];

  printf("Buttons: [");
  // Buttons
  for (uint8_t i = 0; i < 15; ++i) {
    printf("%i ", joy->curr.buttons[i]);
  }
  printf("]\n");

  // Axes
  printf("Axes: [");
  for (uint8_t i = 0; i < 6; ++i) {
    printf("%.2f ", joy->curr.axes[i]);
  }
  printf("]\n");
}

void i_joy_destroy(i_ctx* ctx, uint8_t joy_id) {
  if (ctx->joys[joy_id].id != -1) {
    ctx->joys[joy_id].id = -1;
    --ctx->joy_count;
  }
}

float i_joy_axis(i_ctx* ctx, uint8_t joy_id, uint8_t axis) {
  if (!ctx->joy_exists)
    return 0.f;
  return ctx->joys[joy_id].curr.axes[axis];
}

uint8_t i_joy_down(i_ctx* ctx, uint8_t joy_id, uint8_t button) {
  if (!ctx->joy_exists) {
    return 0;
  }
  return ctx->joys[joy_id].curr.buttons[button];
}

uint8_t i_joy_up(i_ctx* ctx, uint8_t joy_id, uint8_t button) {
  if (!ctx->joy_exists)
    return 0;
  return !ctx->joys[joy_id].curr.buttons[button];
}

uint8_t i_joy_clicked(i_ctx* ctx, uint8_t joy_id, uint8_t button) {
  if (!ctx->joy_exists)
    return 0;
  return ctx->joys[joy_id].curr.buttons[button] &&
         !ctx->joys[joy_id].prev.buttons[button];
}

uint8_t i_joy_released(i_ctx* ctx, uint8_t joy_id, uint8_t button) {
  if (!ctx->joy_exists)
    return 0;
  return !ctx->joys[joy_id].curr.buttons[button] &&
         ctx->joys[joy_id].prev.buttons[button];
}

void i_key_callback(i_ctx* ctx, int key, int scancode, int toggle) {
  if (toggle) {
    if (ctx->keyboard.concurrent_count < ctx->keyboard.capacity - 1) {
      if (!i_contains(key, ctx->keyboard.concurrent, ctx->keyboard.capacity)) {
        ctx->keyboard.concurrent[ctx->keyboard.concurrent_count] = key;
        ++ctx->keyboard.concurrent_count;
      }
    }

    if (ctx->binding_track) {
      i_binding_track_callback(ctx, 0, key, ASTERA_BINDING_KEY);
    }
  } else {
    int start = 0;
    for (uint16_t i = 0; i < ctx->keyboard.capacity - 1; ++i) {
      if (ctx->keyboard.concurrent[i] == key) {
        start = 1;
      }

      if (start) {
        ctx->keyboard.concurrent[i] = ctx->keyboard.concurrent[i + 1];
      }
    }

    if (start) {
      ctx->keyboard.concurrent[ctx->keyboard.capacity - 1] = 0;
      --ctx->keyboard.concurrent_count;
    }
  }
}

void i_set_char_tracking(i_ctx* ctx, int tracking) {
  ctx->char_track = tracking;
}

int i_get_char_tracking(i_ctx* ctx) {
  return ctx->char_track;
}

void i_char_callback(i_ctx* ctx, uint32_t c) {
  if (!ctx->char_track) {
    return;
  }

  if (ctx->char_count < ctx->max_chars - 1) {
    ctx->chars[ctx->char_count] = c;
    ctx->char_count++;
  }
}

int i_get_chars(i_ctx* ctx, char* dst, uint16_t count) {
  if (!ctx->char_count || !count || !dst) {
    return 0;
  }

  uint16_t cpy_count = (count > ctx->char_count) ? ctx->char_count : count;
  memcpy(dst, ctx->chars, sizeof(char) * cpy_count);
  return cpy_count;
}

int i_get_char_count(i_ctx* ctx) {
  return ctx->char_count;
}

void i_clear_chars(i_ctx* ctx) {
  memset(ctx->chars, 0, sizeof(char) * ctx->max_chars);
  ctx->char_count = 0;
}

void i_mouse_grab_set(GLFWwindow* window, int grab) {
  if (grab) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

int i_mouse_grab_get(GLFWwindow* window) {
  int value = glfwGetInputMode(window, GLFW_CURSOR);

  if (value == GLFW_CURSOR_DISABLED) {
    return 1;
  } else if (value == GLFW_CURSOR_NORMAL) {
    return 0;
  } else {
    ASTERA_FUNC_DBG("Error: Invalid input mode for GLFW_CURSOR: %i\n", value);
    return 0;
  }
}

void i_mouse_button_callback(i_ctx* ctx, uint16_t button, int8_t toggle) {
  if (toggle) {
    if (ctx->binding_track) {
      i_binding_track_callback(ctx, 0, button, ASTERA_BINDING_MB);
    }

    if (!i_contains(button, ctx->mouse_b.concurrent,
                    ctx->mouse_b.concurrent_count)) {
      if (ctx->mouse_b.concurrent_count < ctx->mouse_b.capacity - 1) {
        ctx->mouse_b.concurrent[ctx->mouse_b.concurrent_count] = button;
        ++ctx->mouse_b.concurrent_count;
      }
    }
  } else {
    int start = 0;
    for (uint32_t i = 0; i < ctx->mouse_b.concurrent_count; ++i) {
      if (ctx->mouse_b.concurrent[i] == button) {
        start = 1;
      }

      if (start) {
        if (i == (uint32_t)(ctx->mouse_b.concurrent_count - 1)) {
          ctx->mouse_b.concurrent[i] = 0;
        } else {
          ctx->mouse_b.concurrent[i] = ctx->mouse_b.concurrent[i + 1];
        }
      }
    }
    --ctx->mouse_b.concurrent_count;
  }
}

void i_mouse_pos_callback(i_ctx* ctx, double x, double y) {
  ctx->mouse_p.x = x;
  ctx->mouse_p.y = y;
}

void i_mouse_scroll_callback(i_ctx* ctx, double sx, double sy) {
  ctx->mouse_s.x += sx;
  ctx->mouse_s.y += sy;
  ctx->mouse_s.dx = sx;
  ctx->mouse_s.dy = sy;
}

void i_scroll_get(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_s.x;
  *y = ctx->mouse_s.y;
}

void i_scroll_get_d(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_s.dx;
  *y = ctx->mouse_s.dy;
}

void i_scroll_reset(i_ctx* ctx) {
  ctx->mouse_s.x  = 0.0;
  ctx->mouse_s.y  = 0.0;
  ctx->mouse_s.dx = 0.0;
  ctx->mouse_s.dy = 0.0;
}

uint16_t i_key_binding_track(i_ctx* ctx) {
  return ctx->binding_track;
}

uint16_t i_mouse_down(i_ctx* ctx, uint16_t button) {
  return i_contains(button, ctx->mouse_b.curr, ctx->mouse_b.curr_count);
}

uint16_t i_mouse_up(i_ctx* ctx, uint16_t button) {
  return !i_mouse_down(ctx, button);
}

uint16_t i_mouse_clicked(i_ctx* ctx, uint16_t button) {
  return i_contains(button, ctx->mouse_b.concurrent, ctx->mouse_b.curr_count) &&
         !i_contains(button, ctx->mouse_b.prev, ctx->mouse_b.prev_count);
}

uint16_t i_mouse_released(i_ctx* ctx, uint16_t button) {
  return !i_contains(button, ctx->mouse_b.concurrent,
                     ctx->mouse_b.curr_count) &&
         i_contains(button, ctx->mouse_b.prev, ctx->mouse_b.prev_count);
}

double i_scroll_get_x(i_ctx* ctx) {
  return ctx->mouse_s.x;
}

double i_scroll_get_y(i_ctx* ctx) {
  return ctx->mouse_s.y;
}

double i_scroll_get_dx(i_ctx* ctx) {
  return ctx->mouse_s.dx;
}

double i_scroll_get_dy(i_ctx* ctx) {
  return ctx->mouse_s.dy;
}

void i_mouse_get_pos(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_p.x;
  *y = ctx->mouse_p.y;
}

double i_mouse_get_x(i_ctx* ctx) {
  return ctx->mouse_p.x;
}

double i_mouse_get_y(i_ctx* ctx) {
  return ctx->mouse_p.y;
}

void i_mouse_get_delta(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_p.dx;
  *y = ctx->mouse_p.dy;
}

double i_mouse_get_dx(i_ctx* ctx) {
  return ctx->mouse_p.dx;
}

double i_mouse_get_dy(i_ctx* ctx) {
  return ctx->mouse_p.dy;
}

uint16_t i_key_down(i_ctx* ctx, uint16_t key) {
  return i_contains(key, ctx->keyboard.concurrent,
                    ctx->keyboard.concurrent_count);
}

uint16_t i_key_up(i_ctx* ctx, uint16_t key) {
  return !i_key_down(ctx, key);
}

uint16_t i_key_clicked(i_ctx* ctx, uint16_t key) {
  return i_contains(key, ctx->keyboard.curr, ctx->keyboard.curr_count) &&
         !i_contains(key, ctx->keyboard.prev, ctx->keyboard.prev_count);
}

uint16_t i_key_released(i_ctx* ctx, uint16_t key) {
  return i_key_up(ctx, key) &&
         i_contains(key, ctx->keyboard.prev, ctx->keyboard.prev_count);
}

int i_any_event(i_ctx* ctx) {
  for (int i = 0; i < ctx->keyboard.curr_count; ++i) {
    if (ctx->keyboard.curr[i] != 0) {
      return 1;
    }
  }

  for (int i = 0; i < ctx->mouse_b.curr_count; ++i) {
    if (ctx->mouse_b.curr[i] != 0) {
      return 1;
    }
  }

  if (ctx->joy_capacity && ctx->joy_count) {
    for (uint8_t i = 0; i < ctx->joy_capacity; ++i) {
      if (ctx->joys[i].id != -1) {
        for (uint8_t j = 0; j < 15; ++j) {
          if (ctx->joys[i].curr.buttons[j] != 0) {
            return 1;
          }
        }

        for (uint8_t j = 0; j < 6; ++j) {
          if (ctx->joys[i].curr.buttons[j] != 0.f) {
            return 1;
          }
        }
      }
    }
  }

  return 0;
}

uint16_t i_binding_add(i_ctx* ctx, const char* name, int value, int type) {
  if (ctx->binding_count > 0) {
    for (int i = 0; i < ctx->binding_count; ++i) {
      if (strncmp(ctx->bindings[i].name, name, 8) == 0) {
        ctx->bindings[i].value = value;
        ctx->bindings[i].type  = type;
        return i + 1;
      }
    }

    if (ctx->binding_count == ctx->max_bindings) {
      ASTERA_FUNC_DBG("Unable to add more key bindings.\n");
      return 0;
    }
  }

  strncpy(ctx->bindings[ctx->binding_count].name, name, 7 * sizeof(char));
  ctx->bindings[ctx->binding_count].value = value;
  ctx->bindings[ctx->binding_count].type  = type;
  ++ctx->binding_count;

  return ctx->binding_count;
}

uint16_t i_binding_add_alt(i_ctx* ctx, const char* name, int value, int type) {
  for (int i = 0; i < ctx->binding_count; ++i) {
    if (strncmp(name, ctx->bindings[i].name, 8) == 0) {
      ctx->bindings[i].alt      = value;
      ctx->bindings[i].alt_type = type;
      return i + 1;
    }
  }

  return 0;
}

uint16_t i_enable_binding_track(i_ctx* ctx, const char* key_binding,
                                uint8_t alt) {
  uint16_t binding_id = 0;
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, 8)) {
      ctx->tracked_binding = &ctx->bindings[i];
      binding_id           = i + 1;
      break;
    }
  }

  if (ctx->tracked_binding != NULL) {
    ctx->binding_track = 1;
    ctx->track_alt     = alt;
  }

  return binding_id;
}

uint16_t i_enable_bindingi_track(i_ctx* ctx, uint16_t binding_id, uint8_t alt) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  ctx->tracked_binding = &ctx->bindings[binding_id - 1];

  if (ctx->tracked_binding != NULL) {
    ctx->binding_track = 1;
    ctx->track_alt     = alt;
  }

  return binding_id;
}

uint16_t i_bindingi_add_alt(i_ctx* ctx, uint16_t binding_id, int value,
                            int type) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  ctx->bindings[binding_id - 1].alt      = value;
  ctx->bindings[binding_id - 1].alt_type = type;

  return binding_id;
}

uint16_t i_binding_get_id(i_ctx* ctx, const char* name) {
  for (uint16_t i = 0; i < ctx->binding_count; ++i) {
    if (!strcmp(ctx->bindings[i].name, name)) {
      return i + 1;
    }
  }

  return 0;
}

uint16_t i_binding_track(i_ctx* ctx) {
  return ctx->binding_track;
}

void i_binding_track_callback(i_ctx* ctx, int source, int value, int type) {
  if (!ctx->binding_track)
    return;

  if (ctx->tracked_binding != NULL) {
    if (ctx->track_alt) {
      ctx->tracked_binding->alt        = value;
      ctx->tracked_binding->alt_type   = type;
      ctx->tracked_binding->alt_source = source;
    } else {
      ctx->tracked_binding->value  = value;
      ctx->tracked_binding->type   = type;
      ctx->tracked_binding->source = source;
    }
  }

  ctx->binding_track = 0;
}

uint16_t i_binding_get_type(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, 8)) {
      return ctx->bindings[i].type;
    }
  }
  return 0;
}

uint16_t i_binding_get_alt_type(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; ++i) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, 8)) {
      return ctx->bindings[i].alt_type;
    }
  }
  return 0;
}

uint8_t i_binding_clicked(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      int val = 0;
      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_clicked(ctx, ctx->bindings[i].source,
                              ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          return 0;
      }

      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_clicked(ctx, ctx->bindings[i].alt_source,
                              ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          return 0;
      }

      return val;
    }
  }

  return 0;
}

uint8_t i_binding_released(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      if (ctx->bindings[i].type == ASTERA_BINDING_JOYA) {
        return 0;
      }

      int val = 0;
      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_released(ctx, ctx->bindings[i].source,
                               ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          return 0;
      }

      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_released(ctx, ctx->bindings[i].alt_source,
                               ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          return 0;
      }

      return val;
    }
  }

  return 0;
}

uint8_t i_binding_down(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      if (ctx->bindings[i].type == ASTERA_BINDING_JOYA) {
        return 0;
      }

      int val = 0;
      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val =
              i_joy_down(ctx, ctx->bindings[i].source, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].source,
                            ctx->bindings[i].value) < 0.f)
                    ? 1
                    : 0;
          break;
      }
      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_down(ctx, ctx->bindings[i].alt_source,
                           ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].alt_source,
                            ctx->bindings[i].alt) < 0.f)
                    ? 1
                    : 0;
          break;
      }

      return val;
    }
  }

  return 0;
}

uint8_t i_binding_up(i_ctx* ctx, const char* key_binding) {
  return !i_binding_down(ctx, key_binding);
}

float i_binding_val(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      float val = 0.f;
      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_MB:
          val = (i_mouse_down(ctx, ctx->bindings[i].value)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_KEY:
          val = (i_key_down(ctx, ctx->bindings[i].value)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_JOYA:
          val =
              i_joy_axis(ctx, ctx->bindings[i].source, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYB:
          val =
              (i_joy_down(ctx, ctx->bindings[i].source, ctx->bindings[i].value))
                  ? 1.0f
                  : 0.0f;
          break;
      }

      if (val != 0.f) {
        return val;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_MB:
          val = (i_mouse_down(ctx, ctx->bindings[i].alt)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_KEY:
          val = (i_key_down(ctx, ctx->bindings[i].alt)) ? 1.0f : 0.0f;
          break;
        case ASTERA_BINDING_JOYA:
          val = i_joy_axis(ctx, ctx->bindings[i].source, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYB:
          val = (i_joy_down(ctx, ctx->bindings[i].source, ctx->bindings[i].alt))
                    ? 1.0f
                    : 0.0f;
          break;
      }

      return val;
    }
  }

  return 0.0f;
}

uint8_t i_binding_defined(i_ctx* ctx, const char* key_binding) {
  if (!key_binding) {
    return 0;
  }

  if (ctx->binding_count > 0) {
    for (int i = 0; i < ctx->binding_count; ++i) {
      int len = strlen(ctx->bindings[i].name);
      if (len != 0) {
        len = (len > 8) ? 8 : len;
        if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
          return i + 1;
        }
      }
    }
  }
  return 0;
}

uint16_t i_bindingi_get_type(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  return ctx->bindings[binding_id - 1].type;
}

uint16_t i_bindingi_get_alt_type(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }
  return ctx->bindings[binding_id - 1].alt_type;
}

uint8_t i_bindingi_clicked(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  i_binding binding = ctx->bindings[binding_id - 1];

  uint8_t val = 0;

  switch (binding.type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_clicked(ctx, binding.source, binding.value);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_clicked(ctx, binding.value);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_clicked(ctx, binding.value);
      break;
  }

  if (val) {
    return 1;
  }

  switch (binding.alt_type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_clicked(ctx, binding.alt_source, binding.alt);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_clicked(ctx, binding.alt);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_clicked(ctx, binding.alt);
      break;
  }

  return val;
}

uint8_t i_bindingi_released(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  i_binding binding = ctx->bindings[binding_id - 1];
  uint8_t   val     = 0;

  switch (binding.type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_released(ctx, binding.source, binding.value);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_released(ctx, binding.value);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_released(ctx, binding.value);
      break;
  }

  if (val) {
    return 1;
  }

  switch (binding.alt_type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_released(ctx, binding.alt_source, binding.alt);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_released(ctx, binding.alt);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_released(ctx, binding.alt);
      break;
  }

  return val;
}

uint8_t i_bindingi_down(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  i_binding binding = ctx->bindings[binding_id - 1];
  uint8_t   val     = 0;

  switch (binding.type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_down(ctx, binding.source, binding.value);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_down(ctx, binding.value);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_down(ctx, binding.value);
      break;
    case ASTERA_BINDING_JOYA:
      val = (i_joy_axis(ctx, binding.source, binding.value) < 0.f) ? 1 : 0;
      break;
  }

  if (val) {
    return 1;
  }

  switch (binding.alt_type) {
    case ASTERA_BINDING_JOYB:
      val = i_joy_down(ctx, binding.alt_source, binding.alt);
      break;
    case ASTERA_BINDING_KEY:
      val = i_key_down(ctx, binding.alt);
      break;
    case ASTERA_BINDING_MB:
      val = i_mouse_down(ctx, binding.alt);
      break;
    case ASTERA_BINDING_JOYA:
      val = (i_joy_axis(ctx, binding.alt_source, binding.alt) < 0.f) ? 1 : 0;
      break;
  }

  return val;
}

uint8_t i_bindingi_up(i_ctx* ctx, uint16_t binding_id) {
  return !i_bindingi_down(ctx, binding_id);
}

float i_bindingi_val(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  i_binding binding = ctx->bindings[binding_id - 1];
  float     val     = 0.f;

  switch (binding.type) {
    case ASTERA_BINDING_MB:
      val = (i_mouse_down(ctx, binding.value)) ? 1.0f : 0.0f;
      break;
    case ASTERA_BINDING_KEY:
      val = (i_key_down(ctx, binding.value)) ? 1.0f : 0.0f;
      break;
    case ASTERA_BINDING_JOYA:
      val = i_joy_axis(ctx, binding.source, binding.value);
      break;
    case ASTERA_BINDING_JOYB:
      val = (i_joy_down(ctx, binding.source, binding.value)) ? 1.0f : 0.0f;
      break;
  }

  if (val != 0.f) {
    return val;
  }

  switch (binding.alt_type) {
    case ASTERA_BINDING_MB:
      val = (i_mouse_down(ctx, binding.alt)) ? 1.0f : 0.0f;
      break;
    case ASTERA_BINDING_KEY:
      val = (i_key_down(ctx, binding.alt)) ? 1.0f : 0.0f;
      break;
    case ASTERA_BINDING_JOYA:
      val = i_joy_axis(ctx, binding.source, binding.alt);
      break;
    case ASTERA_BINDING_JOYB:
      val = (i_joy_down(ctx, binding.source, binding.alt)) ? 1.0f : 0.0f;
      break;
  }

  return val;
}

uint8_t i_bindingi_defined(i_ctx* ctx, uint16_t binding_id) {
  if (binding_id > ctx->binding_count + 1) {
    return 0;
  }

  return 1;
}

#ifndef INPUT_C
#define INPUT_C

#include <astera/input.h>
#include <astera/debug.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
  char     name[ASTERA_KB_NAMELEN];
  uint16_t uid;
  uint8_t  state;

  uint16_t value;
  uint16_t alt;

  uint8_t type;
  uint8_t alt_type;

  int used : 1;
} i_binding;

struct i_ctx {
  char*    chars;
  uint16_t char_count;

  i_states  joy_b, mouse_b, keyboard;
  i_statesf joy_a;

  // mouse pointer, mouse_last(frame), mouse scrollwheel
  i_positions mouse_p, mouse_l, mouse_s;

  i_binding* bindings;
  i_binding* tracked_binding;

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
  states->curr = (int*)malloc(sizeof(int) * size);

  if (!states->curr) {
    goto fail;
  }

  states->prev = (int*)malloc(sizeof(int) * size);

  if (!states->prev) {
    goto fail;
  }

  states->concurrent = (int*)malloc(sizeof(int) * size);

  if (!states->concurrent) {
    goto fail;
  }

  memset(states->curr, 0, sizeof(int) * size);
  memset(states->prev, 0, sizeof(int) * size);
  memset(states->concurrent, 0, sizeof(int) * size);

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

static void i_create_sf(i_statesf* dst, uint16_t size) {
  dst->curr = (float*)malloc(sizeof(float) * size);

  if (!dst->curr)
    goto fail;

  dst->prev = (float*)malloc(sizeof(float) * size);

  if (!dst->prev)
    goto fail;

  memset(dst->curr, 0, sizeof(float) * size);
  memset(dst->prev, 0, sizeof(float) * size);

  dst->prev_count = 0;
  dst->curr_count = 0;
  dst->capacity   = size;

  return;
fail:
  if (dst->curr)
    free(dst->curr);
  if (dst->prev)
    free(dst->prev);
}

static inline i_positions i_create_p() {
  return (i_positions){.dx = 0.0, .dy = 0.0, .x = 0.0, .y = 0.0};
}

i_ctx* i_ctx_create(uint16_t max_mouse_buttons, uint16_t max_keys,
                    uint16_t max_bindings, uint16_t max_joy_axes,
                    uint16_t max_joy_buttons, uint16_t max_chars) {
  i_ctx* ctx = (i_ctx*)malloc(sizeof(i_ctx));

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
  ctx->bindings        = (i_binding*)malloc(sizeof(i_binding) * max_bindings);
  memset(ctx->bindings, 0, sizeof(i_binding) * max_bindings);
  ctx->max_bindings = max_bindings;

  ctx->max_joy_axes    = max_joy_axes;
  ctx->max_joy_buttons = max_joy_buttons;

  ctx->chars      = (char*)malloc(sizeof(char) * max_chars);
  ctx->char_count = 0;

  ctx->max_chars = max_chars;

  if (!ctx->bindings) {
    ASTERA_DBG("i_ctx_create: Unable to malloc space for key bindings.\n");
    i_ctx_destroy(ctx);
    return 0;
  }

  return ctx;
}

void i_ctx_update(i_ctx* ctx) {
  glfwPollEvents();

  ctx->mouse_p.dx = ctx->mouse_p.x - ctx->mouse_l.x;
  ctx->mouse_p.dy = ctx->mouse_p.y - ctx->mouse_l.y;

  ctx->mouse_l.x = ctx->mouse_p.x;
  ctx->mouse_l.y = ctx->mouse_p.y;

  if (!ctx->joy_exists && !ctx->max_joy_buttons > 0) {
    for (uint8_t i = 0; i < GLFW_JOYSTICK_LAST; ++i) {
      int8_t present = glfwJoystickPresent(i);
      if (i != ctx->joystick_id && present) {
        i_joy_create(ctx, i);
        break;
      }
    }
  }

  memset(ctx->keyboard.prev, 0, sizeof(int) * ctx->keyboard.prev_count);
  memcpy(ctx->keyboard.prev, ctx->keyboard.curr,
         sizeof(uint16_t) * ctx->keyboard.curr_count);
  ctx->keyboard.prev_count = ctx->keyboard.curr_count;

  memset(ctx->keyboard.curr, 0, ctx->keyboard.curr_count * sizeof(int));
  memcpy(ctx->keyboard.curr, ctx->keyboard.concurrent,
         sizeof(uint16_t) * ctx->keyboard.concurrent_count);
  ctx->keyboard.curr_count = ctx->keyboard.concurrent_count;

  memset(ctx->mouse_b.prev, 0, sizeof(int) * ctx->mouse_b.prev_count);
  memcpy(ctx->mouse_b.prev, ctx->mouse_b.curr,
         sizeof(uint16_t) * ctx->mouse_b.prev_count);
  ctx->mouse_b.prev_count = ctx->mouse_b.curr_count;

  memset(ctx->mouse_b.curr, 0, sizeof(int) * ctx->mouse_b.curr_count);
  memcpy(ctx->mouse_b.curr, ctx->mouse_b.concurrent,
         ctx->mouse_b.concurrent_count * sizeof(int));
  ctx->mouse_b.curr_count = ctx->mouse_b.concurrent_count;

  if (ctx->joy_exists) {
    int          count;
    const float* axes = glfwGetJoystickAxes(ctx->joystick_id, &count);

    // Joy axes in variable
    memset(ctx->joy_a.prev, 0, sizeof(float) * ctx->joy_a.capacity);
    memcpy(ctx->joy_a.prev, ctx->joy_a.curr,
           sizeof(float) * ctx->joy_a.curr_count);
    memset(ctx->joy_a.curr, 0, sizeof(float) * ctx->joy_a.capacity);

    memcpy(ctx->joy_a.curr, axes,
           sizeof(float) *
               ((count > ctx->joy_a.capacity) ? ctx->joy_a.capacity : count));

    for (int i = 0; i < ctx->joy_a.capacity; ++i) {
      if (ctx->joy_a.curr[i] != 0.f) {
        i_binding_track_callback(ctx, i, ASTERA_BINDING_JOYA);
      }
    }

    // Really, not needed just for safety.
    ctx->joy_a.prev_count = ctx->joy_a.curr_count;
    ctx->joy_a.curr_count = count;

    const unsigned char* buttons =
        glfwGetJoystickButtons(ctx->joystick_id, &count);

    memcpy(ctx->joy_b.prev, ctx->joy_b.curr,
           sizeof(uint16_t) * ctx->joy_b.curr_count);
    memset(ctx->joy_b.curr, 0, sizeof(int) * ctx->joy_b.capacity);

    for (int i = 0; i < count; ++i) {
      if (i >= ctx->joy_b.capacity) {
        break;
      }

      if (buttons[i] == GLFW_PRESS || buttons[i] == GLFW_REPEAT) {
        if (ctx->binding_track) {
          i_binding_track_callback(ctx, i, ASTERA_BINDING_JOYB);
        }

        ctx->joy_b.curr[i] = 1;
      } else {
        ctx->joy_b.curr[i] = 0;
      }
    }

    ctx->joy_b.prev_count = ctx->joy_b.curr_count;
    ctx->joy_b.curr_count = count;
  }
}

void i_poll_events() { glfwPollEvents(); }

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

  if (ctx->joy_exists) {
    if (ctx->joy_b.curr)
      free(ctx->joy_b.curr);
    if (ctx->joy_b.prev)
      free(ctx->joy_b.prev);
    if (ctx->joy_b.concurrent)
      free(ctx->joy_b.concurrent);
    if (ctx->joy_a.curr)
      free(ctx->joy_a.curr);
    if (ctx->joy_a.prev)
      free(ctx->joy_a.prev);
  }
}

void i_joy_create(i_ctx* ctx, uint16_t joy_id) {
  int8_t present = glfwJoystickPresent(joy_id);

  if (!present) {
    return;
  }

  i_create_sf(&ctx->joy_a, ctx->max_joy_axes);

  i_create_s(&ctx->joy_b, ctx->max_joy_buttons);

  ctx->joystick_id = joy_id;
  ctx->joy_exists  = 1;
}

int i_joy_connected(i_ctx* ctx) { return ctx->joy_exists; }

float i_joy_axis_delta(i_ctx* ctx, uint16_t axis) {
  if (!ctx->joy_exists)
    return 0;
  return ctx->joy_a.curr[axis] - ctx->joy_a.prev[axis];
}

void i_get_joy_buttons(i_ctx* ctx, uint16_t* dst, int count) {
  if (!ctx->joy_exists)
    return;
  int cpy_count = (count > ctx->max_joy_buttons) ? ctx->max_joy_buttons : count;
  memcpy(dst, ctx->joy_b.curr, cpy_count * sizeof(uint16_t));
}

void i_get_joy_axes(i_ctx* ctx, float* dst, int count) {
  if (!ctx->joy_exists)
    return;
  int cpy_count = (count > ctx->max_joy_axes) ? ctx->max_joy_axes : count;
  memcpy(dst, ctx->joy_a.curr, cpy_count * sizeof(float));
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

void i_joy_destroy(i_ctx* ctx, uint16_t joy_id) {
  if (joy_id == ctx->joystick_id) {
    if (ctx->joy_exists) {
      ctx->joy_exists = 0;
      // TODO Free joystick axes / buttons
    }
  }
}

float i_joy_axis(i_ctx* ctx, uint16_t axis) {
  if (!ctx->joy_exists)
    return 0.f;
  return ctx->joy_a.curr[axis];
}

uint16_t i_joy_down(i_ctx* ctx, uint16_t button) {
  if (!ctx->joy_exists)
    return 0;
  return ctx->joy_b.curr[button];
}

uint16_t i_joy_up(i_ctx* ctx, uint16_t button) {
  if (!ctx->joy_exists)
    return 0;
  return !ctx->joy_b.curr[button];
}

uint16_t i_joy_clicked(i_ctx* ctx, uint16_t button) {
  if (!ctx->joy_exists)
    return 0;
  return ctx->joy_b.curr[button] && !ctx->joy_b.prev[button];
}

uint16_t i_joy_released(i_ctx* ctx, uint16_t button) {
  if (!ctx->joy_exists)
    return 0;
  return !ctx->joy_b.curr[button] && ctx->joy_b.prev[button];
}

// AHHHH
// ok
// so it's just not catching up because it's dumping repeat keys into it
// (looked at the old version of this and that's why)
void i_key_callback(i_ctx* ctx, int key, int scancode, int toggle) {
  if (toggle) {
    if (ctx->keyboard.concurrent_count < ctx->keyboard.capacity - 1) {
      if (!i_contains(key, ctx->keyboard.concurrent, ctx->keyboard.capacity)) {
        ctx->keyboard.concurrent[ctx->keyboard.concurrent_count] = key;
        ++ctx->keyboard.concurrent_count;
      }
    }

    if (ctx->binding_track) {
      i_binding_track_callback(ctx, key, ASTERA_BINDING_KEY);
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

    ctx->keyboard.concurrent[ctx->keyboard.capacity - 1] = 0;
    --ctx->keyboard.concurrent_count;
  }
}

void i_set_char_tracking(i_ctx* ctx, int tracking) {
  ctx->char_track = tracking;
}

int i_get_char_tracking(i_ctx* ctx) { return ctx->char_track; }

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

int i_get_char_count(i_ctx* ctx) { return ctx->char_count; }

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
    ASTERA_DBG("Error: Invalid input mode for GLFW_CURSOR: %i\n", value);
    return 0;
  }
}

void i_mouse_button_callback(i_ctx* ctx, uint16_t button, int8_t toggle) {
  if (toggle) {
    if (ctx->binding_track) {
      i_binding_track_callback(ctx, button, ASTERA_BINDING_MB);
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
        if (i == ctx->mouse_b.concurrent_count - 1) {
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
  *x = ctx->mouse_s.dx;
  *y = ctx->mouse_s.dy;
}

void i_scroll_reset(i_ctx* ctx) {
  ctx->mouse_s.x = 0.0;
  ctx->mouse_s.y = 0.0;
}

uint16_t i_key_binding_track(i_ctx* ctx) { return ctx->binding_track; }

uint16_t i_mouse_down(i_ctx* ctx, uint16_t button) {
  return i_contains(button, ctx->mouse_b.curr, ctx->mouse_b.curr_count);
}

uint16_t i_mouse_up(i_ctx* ctx, uint16_t button) {
  return !i_contains(button, ctx->mouse_b.curr, ctx->mouse_b.curr_count);
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

double i_scroll_get_x(i_ctx* ctx) { return ctx->mouse_s.x; }

double i_scroll_get_y(i_ctx* ctx) { return ctx->mouse_s.y; }

void i_mouse_get_pos(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_p.x;
  *y = ctx->mouse_p.y;
}

double i_mouse_get_x(i_ctx* ctx) { return ctx->mouse_p.x; }

double i_mouse_get_y(i_ctx* ctx) { return ctx->mouse_p.y; }

void i_mouse_get_delta(i_ctx* ctx, double* x, double* y) {
  *x = ctx->mouse_p.dx;
  *y = ctx->mouse_p.dy;
}

double i_mouse_get_dx(i_ctx* ctx) { return ctx->mouse_p.dx; }

double i_mouse_get_dy(i_ctx* ctx) { return ctx->mouse_p.dy; }

uint16_t i_key_down(i_ctx* ctx, uint16_t key) {
  return i_contains(key, ctx->keyboard.concurrent,
                    ctx->keyboard.concurrent_count);
}

uint16_t i_key_up(i_ctx* ctx, uint16_t key) {
  return !i_contains(key, ctx->keyboard.concurrent,
                     ctx->keyboard.concurrent_count);
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

  if (ctx->joy_exists) {
    for (int i = 0; i < ctx->joy_b.curr_count; ++i) {
      if (ctx->joy_b.curr[i] != 0) {
        return 1;
      }
    }
  }

  return 0;
}

void i_binding_add(i_ctx* ctx, const char* name, int value, int type) {
  if (ctx->binding_count > 0) {
    for (int i = 0; i < ctx->binding_count; ++i) {
      if (strncmp(ctx->bindings[i].name, name, ASTERA_KB_NAMELEN) == 0) {
        ctx->bindings[i].value = value;
        ctx->bindings[i].type  = type;
        return;
      }
    }

    if (ctx->binding_count == ctx->max_bindings) {
      ASTERA_DBG("Unable to add more key bindings.\n");
      return;
    }
  }

  strncpy(ctx->bindings[ctx->binding_count].name, name, ASTERA_KB_NAMELEN);
  ctx->bindings[ctx->binding_count].value = value;
  ctx->bindings[ctx->binding_count].type  = type;
  ++ctx->binding_count;
}

void i_binding_add_alt(i_ctx* ctx, const char* name, int value, int type) {
  for (int i = 0; i < ctx->binding_count; ++i) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (strncmp(name, ctx->bindings[i].name, len) == 0) {
      ctx->bindings[i].alt      = value;
      ctx->bindings[i].alt_type = type;
      return;
    }
  }
}

void i_enable_binding_track(i_ctx* ctx, const char* key_binding, uint8_t alt) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, 8)) {
      ctx->tracked_binding = &ctx->bindings[i];
      break;
    }
  }

  if (ctx->tracked_binding != NULL) {
    ctx->binding_track = 1;
    ctx->track_alt     = alt;
  }
}

uint16_t i_binding_track(i_ctx* ctx) { return ctx->binding_track; }

void i_binding_track_callback(i_ctx* ctx, int value, int type) {
  if (ctx->tracked_binding != NULL) {
    if (ctx->track_alt) {
      ctx->tracked_binding->alt      = value;
      ctx->tracked_binding->alt_type = type;
    } else {
      ctx->tracked_binding->value = value;
      ctx->tracked_binding->type  = type;
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

uint16_t i_binding_clicked(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      int val = 0;
      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_clicked(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(ctx, ctx->bindings[i].value);
          break;
      }

      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_clicked(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_clicked(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_clicked(ctx, ctx->bindings[i].alt);
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_released(i_ctx* ctx, const char* key_binding) {
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
          val = i_joy_released(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(ctx, ctx->bindings[i].value);
          break;
      }

      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_released(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_released(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_released(ctx, ctx->bindings[i].alt);
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_down(i_ctx* ctx, const char* key_binding) {
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
          val = i_joy_down(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].value) < 0.f) ? 1 : 0;
          break;
      }
      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_down(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_down(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_down(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].alt) < 0.f) ? 1 : 0;
          break;
      }

      return val;
    }
  }

  return 0;
}

uint16_t i_binding_up(i_ctx* ctx, const char* key_binding) {
  for (int i = 0; i < ctx->binding_count; i++) {
    int len = strlen(ctx->bindings[i].name);
    len     = (len > 8) ? 8 : len;
    if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
      int val = 0;

      switch (ctx->bindings[i].type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_up(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_up(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_up(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].value) > 0.f) ? 1 : 0;
          break;
      }

      if (val) {
        return 1;
      }

      switch (ctx->bindings[i].alt_type) {
        case ASTERA_BINDING_JOYB:
          val = i_joy_up(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_MB:
          val = i_mouse_up(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_KEY:
          val = i_key_up(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYA:
          val = (i_joy_axis(ctx, ctx->bindings[i].value) > 0.f) ? 1 : 0;
          break;
      }

      return val;
    }
  }
  return 0;
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
          val = i_joy_axis(ctx, ctx->bindings[i].value);
          break;
        case ASTERA_BINDING_JOYB:
          val = (i_joy_down(ctx, ctx->bindings[i].value)) ? 1.0f : 0.0f;
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
          val = i_joy_axis(ctx, ctx->bindings[i].alt);
          break;
        case ASTERA_BINDING_JOYB:
          val = (i_joy_down(ctx, ctx->bindings[i].alt)) ? 1.0f : 0.0f;
          break;
      }

      return val;
    }
  }

  return 0.0f;
}

uint16_t i_binding_defined(i_ctx* ctx, const char* key_binding) {
  if (!key_binding) {
    return 0;
  }

  if (ctx->binding_count > 0) {
    for (int i = 0; i < ctx->binding_count; ++i) {
      int len = strlen(ctx->bindings[i].name);
      if (len != 0) {
        len = (len > 8) ? 8 : len;
        if (!strncmp(ctx->bindings[i].name, key_binding, len)) {
          return 1;
        }
      }
    }
  }
  return 0;
}

#endif

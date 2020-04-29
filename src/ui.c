// TODO: Refactor ui_image functions to ui_img

#include <astera/ui.h>

#if !defined DBG_E
#if defined  ASTERA_DEBUG_OUTPUT
#if defined  ASTERA_DEBUG_INCLUDED
#warning "ASTERA: Standard debug output"
#define DBG_E(fmt, ...) _l(fmt, ##__VA_ARGS_)
#else
#warning "ASTERA: stdio debug output"
#include <stdio.h>
#define DBG_E(fmt, ...) printf(fmt, ##__VA_ARGS_)
#endif
#else
#warning "ASTERA: NO DEBUG OUTPUT"
#define DBG_E(fmt, ...)
#endif
#endif

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <nanovg/nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg/nanovg_gl.h>

#if !defined(UI_DEFAULT_ATTRIB_CAPACITY)
#define UI_DEFAULT_ATTRIB_CAPACITY 24
#endif

#include <assert.h>
#include <stdio.h>

typedef struct {
  vec2        size;
  float       pixel_scale;
  NVGcontext* nvg;
  uint32_t    global_id;

  ui_attrib_map attribs;

  vec2 mouse_pos;
  int  use_mouse : 1;
} ui_ctx;

static ui_ctx g_ui_ctx;

static NVGcolor ui_vec4_color(vec4 v) {
  return nvgRGBA(v[0] * 255.f, v[1] * 255.f, v[2] * 255.f, v[3] * 255.f);
}

uint32_t ui_element_get_uid(ui_element element) {
  if (element.data) {
    return *((uint32_t*)element.data);
  } else {
    return 0;
  }
}

uint8_t ui_init(vec2 size, float pixel_scale, int use_mouse) {
  DBG_E("%s\n", "TEST OUTPUT");

  g_ui_ctx = (ui_ctx){0};

  vec2_dup(g_ui_ctx.size, size);
  g_ui_ctx.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

  g_ui_ctx.pixel_scale = pixel_scale;
  g_ui_ctx.use_mouse   = use_mouse;

  g_ui_ctx.attribs.capacity = UI_DEFAULT_ATTRIB_CAPACITY;
  g_ui_ctx.attribs.attribs =
      malloc(sizeof(ui_attrib_storage) * g_ui_ctx.attribs.capacity);
  g_ui_ctx.attribs.count = 0;

  return g_ui_ctx.nvg != 0;
}

void ui_update(vec2 mouse_pos) {
  if (g_ui_ctx.use_mouse) {
    vec2_dup(g_ui_ctx.mouse_pos, mouse_pos);
  }
}

void ui_destroy() { nvgDeleteGL3(g_ui_ctx.nvg); }

static int ui_hex_number(const char v) {
  if (v >= '0' && v <= '9') {
    return v - 0x30;
  } else {
    switch (v) {
      case 'A':
      case 'a':
        return 10;
      case 'B':
      case 'b':
        return 11;
      case 'C':
      case 'c':
        return 12;
      case 'D':
      case 'd':
        return 13;
      case 'E':
      case 'e':
        return 14;
      case 'F':
      case 'f':
        return 15;
      default:
        return 0;
    }
  }
}

static int ui_hex_multi(const char* v, int len) {
  if (len == 2) {
    return ui_hex_number(v[0]) * 16 + ui_hex_number(v[1]);
  } else if (len == 1) {
    return ui_hex_number(v[0]) * 16 + ui_hex_number(v[0]);
  }
  return -1;
}

void ui_get_color(vec4 val, const char* v) {
  int len    = strlen(v);
  int offset = 0;
  if (len == 4) {
    offset = 1;
    len    = 3;
  } else if (len == 7) {
    offset = 1;
    len    = 6;
  }

  if (len == 3) {
    val[0] = ui_hex_multi(&v[offset], 1) / 255.f;
    val[1] = ui_hex_multi(&v[offset + 1], 1) / 255.f;
    val[2] = ui_hex_multi(&v[offset + 2], 1) / 255.f;
  } else if (len == 6) {
    val[0] = ui_hex_multi(&v[offset], 2) / 255.f;
    val[1] = ui_hex_multi(&v[offset + 2], 2) / 255.f;
    val[2] = ui_hex_multi(&v[offset + 4], 2) / 255.f;
  }
  val[3] = 1.f;
}

void ui_px_to_scale(vec2 dst, vec2 px) {
  vec2 tmp;
  tmp[0] = px[0] / g_ui_ctx.size[0];
  tmp[1] = px[1] / g_ui_ctx.size[1];

  vec2_dup(dst, tmp);
}

void ui_scale_to_px(vec2 dst, vec2 scale) {
  vec2 tmp;
  tmp[0] = scale[0] * g_ui_ctx.size[0];
  tmp[1] = scale[1] * g_ui_ctx.size[1];

  vec2_dup(dst, tmp);
}

void ui_px_from_scale(vec2 dst, vec2 px, vec2 screen) {
  vec2 tmp;
  tmp[0] = px[0] / screen[0];
  tmp[1] = px[1] / screen[1];

  vec2_dup(dst, tmp);
}

void ui_scale_move_px(vec2 dst, vec2 scale, vec2 px) {
  vec2 tmp;
  ui_px_to_scale(tmp, px);
  vec2_add(dst, tmp, scale);
}

void ui_px_move_scale(vec2 dst, vec2 px, vec2 scale) {
  vec2 tmp;
  ui_scale_to_px(tmp, scale);
  vec2_add(dst, tmp, px);
}

void ui_ctx_scale_set(vec2 size_px) { vec2_dup(g_ui_ctx.size, size_px); }

void ui_ctx_scale_get(vec2 dst_px) { vec2_dup(dst_px, g_ui_ctx.size); }

void ui_frame_start() {
  nvgBeginFrame(g_ui_ctx.nvg, g_ui_ctx.size[0], g_ui_ctx.size[1],
                g_ui_ctx.pixel_scale);
}

int8_t ui_is_type(int value, int type) { return ((value) & (type)) == type; }

void ui_frame_end() { nvgEndFrame(g_ui_ctx.nvg); }

static int16_t ui_attrib_size(ui_attrib_type type) {
  switch (type) {
    case UI_INT:
      return sizeof(int32_t);
    case UI_FLOAT:
      return sizeof(float);
    case UI_VEC2:
      return sizeof(vec2);
    case UI_VEC3:
      return sizeof(vec3);
    case UI_VEC4:
      return sizeof(vec4);
    default:
      return sizeof(char);
  }
}

void ui_attrib_set(ui_attrib attrib, void* value, ui_attrib_type type) {
  ui_attrib_map map = g_ui_ctx.attribs;

  int size = ui_attrib_size(type);
  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib) {
      memcpy(map.attribs[i].data, value, size);
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data = malloc(size);
  memcpy(new_data, value, size);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = type;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set3f(ui_attrib attrib, float x, float y, float z) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC3) {
      vec3* data_value = ((vec3*)map.attribs[i].data);
      *data_value[0]   = x;
      *data_value[1]   = y;
      *data_value[2]   = z;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec3));
  vec3* data_value = ((vec3*)map.attribs[map.count].data);
  *data_value[0]   = x;
  *data_value[1]   = y;
  *data_value[2]   = z;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC3;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set3fv(ui_attrib attrib, vec3 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC3) {
      vec3* data_value = ((vec3*)map.attribs[i].data);
      vec3_dup(*data_value, value);
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec3));
  vec3* data_value = (vec3*)map.attribs[map.count].data;
  vec3_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC3;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set4f(ui_attrib attrib, float x, float y, float z, float w) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC4) {
      vec4* data_value = ((vec4*)map.attribs[i].data);
      *data_value[0]   = x;
      *data_value[1]   = y;
      *data_value[2]   = z;
      *data_value[3]   = w;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec4));
  vec4* data_value = ((vec4*)map.attribs[map.count].data);
  *data_value[0]   = x;
  *data_value[1]   = y;
  *data_value[2]   = z;
  *data_value[3]   = w;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC4;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set4fv(ui_attrib attrib, vec4 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC4) {
      vec4* data_value = (vec4*)map.attribs[i].data;
      vec4_dup(*data_value, value);
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec4));
  vec4* data_value = (vec4*)new_data;
  vec4_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC4;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set2f(ui_attrib attrib, float x, float y) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC2) {
      vec2* data_value = (vec2*)map.attribs[i].data;
      *data_value[0]   = x;
      *data_value[1]   = y;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec2));
  vec2* data_value = (vec2*)new_data;
  *data_value[0]   = x;
  *data_value[1]   = y;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC2;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_set2fv(ui_attrib attrib, vec2 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC2) {
      vec2* data_value = (vec2*)map.attribs[i].data;
      vec2_dup(*data_value, value);
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void* new_data   = malloc(sizeof(vec2));
  vec2* data_value = (vec2*)new_data;
  vec2_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_VEC2;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_setf(ui_attrib attrib, float value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_FLOAT) {
      float* data_value = (float*)map.attribs[i].data;
      *data_value       = value;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void*  new_data   = malloc(sizeof(float));
  float* data_value = (float*)new_data;
  *data_value       = value;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_FLOAT;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

void ui_attrib_seti(ui_attrib attrib, int32_t value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_INT) {
      int32_t* data_value = (int32_t*)map.attribs[i].data;
      *data_value         = value;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void*    new_data   = malloc(sizeof(int32_t));
  int32_t* data_value = (int32_t*)new_data;
  *data_value         = value;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type   = UI_INT;
  map.attribs[map.count].data   = new_data;
  map.count++;
}

int8_t ui_attrib_exists(ui_attrib attrib) {
  for (int i = 0; i < g_ui_ctx.attribs.count; ++i) {
    if (g_ui_ctx.attribs.attribs[i].attrib == attrib) {
      return 1;
    }
  }
  return 0;
}

ui_attrib_storage ui_attrib_get(ui_attrib attrib) {
  ui_attrib_map map = g_ui_ctx.attribs;
  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib) {
      return map.attribs[i];
    }
  }
  return (ui_attrib_storage){.type = UI_NONE};
}

int ui_attrib_geti(ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_INT) {
    return -1;
  }
  return *((int*)storage.data);
}

float ui_attrib_getf(ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_FLOAT) {
    return -1.f;
  }
  return *((float*)storage.data);
}

void ui_attrib_get2f(ui_attrib attrib, vec2 dst) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC2) {
    dst[0] = -1.f;
    return;
  }
  vec2_dup(dst, *(vec2*)storage.data);
}

void ui_attrib_get3f(ui_attrib attrib, vec3 dst) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC3) {
    dst[0] = -1.f;
    return;
  }
  vec3_dup(dst, *(vec3*)storage.data);
}

void ui_attrib_get4f(ui_attrib attrib, vec4 dst) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC4) {
    dst[0] = -1.f;
    return;
  }
  vec4_dup(dst, *(vec4*)storage.data);
}

ui_element ui_element_get(void* data, int type) {
  return (ui_element){data, type};
}

void ui_element_center_to(ui_element element, vec2 point) {
  vec2 center;
  vec2_dup(center, point);

  switch (element.type) {
    case UI_BOX: {
      ui_box* box = (ui_box*)element.data;
      center[0] -= box->size[0] * 0.5f;
      center[1] -= box->size[1] * 0.5f;
      vec2_dup(box->position, center);
    } break;
    case UI_TEXT: {
      ui_text* text = (ui_text*)element.data;

      if (ui_is_type(text->align, UI_ALIGN_CENTER) ||
          ui_is_type(text->align, UI_ALIGN_MIDDLE)) {
        vec2_dup(text->position, point);
      } else {
        if (ui_is_type(text->align, UI_ALIGN_LEFT)) {
          text->position[0] = center[0] + (text->bounds[0] * 0.5f);
        } else if (ui_is_type(text->align, UI_ALIGN_RIGHT)) {
          text->position[0] = center[0] - (text->bounds[0] * 0.5f);
        }

        if (ui_is_type(text->align, UI_ALIGN_BOTTOM)) {
          text->position[1] = center[1] + (text->bounds[1] * 0.5f);
        } else if (ui_is_type(text->align, UI_ALIGN_TOP)) {
          text->position[1] = center[1] - (text->bounds[1] * 0.5f);
        }
      }
    } break;
    case UI_BUTTON: {
      ui_button* button = (ui_button*)element.data;
      center[0] -= button->size[0] * 0.5f;
      center[1] -= button->size[1] * 0.5f;
      vec2_dup(button->position, center);
    } break;
    case UI_LINE: {
      ui_line* line = (ui_line*)element.data;

      vec2 delta;
      delta[0] = line->end[0] - line->start[0];
      delta[1] = line->end[1] - line->start[1];

      center[0] -= (delta[0] * 0.5f);
      center[1] -= (delta[1] * 0.5f);

      line->start[0] = center[0];
      line->start[1] = center[1];

      line->end[0] = center[0] + delta[0];
      line->end[1] = center[1] + delta[1];
    } break;
    case UI_DROPDOWN: {
      ui_dropdown* dropdown = (ui_dropdown*)element.data;
      center[0] -= dropdown->size[0] * 0.5f;
      center[1] -= dropdown->size[1] * 0.5f;
      vec2_dup(dropdown->position, center);
    } break;
    case UI_OPTION: {
      ui_option* option = (ui_option*)element.data;
      center[0] -= option->size[0] * 0.5f;
      center[1] -= option->size[1] * 0.5f;
      vec2_dup(option->position, center);
    } break;
    case UI_IMAGE: {
      ui_img* image = (ui_img*)element.data;
      center[0] -= image->size[0] * 0.5f;
      center[1] -= image->size[1] * 0.5f;
      vec2_dup(image->position, center);
    } break;
  }
}

ui_tree ui_tree_create(uint16_t capacity) {
  ui_tree  tree;
  ui_leaf* raw = malloc(sizeof(ui_leaf) * capacity);
  memset(raw, 0, sizeof(ui_leaf) * capacity);
  tree.raw = raw;

  assert(raw != 0);

  tree.start  = 0;
  tree.end    = 0;
  tree.cursor = 0;

  tree.count    = 0;
  tree.capacity = capacity;
  tree.loop     = 0;
  return tree;
}

int16_t ui_element_contains(ui_element element, vec2 point) {
  switch (element.type) {
    case UI_BOX: {
      ui_box* box = (ui_box*)element.data;
      vec2    box_position, box_size;

      ui_scale_to_px(box_position, box->position);
      ui_scale_to_px(box_size, box->size);

      if (point[0] > box_position[0] &&
          point[0] < box_position[0] + box_size[0]) {
        if (point[1] > box_position[1] &&
            point[1] < box_position[1] + box_size[1]) {
          return 1;
        }
      }
    }
      return -1;
    case UI_DROPDOWN: {
      ui_dropdown* dropdown = (ui_dropdown*)element.data;
      vec2         dropdown_position, dropdown_size;
      ui_scale_to_px(dropdown_position, dropdown->position);
      ui_scale_to_px(dropdown_size, dropdown->size);

      float option_height = dropdown_size[1];

      if (dropdown->showing) {
        dropdown_size[1] *= dropdown->option_display;
      }

      vec2 adjusted;
      vec2_sub(adjusted, dropdown_position, dropdown_size);

      if (point[0] > dropdown_position[0] &&
          point[0] < dropdown_position[0] + dropdown_size[0]) {
        if (point[1] > dropdown_position[1] &&
            point[1] < dropdown_position[1] + dropdown_size[1]) {
          if (dropdown->showing) {
            int8_t selection;
            float  y = point[1] - dropdown_position[1];
            y /= option_height;
            selection = (int8_t)y;
            return selection + 1;
          } else {
            return 0;
          }
        }
      }
    }
      return -1;
    case UI_OPTION: {
      ui_option* option = (ui_option*)element.data;

      vec2 option_position, option_size;
      ui_scale_to_px(option_position, option->position);
      ui_scale_to_px(option_size, option->size);

      if (option->use_img) {
        vec2 opt_img_size, opt_img_offset;
        ui_scale_to_px(opt_img_size, option->img_size);
        ui_scale_to_px(opt_img_offset, option->img_offset);

        vec2 img_pos = {opt_img_offset[0] + option_position[0],
                        opt_img_offset[1] + option_position[1]};
        if (point[0] > img_pos[0] && point[0] < img_pos[0] + opt_img_size[0]) {
          if (point[1] > img_pos[1] &&
              point[1] < img_pos[1] + opt_img_size[1]) {
            return 1;
          }
        }
      }

      if (option->use_text) {
        nvgFontFaceId(g_ui_ctx.nvg, option->font);
        nvgFontSize(g_ui_ctx.nvg, option->font_size);
        nvgTextAlign(g_ui_ctx.nvg, option->align);

        vec4 opt_text_bounds;
        nvgTextBounds(g_ui_ctx.nvg, option_position[0], option_position[1],
                      option->text, 0, opt_text_bounds);

        if (point[0] > opt_text_bounds[0] && point[0] < opt_text_bounds[2]) {
          if (point[1] > opt_text_bounds[1] && point[1] < opt_text_bounds[3]) {
            return 1;
          }
        }
      }
    }
      return -1;
    case UI_BUTTON: {
      ui_button* button = (ui_button*)element.data;
      vec2       button_position, button_size;
      ui_scale_to_px(button_position, button->position);
      ui_scale_to_px(button_size, button->size);

      if (point[0] > button_position[0] &&
          point[0] < button_position[0] + button_size[0]) {
        if (point[1] > button_position[1] &&
            point[1] < button_position[1] + button_size[1]) {
          return 1;
        }
      }
    }
      return -1;
    case UI_TEXT: {
      ui_text* text = (ui_text*)element.data;
      nvgFontFaceId(g_ui_ctx.nvg, text->font);
      nvgFontSize(g_ui_ctx.nvg, text->size);
      nvgTextAlign(g_ui_ctx.nvg, text->align);

      vec4 text_bounds;
      vec2 text_position, text_internal_bounds;
      ui_scale_to_px(text_position, text->position);
      ui_scale_to_px(text_internal_bounds, text->bounds);

      if (text->use_box) {
        nvgTextBoxBounds(g_ui_ctx.nvg, text_position[0], text_position[1],
                         text_internal_bounds[0], text->text,
                         (text->use_reveal) ? text->reveal : 0, text_bounds);
      } else {
        nvgTextBounds(g_ui_ctx.nvg, text_position[0], text_position[1],
                      text->text, (text->use_reveal) ? text->reveal : 0,
                      text_bounds);
      }

      if (point[0] > text_bounds[0] && point[0] < text_bounds[2]) {
        if (point[1] > text_bounds[1] && point[1] < text_bounds[3]) {
          return 1;
        }
      }
    }
      return -1;
    case UI_IMAGE: {
      ui_img* img = (ui_img*)element.data;
      vec2    img_position, img_size;
      ui_scale_to_px(img_position, img->position);
      ui_scale_to_px(img_size, img->size);

      if (point[0] > img_position[0] &&
          point[0] < img_position[0] + img_size[0]) {
        if (point[1] > img_position[1] &&
            point[1] < img_position[1] + img_size[1]) {
          return 1;
        }
      }
    }
      return -1;
    default:
      return -1;
  }
}

uint32_t ui_tree_check(ui_tree* tree) {
  if (!g_ui_ctx.use_mouse) {
    return 0;
  }

  vec2 mouse_pos;
  vec2_dup(mouse_pos, g_ui_ctx.mouse_pos);

  assert(tree);
  assert(tree->start);
  ui_leaf* cursor = tree->start;

  int      hover_priority = -1;
  ui_leaf* potential      = 0;

  while (cursor) {
    int current_priority = cursor->priority;

    if (cursor->selectable) {
      int16_t hovered = ui_element_contains(cursor->element, mouse_pos);
      int8_t  current_priority = cursor->priority;

      if (cursor->element.type == UI_DROPDOWN && hovered > -1) {
        ui_dropdown* dropdown = (ui_dropdown*)cursor->element.data;
        if (dropdown->showing) {
          dropdown->cursor = dropdown->start + hovered - 1;
          current_priority += 1;
        } else {
          hovered += 1;
        }
      }

      if (hovered > -1 && current_priority > hover_priority) {
        potential      = cursor;
        hover_priority = current_priority;
      }
    }

    if (cursor->next) {
      cursor = cursor->next;
    } else {
      break;
    }
  }

  if (potential) {
    tree->mouse_hover = potential;
    return potential->uid;
  }
  // this should fix it, I just had it in the wrong scope

  tree->mouse_hover = 0;
  return 0;
}

void ui_text_draw(ui_text* text) {
  nvgBeginPath(g_ui_ctx.nvg);
  nvgFontSize(g_ui_ctx.nvg, text->size);
  nvgFontFaceId(g_ui_ctx.nvg, text->font);
  nvgTextAlign(g_ui_ctx.nvg, text->align);

  if (text->use_spacing) {
    nvgTextLetterSpacing(g_ui_ctx.nvg, text->spacing);
    nvgTextLineHeight(g_ui_ctx.nvg, text->line_height);
  }

  vec2 draw_pos;
  ui_scale_to_px(draw_pos, text->position);

  if (text->use_box) {
    if (text->use_shadow) {
      nvgFontBlur(g_ui_ctx.nvg, text->shadow_size);
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->shadow));
      nvgTextBox(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->bounds[0],
                 text->text, text->reveal);

      nvgFontBlur(g_ui_ctx.nvg, 0.f);
    }

    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->color));
    nvgTextBox(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->bounds[0],
               text->text, text->reveal);
  } else {
    if (text->use_shadow) {
      nvgFontBlur(g_ui_ctx.nvg, text->shadow_size);
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->shadow));
      nvgText(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->text, text->reveal);

      nvgFontBlur(g_ui_ctx.nvg, 0.f);
    }

    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->color));
    nvgText(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->text, text->reveal);
  }
}

void ui_box_draw(ui_box* box, int8_t focused) {
  assert(box);

  vec2 box_position, box_size;
  ui_scale_to_px(box_position, box->position);
  ui_scale_to_px(box_size, box->size);

  nvgBeginPath(g_ui_ctx.nvg);
  if (box->use_border) {
    if (box->border_radius != 0.f) {
      nvgRoundedRect(g_ui_ctx.nvg, box_position[0], box_position[1],
                     box_size[0], box_size[1], box->border_radius);
    } else {
      nvgRect(g_ui_ctx.nvg, box_position[0], box_position[1], box_size[0],
              box_size[1]);
    }

    if (box->border_size > 0.f) {
      nvgStrokeWidth(g_ui_ctx.nvg, box->border_size);
      if (focused) {
        nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(box->hover_border_color));
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(box->hover_bg));
      } else {
        nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(box->border_color));
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(box->bg));
      }
      nvgStroke(g_ui_ctx.nvg);
    }

    nvgFill(g_ui_ctx.nvg);
  } else {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(box->hover_bg));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(box->bg));
    }

    nvgRect(g_ui_ctx.nvg, box_position[0], box_position[1], box_size[0],
            box_size[1]);
    nvgFill(g_ui_ctx.nvg);
  }
}

void ui_button_draw(ui_button* button, int8_t focused) {
  nvgBeginPath(g_ui_ctx.nvg);

  vec2 button_position, button_size;

  ui_scale_to_px(button_position, button->position);
  ui_scale_to_px(button_size, button->size);

  if (button->use_border) {
    if (button->border_radius != 0.f) {
      nvgRoundedRect(g_ui_ctx.nvg, button_position[0], button_position[1],
                     button_size[0], button_size[1], button->border_radius);
    } else {
      nvgRect(g_ui_ctx.nvg, button_position[0], button_position[1],
              button_size[0], button_size[1]);
    }

    if (button->border_size > 0.f) {
      nvgStrokeWidth(g_ui_ctx.nvg, button->border_size);
      if (focused) {
        nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_border_color));
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_bg));
      } else {
        nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(button->border_color));
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->bg));
      }
      nvgStroke(g_ui_ctx.nvg);
    }

    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_bg));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->bg));
    }

    nvgFill(g_ui_ctx.nvg);
  } else {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_bg));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->bg));
    }

    nvgRect(g_ui_ctx.nvg, button_position[0], button_position[1],
            button_size[0], button_size[1]);
    nvgFill(g_ui_ctx.nvg);
  }

  if (button->text) {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_color));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->color));
    }

    nvgFontSize(g_ui_ctx.nvg, button->font_size);
    nvgFontFaceId(g_ui_ctx.nvg, button->font);
    nvgTextAlign(g_ui_ctx.nvg, button->align);
    nvgTextLineHeight(g_ui_ctx.nvg, button->font_size);

    vec2 offset = {0.f, 0.f};

    vec4 text_bounds;
    nvgTextBounds(g_ui_ctx.nvg, 0.f, 0.f, button->text, 0, text_bounds);
    vec2 text_size = {text_bounds[2] - text_bounds[0],
                      text_bounds[3] - text_bounds[1]};

    if (ui_is_type(button->align, UI_ALIGN_LEFT)) {
    } else if (ui_is_type(button->align, UI_ALIGN_MIDDLE)) {
      offset[0] = button_size[0] / 2.f;
    } else if (ui_is_type(button->align, UI_ALIGN_RIGHT)) {
      offset[0] = button_size[0];
    }

    if (ui_is_type(button->align, UI_ALIGN_TOP)) {
    } else if (ui_is_type(button->align, UI_ALIGN_MIDDLE)) {
      offset[1] = (button_size[1] / 2.f);
    } else if (ui_is_type(button->align, UI_ALIGN_BOTTOM)) {
      offset[1] = button_size[1];
    } else {
      offset[1] = text_size[1];
    }

    nvgText(g_ui_ctx.nvg, button_position[0] + offset[0],
            button_position[1] + offset[1], button->text, 0);
  }
}

float ui_dropdown_max_font_size(ui_dropdown dropdown) {
  if (dropdown.option_count == 0) {
    DBG_E("Dropdown doesn't contain any options.\n");
    return -1.f;
  }

  uint32_t longest_option_len = 0;
  int32_t  longest_option     = -1;
  for (uint32_t i = 0; i < dropdown.option_count; ++i) {
    if (!dropdown.options[i])
      continue;

    int32_t option_len = strlen(dropdown.options[i]);
    if (option_len > longest_option_len) {
      longest_option     = (uint32_t)i;
      longest_option_len = option_len;
    }
  }

  if (longest_option == -1) {
    longest_option = 0;
  }

  float current_size = dropdown.font_size;

  vec4 text_bounds = {0.f};
  vec2 dropdown_size;
  ui_scale_to_px(dropdown_size, dropdown.size);

  nvgFontFaceId(g_ui_ctx.nvg, dropdown.font);
  nvgTextAlign(g_ui_ctx.nvg, dropdown.align);
  nvgTextLineHeight(g_ui_ctx.nvg, dropdown_size[1]);

  while (1) {
    nvgFontSize(g_ui_ctx.nvg, current_size);
    nvgTextBounds(g_ui_ctx.nvg, 0.f, 0.f, dropdown.options[longest_option], 0,
                  text_bounds);

    float width  = text_bounds[2] - text_bounds[0];
    float height = text_bounds[3] - text_bounds[1];

    if (width >= dropdown_size[0] || height >= dropdown_size[1]) {
      break;
    } else {
      current_size += 1.f;
    }
  }

  return current_size;
}

void ui_dropdown_draw(ui_dropdown* dropdown, int8_t focused) {
  nvgBeginPath(g_ui_ctx.nvg);

  vec2 dropdown_size, dropdown_position;
  vec2 option_size;
  ui_scale_to_px(dropdown_size, dropdown->size);
  ui_scale_to_px(dropdown_position, dropdown->position);
  ui_scale_to_px(option_size, dropdown->size);

  if (dropdown->showing) {
    dropdown_size[1] *= dropdown->option_display;
  }

  if (dropdown->use_border) {
    if (dropdown->border_size != 0.f) {
      nvgStrokeWidth(g_ui_ctx.nvg, dropdown->border_size);

      if (focused) {
        nvgStrokeColor(g_ui_ctx.nvg,
                       ui_vec4_color(dropdown->hover_border_color));
      } else {
        nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->border_color));
      }

      if (dropdown->border_radius != 0.f) {
        nvgRoundedRect(g_ui_ctx.nvg, dropdown_position[0], dropdown_position[1],
                       dropdown_size[0], dropdown_size[1],
                       dropdown->border_radius);
      } else {
        nvgRect(g_ui_ctx.nvg, dropdown_position[0], dropdown_position[1],
                dropdown_size[0], dropdown_size[1]);
      }

      nvgStroke(g_ui_ctx.nvg);
    }
  }

  if (focused) {
    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->hover_bg));
  } else {
    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->bg));
  }

  if (dropdown->border_radius != 0.f) {
    nvgRoundedRect(g_ui_ctx.nvg, dropdown_position[0], dropdown_position[1],
                   dropdown_size[0], dropdown_size[1], dropdown->border_radius);
  } else {
    nvgRect(g_ui_ctx.nvg, dropdown_position[0], dropdown_position[1],
            dropdown_size[0], dropdown_size[1]);
  }

  nvgFill(g_ui_ctx.nvg);

  // Showing options
  nvgFontSize(g_ui_ctx.nvg, dropdown->font_size);
  nvgFontFaceId(g_ui_ctx.nvg, dropdown->font);
  nvgTextAlign(g_ui_ctx.nvg, dropdown->align);

  if (dropdown->showing) {
    int start = dropdown->start;
    for (int i = 0; i < dropdown->option_display; ++i) {
      int cursor_index = i + start;

      if (cursor_index == dropdown->cursor ||
          cursor_index == dropdown->selected ||
          cursor_index == dropdown->mouse_cursor) {
        if (focused) {
          nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->hover_select_bg));
        } else {
          nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->select_bg));
        }

        if (i + start == dropdown->option_display &&
            dropdown->border_radius != 0.f) {
          nvgRoundedRectVarying(g_ui_ctx.nvg, dropdown_position[0],
                                dropdown_position[1] + (dropdown_size[1] * i),
                                dropdown_size[0], option_size[1], 0.f, 0.f,
                                dropdown->border_radius,
                                dropdown->border_radius);
        } else {
          nvgRect(g_ui_ctx.nvg, dropdown_position[0],
                  dropdown_position[1] + (option_size[1] * i), dropdown_size[0],
                  option_size[1]);
        }

        nvgFill(g_ui_ctx.nvg);

        // Draw overlaying text
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->hover_color));

        vec2 text_offset = {0.f, option_size[1] * i};

        if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
          text_offset[0] += option_size[0] / 2.f;
        } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
          text_offset[0] += option_size[0];
        }

        if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
          text_offset[1] += (option_size[1] / 2.f);
        } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
          text_offset[1] += option_size[1];
        } else { // Should be UI_ALIGN_BASELINE
          text_offset[1] += option_size[1];
        }

        nvgText(g_ui_ctx.nvg, dropdown_position[0] + text_offset[0],
                dropdown_position[1] + text_offset[1],
                dropdown->options[start + i], 0);
      } else {
        nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->color));
        vec2 text_offset = {0.f, option_size[1] * i};

        if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
          text_offset[0] += option_size[0] / 2.f;
        } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
          text_offset[0] += option_size[0];
        }

        if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
          text_offset[1] += (option_size[1] / 2.f);
        } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
          text_offset[1] += option_size[1];
        } else { // Should be UI_ALIGN_BASELINE
          text_offset[1] += option_size[1];
        }

        nvgText(g_ui_ctx.nvg, dropdown_position[0] + text_offset[0],
                dropdown_position[1] + text_offset[1],
                dropdown->options[start + i], 0);
      }
    }
  } else {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->hover_color));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(dropdown->color));
    }

    vec2 text_offset;

    if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
    } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
      text_offset[0] = option_size[0] / 2.f;
    } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
      text_offset[0] = option_size[0];
    }

    if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
    } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE)) {
      text_offset[1] = (option_size[1] / 2.f);
    } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
      text_offset[1] = option_size[1];
    } else { // Should be UI_ALIGN_BASELINE
      text_offset[1] = option_size[1];
    }

    nvgText(g_ui_ctx.nvg, dropdown_position[0] + text_offset[0],
            dropdown_position[1] + text_offset[1],
            dropdown->options[dropdown->selected], 0);
  }
}

void ui_line_draw(ui_line* line) {
  if (!line) {
    return;
  }

  vec2 line_start, line_end;
  ui_scale_to_px(line_start, line->start);
  ui_scale_to_px(line_end, line->end);

  nvgBeginPath(g_ui_ctx.nvg);
  nvgMoveTo(g_ui_ctx.nvg, line_start[0], line_start[1]);
  nvgLineTo(g_ui_ctx.nvg, line_end[0], line_end[1]);
  nvgStrokeWidth(g_ui_ctx.nvg, line->thickness);
  nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(line->color));
  nvgStroke(g_ui_ctx.nvg);
}

void ui_option_draw(ui_option* option, int8_t focused) {
  if (!option) {
    return;
  }

  if (option->use_img) {
    nvgBeginPath(g_ui_ctx.nvg);
    NVGpaint img_paint =
        nvgImagePattern(g_ui_ctx.nvg, 0.f, 0.f, option->img_size[0],
                        option->img_size[1], 0.f, option->img_handle, 1.0);
    nvgRect(g_ui_ctx.nvg, option->img_offset[0] + option->position[0],
            option->img_offset[1] + option->position[1], option->img_size[0],
            option->img_size[1]);
    nvgFillPaint(g_ui_ctx.nvg, img_paint);
    nvgFill(g_ui_ctx.nvg);
  }

  if (option->use_color) {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(option->hover_bg));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(option->bg));
    }
  }

  if (option->use_text) {
    nvgBeginPath(g_ui_ctx.nvg);
    nvgFontFaceId(g_ui_ctx.nvg, option->font);
    nvgFontSize(g_ui_ctx.nvg, option->font_size);
    nvgTextAlign(g_ui_ctx.nvg, option->align);

    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(option->hover_color));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(option->color));
    }

    nvgText(g_ui_ctx.nvg, option->position[0], option->position[1],
            option->text, 0);
  }
}

void ui_image_draw(ui_img* img, int8_t focused) {
  if (!img) {
    return;
  }

  vec2 img_position, img_size;
  ui_scale_to_px(img_position, img->position);
  ui_scale_to_px(img_size, img->size);

  NVGpaint img_paint =
      nvgImagePattern(g_ui_ctx.nvg, img_position[0], img_position[1],
                      img_size[0], img_size[1], 0.f, img->handle, 1.0f);
  nvgBeginPath(g_ui_ctx.nvg);

  if (img->border_size != 0.f) {
    if (focused) {
      nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(img->hover_border_color));
    } else {
      nvgStrokeColor(g_ui_ctx.nvg, ui_vec4_color(img->border_color));
    }

    nvgStrokeWidth(g_ui_ctx.nvg, img->border_size);
    nvgStroke(g_ui_ctx.nvg);
  }

  if (img->border_radius != 0.f) {
    nvgRoundedRect(g_ui_ctx.nvg, img_position[0], img_position[1], img_size[0],
                   img_size[1], img->border_radius);
  } else {
    nvgRect(g_ui_ctx.nvg, img_position[0], img_position[1], img_size[0],
            img_size[1]);
  }

  nvgFillPaint(g_ui_ctx.nvg, img_paint);
  nvgFill(g_ui_ctx.nvg);
}

void ui_im_text_draw(vec2 pos, float font_size, ui_font font, char* text) {
  nvgBeginPath(g_ui_ctx.nvg);
  nvgFontSize(g_ui_ctx.nvg, font_size);
  vec4 im_text_color = {1.f, 1.f, 1.f, 1.f};
  nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(im_text_color));
  nvgFontFaceId(g_ui_ctx.nvg, font);
  nvgText(g_ui_ctx.nvg, pos[0], pos[1], text, 0);
}

ASTERA_API void ui_im_box_draw(vec2 pos, vec2 size, vec4 color) {
  nvgBeginPath(g_ui_ctx.nvg);
  nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(color));
  nvgRect(g_ui_ctx.nvg, pos[0], pos[1], size[0], size[1]);
  nvgFill(g_ui_ctx.nvg);
}

ASTERA_API void ui_im_circle_draw(vec2 pos, float radius, vec4 color) {
  nvgBeginPath(g_ui_ctx.nvg);
  nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(color));
  nvgCircle(g_ui_ctx.nvg, pos[0], pos[1], radius);
  nvgFill(g_ui_ctx.nvg);
}

static int ui_text_fits(ui_text text, float size, vec2 scaled_pos, vec2 bounds,
                        int allow_reveal) {
  nvgFontSize(g_ui_ctx.nvg, size);
  vec4 text_bounds;

  if (text.use_box) {
    nvgTextBoxBounds(
        g_ui_ctx.nvg, scaled_pos[0], scaled_pos[1], text.bounds[0], text.text,
        (text.use_reveal && allow_reveal) ? text.reveal : 0, text_bounds);
  } else {
    nvgTextBounds(g_ui_ctx.nvg, scaled_pos[0], scaled_pos[1], text.text,
                  (text.use_reveal && allow_reveal) ? text.reveal : 0,
                  text_bounds);
  }

  float width  = text_bounds[2] - text_bounds[0];
  float height = text_bounds[3] - text_bounds[1];

  if (width >= bounds[0]) {
    return 0;
  }

  if (height >= bounds[1]) {
    return 0;
  }

  return 1;
}

float ui_text_max_size(ui_text text, vec2 bounds, int allow_reveal) {
  float current_size = text.size;

  vec2 text_position;
  ui_scale_to_px(text_position, text.position);

  nvgFontFaceId(g_ui_ctx.nvg, text.font);
  nvgTextAlign(g_ui_ctx.nvg, text.align);

  if (text.use_spacing) {
    nvgTextLetterSpacing(g_ui_ctx.nvg, text.spacing);
    nvgTextLineHeight(g_ui_ctx.nvg, text.line_height);
  }

  if (!ui_text_fits(text, current_size, text_position, bounds, allow_reveal)) {
    current_size = 1.f;
  }

  while (1) {
    if (!ui_text_fits(text, current_size, text_position, bounds,
                      allow_reveal)) {
      current_size -= 1.f;
      break;
    } else {
      current_size += 1.f;
    }
  }

  return current_size;
}

void ui_tree_destroy(ui_tree* tree) {
  ui_leaf* cursor = tree->start;
  while (cursor) {
    if (cursor->element.data) {
      switch (cursor->element.type) {
        case UI_IMAGE:
          ui_image_destroy((ui_img*)cursor->element.data);
          break;
        default:
          // These types don't need freeing
          break;
      }
    }
    cursor = cursor->next;
  }
  free(tree->raw);
}

uint32_t ui_tree_add(ui_tree* tree, void* data, ui_element_type type,
                     int8_t priority, int8_t selectable) {
  if (!data || !tree) {
    return 0;
  }

  if (tree->count == tree->capacity - 1) {
    DBG_E("No free space in tree.\n");
    return 0;
  }

  ui_leaf* leaf_ptr = &tree->raw[tree->count];
  if (!leaf_ptr) {
    DBG_E("Invalid leaf pointer.\n");
    return 0;
  }

  ++g_ui_ctx.global_id;
  uint32_t uid  = g_ui_ctx.global_id;
  leaf_ptr->uid = uid;

  uint32_t* element_uid = (uint32_t*)data;
  *element_uid          = uid;

  leaf_ptr->selectable = selectable;
  leaf_ptr->priority   = (priority >= 127) ? 126 : priority;

  ui_element* element = &leaf_ptr->element;
  element->data       = data;
  element->type       = type;

  if (!tree->cursor) {
    tree->cursor = leaf_ptr;
  }

  if (!tree->start) {
    tree->start = leaf_ptr;
  }

  if (!tree->end) {
    tree->end      = leaf_ptr;
    leaf_ptr->prev = 0;
  } else {
    leaf_ptr->prev  = tree->end;
    tree->end->next = leaf_ptr;
    tree->end       = leaf_ptr;
  }

  leaf_ptr->next = 0;

  ++tree->count;
  return uid;
}

void ui_tree_print(ui_tree* tree) {
  if (!tree) {
    DBG_E("No tree passed.\n");
    return;
  }

  ui_leaf* cursor = tree->start;
  DBG_E("Tree: ");

  if (!cursor) {
    DBG_E("None\n");
  } else {
    while (cursor) {
      switch (cursor->element.type) {
        case UI_TEXT:
          DBG_E("Text");
          break;
        case UI_BOX:
          DBG_E("Box");
          break;
        case UI_OPTION:
          DBG_E("Option");
          break;
        case UI_BUTTON:
          DBG_E("Button");
          break;
        case UI_DROPDOWN:
          DBG_E("Dropdown");
          break;
        case UI_LINE:
          DBG_E("Line");
          break;
        case UI_IMAGE:
          DBG_E("Image");
          break;
        default:
          DBG_E("Unk");
          break;
      }

      if (cursor->next) {
        DBG_E(" -> ");
        cursor = cursor->next;
      } else {
        break;
      }
    }
    DBG_E("\n");
  }
}

int32_t ui_element_event(ui_tree* tree, uint32_t uid) {
  if (!tree || !tree->start) {
    return 0;
  }

  ui_leaf* cursor = tree->start;
  while (cursor) {
    if (cursor->uid == uid) {
      int32_t event_type = cursor->event;
      cursor->event      = 0;
      return event_type;
    }

    if (!cursor->next) {
      return 0;
    } else {
      cursor = cursor->next;
    }
  }

  return 0;
}

uint32_t ui_tree_get_cursor_id(ui_tree* tree) { return tree->cursor->uid; }

void ui_tree_draw(ui_tree tree) {
  if (!tree.start) {
    DBG_E("No start in tree.\n");
    return;
  }

  ui_leaf* cursor = tree.start;

  uint32_t tree_cursor_uid = (tree.cursor) ? tree.cursor->uid : 0;
  uint32_t tree_hover_id   = (tree.mouse_hover) ? tree.mouse_hover->uid : 0;

  while (cursor) {
    int8_t focused = 0;

    if (g_ui_ctx.use_mouse) {
      focused = cursor->uid == tree_cursor_uid || cursor->uid == tree_hover_id;
    } else {
      focused = cursor->uid == tree_cursor_uid;
    }

    if (cursor->element.data) {
      ui_element element = cursor->element;

      switch (element.type) {
        case UI_TEXT:
          ui_text_draw((ui_text*)element.data);
          break;
        case UI_IMAGE:
          ui_image_draw((ui_img*)element.data, focused);
        case UI_BOX:
          ui_box_draw((ui_box*)element.data, focused);
          break;
        case UI_BUTTON:
          ui_button_draw((ui_button*)element.data, focused);
          break;
        case UI_LINE:
          ui_line_draw((ui_line*)element.data);
          break;
        case UI_DROPDOWN:
          ui_dropdown_draw((ui_dropdown*)element.data, focused);
          break;
        case UI_OPTION:
          ui_option_draw((ui_option*)element.data, focused);
          break;
      }
    }

    if (cursor->next) {
      cursor = cursor->next;
    } else {
      break;
    }
  }
}

uint16_t ui_select_next(ui_tree* tree) {
  if (tree->cursor->next) {
    tree->cursor = tree->cursor->next;
  } else {
    tree->cursor = tree->start;
  }

  return tree->cursor->uid;
}

ui_text ui_text_create(vec2 pos, char* string, float font_size, ui_font font_id,
                       int alignment) {
  ui_text text;

  vec2_dup(text.position, pos);
  text.text   = string;
  text.size   = font_size;
  text.font   = font_id;
  text.align  = alignment;
  text.reveal = NULL;

  if (string) {
    text.text_length = strlen(string);
  } else {
    text.text_length = 0;
  }

  vec4 color, shadow;
  ui_attrib_get4f(UI_TEXT_COLOR, color);
  ui_attrib_get4f(UI_TEXT_SHADOW, shadow);
  float shadow_size = ui_attrib_getf(UI_TEXT_SHADOW_SIZE);
  float spacing     = ui_attrib_getf(UI_TEXT_SPACING);
  float line_height = ui_attrib_getf(UI_TEXT_LINE_HEIGHT);

  if (color[0] > 0.f) {
    vec4_dup(text.color, color);
  } else {
    text.color[0] = 1.f;
    text.color[1] = 1.f;
    text.color[2] = 1.f;
    text.color[3] = 1.f;
  }

  if (spacing > 0.f) {
    text.spacing     = spacing;
    text.use_spacing = 1;
  }

  if (line_height > 0.f) {
    text.line_height = line_height;
    text.use_spacing = 1;
  } else {
    text.line_height = font_size;
  }

  if (shadow[0] >= 0.f) {
    vec4_dup(text.shadow, shadow);

    if (shadow_size == -1.f) {
      text.shadow_size = 3.f;
    }

    text.use_shadow = 1;
  }

  if (shadow_size > 0.f) {
    if (shadow[0] <= 0.f) {
      text.shadow[0] = 0.f;
      text.shadow[1] = 0.f;
      text.shadow[2] = 0.f;
    }

    text.shadow_size = shadow_size;
    text.use_shadow  = 1;
  }

  vec2_clear(text.bounds);

  text.use_box     = 0;
  text.use_reveal  = 0;
  text.use_shadow  = 0;
  text.use_spacing = 0;

  return text;
}

ui_button ui_button_create(vec2 pos, vec2 size, char* text, int text_alignment,
                           float font_size) {
  ui_button button;
  vec2_dup(button.position, pos);
  vec2_dup(button.size, size);

  button.font_size = font_size;
  button.text      = text;
  button.align     = text_alignment;

  vec4 _color, _bg, _color_hover, _bg_hover, _border_color, _border_color_hover;
  float   _border_size, _border_radius;
  int32_t _font;

  ui_attrib_get4f(UI_BUTTON_COLOR, _color);
  ui_attrib_get4f(UI_BUTTON_COLOR_HOVER, _color_hover);
  ui_attrib_get4f(UI_BUTTON_BG, _bg);
  ui_attrib_get4f(UI_BUTTON_BG_HOVER, _bg_hover);
  ui_attrib_get4f(UI_BUTTON_BORDER_COLOR, _border_color);
  ui_attrib_get4f(UI_BUTTON_BORDER_COLOR_HOVER, _border_color_hover);

  _border_size   = ui_attrib_getf(UI_BUTTON_BORDER_SIZE);
  _border_radius = ui_attrib_getf(UI_BUTTON_BORDER_RADIUS);
  _font          = ui_attrib_geti(UI_BUTTON_FONT);

  if (_border_size != -1.f) {
    button.border_size = _border_size;
    button.use_border  = 1;
  } else {
    button.border_size = 0.f;
  }

  if (_border_radius != -1.f) {
    button.border_radius = _border_radius;
    button.use_border    = 1;
  } else {
    button.border_radius = 0.f;
  }

  if (_color[0] != -1.f) {
    vec4_dup(button.color, _color);
  } else {
    vec4_clear(button.color);
  }

  if (_bg[0] != -1.f) {
    vec4_dup(button.bg, _bg);
  } else {
    vec4_clear(button.bg);
  }

  if (_color_hover[0] != -1.f) {
    vec4_dup(button.hover_color, _color_hover);
  } else {
    vec4_clear(button.hover_color);
  }

  if (_bg_hover[0] != -1.f) {
    vec4_dup(button.hover_bg, _bg_hover);
  } else {
    vec4_clear(button.hover_bg);
  }

  if (_border_color[0] != -1.f) {
    vec4_dup(button.border_color, _border_color);
  } else {
    vec4_clear(button.border_color);
  }

  if (_border_color_hover[0] != -1.f) {
    vec4_dup(button.hover_border_color, _border_color_hover);
  } else {
    vec4_clear(button.hover_border_color);
  }

  return button;
}

ui_line ui_line_create(vec2 start, vec2 end, vec4 color, float thickness) {
  ui_line line;

  vec2_dup(line.start, start);
  vec2_dup(line.end, end);
  vec4_dup(line.color, color);

  line.thickness = thickness;

  return line;
}

ui_dropdown ui_dropdown_create(vec2 pos, vec2 size, char** options,
                               int option_count) {
  ui_dropdown dropdown = (ui_dropdown){0};

  if (option_count > 0) {
    char** _options = (char**)malloc(sizeof(char*) * (option_count + 1));
    for (int i = 0; i < option_count; ++i) {
      int str_len = strlen(options[i]);

      _options[i] = (char*)malloc(sizeof(char) * (str_len + 1));
      memcpy(_options[i], options[i], sizeof(char) * (str_len + 1));

      _options[i][str_len] = 0;
    }

    _options[option_count] = 0;

    // we'll do our modifications before assignment then just cast it so it's
    // not read only
    dropdown.options         = (const char**)_options;
    dropdown.option_count    = option_count;
    dropdown.option_capacity = option_count + 1;
  } else {
    dropdown.options         = (const char**)malloc(sizeof(char*) * 8);
    dropdown.option_count    = 0;
    dropdown.option_capacity = 8;

    if (!dropdown.options) {
      DBG_E("Unable to allocate space for dropdown options\n");
      return (ui_dropdown){0};
    }
  }

  dropdown.selected = 0;
  dropdown.cursor   = 0;

  vec2_dup(dropdown.position, pos);
  vec2_dup(dropdown.size, size);

  // Attributes
  vec4 _select_color, _select_bg, _select_color_hover, _select_bg_hover;
  vec4 _color, _bg, _bg_hover, _color_hover;

  float _border_radius, _border_size;
  vec4  _border_color, _border_color_hover;

  int32_t _font;
  float   _font_size;

  ui_attrib_get4f(UI_DROPDOWN_SELECT_COLOR, _select_color);
  ui_attrib_get4f(UI_DROPDOWN_SELECT_BG, _select_bg);
  ui_attrib_get4f(UI_DROPDOWN_SELECT_COLOR_HOVER, _select_color_hover);
  ui_attrib_get4f(UI_DROPDOWN_SELECT_BG_HOVER, _select_bg_hover);

  ui_attrib_get4f(UI_DROPDOWN_COLOR, _color);
  ui_attrib_get4f(UI_DROPDOWN_BG, _bg);
  ui_attrib_get4f(UI_DROPDOWN_COLOR_HOVER, _color_hover);
  ui_attrib_get4f(UI_DROPDOWN_BG_HOVER, _bg_hover);

  ui_attrib_get4f(UI_DROPDOWN_BORDER_COLOR, _border_color);
  ui_attrib_get4f(UI_DROPDOWN_BORDER_COLOR_HOVER, _border_color_hover);

  _border_radius = ui_attrib_getf(UI_DROPDOWN_BORDER_RADIUS);
  _border_size   = ui_attrib_getf(UI_DROPDOWN_BORDER_SIZE);

  _font      = ui_attrib_getf(UI_DROPDOWN_FONT);
  _font_size = ui_attrib_getf(UI_DROPDOWN_FONT_SIZE);

  if (_font != -1) {
    dropdown.font = _font;
  } else {
    dropdown.font = 0;
  }

  if (_font_size != -1.f) {
    dropdown.font_size = _font_size;
  } else {
    // NOTE: Default font size, probably change later
    dropdown.font_size = 12.f;
  }

  if (_border_radius != -1.f) {
    dropdown.use_border    = 1;
    dropdown.border_radius = _border_radius;
  } else {
    dropdown.border_radius = 0.f;
  }

  if (_border_size != -1.f) {
    dropdown.use_border  = 1;
    dropdown.border_size = _border_size;
  } else {
    dropdown.border_size = 0.f;
  }

  if (_border_color[0] != -1.f) {
    vec4_dup(dropdown.border_color, _border_color);
    dropdown.use_border = 1;
  } else {
    vec4_clear(dropdown.border_color);
  }

  if (_border_color_hover[0] != -1.f) {
    vec4_dup(dropdown.hover_border_color, _border_color_hover);
    dropdown.use_border = 1;
  } else {
    vec4_clear(dropdown.hover_border_color);
  }

  if (_select_color[0] != -1.f) {
    vec4_dup(dropdown.select_color, _select_color);
  } else {
    vec4_clear(dropdown.select_color);
  }

  if (_select_bg[0] != -1.f) {
    vec4_dup(dropdown.select_bg, _select_bg);
  } else {
    vec4_clear(dropdown.select_bg);
  }

  if (_select_color_hover[0] != -1.f) {
    vec4_dup(dropdown.hover_select_color, _select_color_hover);
  } else {
    vec4_clear(dropdown.hover_select_color);
  }

  if (_select_bg_hover[0] != -1.f) {
    vec4_dup(dropdown.hover_select_bg, _select_bg_hover);
  } else {
    vec4_clear(dropdown.hover_select_bg);
  }

  if (_color[0] != -1.f) {
    vec4_dup(dropdown.color, _color);
  } else {
    vec4_clear(dropdown.color);
  }

  if (_bg[0] != -1.f) {
    vec4_dup(dropdown.bg, _bg);
  } else {
    vec4_clear(dropdown.bg);
  }

  if (_bg_hover[0] != -1.f) {
    vec4_dup(dropdown.hover_bg, _bg_hover);
  } else {
    vec4_clear(dropdown.hover_bg);
  }

  if (_color_hover[0] != -1.f) {
    vec4_dup(dropdown.hover_color, _color_hover);
  } else {
    vec4_clear(dropdown.hover_color);
  }

  dropdown.showing = 0;

  dropdown.bottom_scroll_pad = 1;
  dropdown.top_scroll_pad    = 1;

  return dropdown;
}

ui_option ui_option_create(const char* text, float font_size,
                           int32_t text_alignment, vec2 pos, vec2 size) {
  ui_option option;

  int32_t option_img_attrib = ui_attrib_geti(UI_OPTION_IMAGE);
  if (option_img_attrib > -1) {
    option.img_handle = option_img_attrib;
    option.use_img    = 1;
  } else {
    option.img_handle = -1;
    option.use_img    = 0;
  }

  vec2 option_img_size_attrib;
  ui_attrib_get2f(UI_OPTION_IMAGE_SIZE, option_img_size_attrib);
  if (option_img_size_attrib[0] != -1.f) {
    vec2_dup(option.img_size, option_img_size_attrib);
  } else {
    vec2_clear(option.img_size);
  }

  int32_t _font = ui_attrib_geti(UI_OPTION_FONT);
  if (_font != -1) {
    option.font = _font;
  } else {
    option.font = 0;
  }

  option.font_size = font_size;
  option.align     = text_alignment;

  if (text) {
    option.text     = (char*)text;
    option.use_text = 1;
  } else {
    option.use_text = 0;
  }

  vec2_clear(option.img_offset);

  vec2_dup(option.position, pos);
  vec2_dup(option.size, size);

  vec4_clear(option.bg);
  vec4_clear(option.hover_bg);
  vec4_clear(option.color);
  vec4_clear(option.hover_color);

  option.state     = 0;
  option.use_color = 0;

  return option;
}

ui_box ui_box_create(vec2 pos, vec2 size, vec4 color, vec4 hover_color) {
  ui_box box;
  vec2_dup(box.position, pos);
  vec2_dup(box.size, size);

  if (color) {
    vec4_dup(box.bg, color);
  } else {
    vec4_clear(box.bg);
  }

  if (hover_color) {
    vec4_dup(box.hover_bg, hover_color);
  } else {
    vec4_clear(box.hover_bg);
  }

  vec4_clear(box.border_color);
  vec4_clear(box.hover_border_color);

  box.border_size   = 0.f;
  box.border_radius = 0.f;
  box.use_border    = 0;

  return box;
}

ui_img ui_image_create(unsigned char* data, int data_length, ui_img_flags flags,
                       vec2 pos, vec2 size) {
  if (!data || !data_length) {
    DBG_E("No data passed.\n");
    return (ui_img){0};
  }

  int32_t image_handle =
      nvgCreateImageMem(g_ui_ctx.nvg, flags, data, data_length);
  ui_img img;
  img.handle = image_handle;
  vec2_dup(img.position, pos);
  vec2_dup(img.size, size);
  return img;
}

// TODO Refactor color names within structs to bg/fg
void ui_dropdown_set_colors(ui_dropdown* dropdown, vec4 bg, vec4 hover_bg,
                            vec4 fg, vec4 hover_fg, vec4 border_color,
                            vec4 hover_border_color, vec4 select_bg,
                            vec4 select_fg, vec4 hover_select_bg,
                            vec4 hover_select_fg) {
  if (bg) {
    vec4_dup(dropdown->bg, bg);
  }

  if (hover_bg) {
    vec4_dup(dropdown->hover_bg, hover_bg);
  }

  if (fg) {
    vec4_dup(dropdown->color, fg);
  }

  if (hover_fg) {
    vec4_dup(dropdown->hover_color, hover_fg);
  }

  if (border_color) {
    vec4_dup(dropdown->border_color, border_color);
  }

  if (hover_border_color) {
    vec4_dup(dropdown->hover_border_color, hover_border_color);
  }

  if (select_bg) {
    vec4_dup(dropdown->select_bg, select_bg);
  }

  if (hover_select_bg) {
    vec4_dup(dropdown->hover_select_bg, hover_select_bg);
  }

  if (select_fg) {
    vec4_dup(dropdown->select_color, select_fg);
  }

  if (hover_select_fg) {
    vec4_dup(dropdown->hover_select_color, hover_select_fg);
  }
}

void ui_box_set_colors(ui_box* box, vec4 bg, vec4 hover_bg, vec4 border_color,
                       vec4 hover_border_color) {
  if (bg) {
    vec4_dup(box->bg, bg);
  }

  if (hover_bg) {
    vec4_dup(box->hover_bg, hover_bg);
  }

  if (border_color) {
    vec4_dup(box->border_color, border_color);
  }

  if (hover_border_color) {
    vec4_dup(box->hover_border_color, hover_border_color);
  }
}

void ui_text_set_colors(ui_text* text, vec4 color, vec4 shadow) {
  if (color) {
    vec4_dup(text->color, color);
  }

  if (shadow) {
    vec4_dup(text->shadow, shadow);
  }
}

void ui_button_set_colors(ui_button* button, vec4 bg, vec4 hover_bg, vec4 fg,
                          vec4 hover_fg, vec4 border_color,
                          vec4 hover_border_color) {
  if (bg) {
    vec4_dup(button->bg, bg);
  }

  if (hover_bg) {
    vec4_dup(button->hover_bg, hover_bg);
  }

  if (fg) {
    vec4_dup(button->color, fg);
  }

  if (hover_fg) {
    vec4_dup(button->hover_color, hover_fg);
  }

  if (border_color) {
    vec4_dup(button->border_color, border_color);
  }

  if (hover_border_color) {
    vec4_dup(button->hover_border_color, hover_border_color);
  }
}

void ui_line_set_colors(ui_line* line, vec4 color) {
  if (color) {
    vec4_dup(line->color, color);
  }
}

void ui_option_set_colors(ui_option* option, vec4 bg, vec4 hover_bg, vec4 fg,
                          vec4 hover_fg) {
  if (bg) {
    vec4_dup(option->bg, bg);
  }

  if (hover_bg) {
    vec4_dup(option->hover_bg, hover_bg);
  }

  if (fg) {
    vec4_dup(option->color, fg);
  }

  if (hover_fg) {
    vec4_dup(option->hover_color, hover_fg);
  }
}

void ui_img_set_colors(ui_img* img, vec4 border_color,
                       vec4 hover_border_color) {
  if (border_color) {
    vec4_dup(img->border_color, border_color);
  }

  if (hover_border_color) {
    vec4_dup(img->hover_border_color, hover_border_color);
  }
}

void ui_img_set_border_radius(ui_img* img, float radius) {
  img->border_radius = radius;
}

void ui_button_set_border_radius(ui_button* button, float radius) {
  button->border_radius = radius;
  button->use_border    = 1;
}

void ui_box_set_border_radius(ui_box* box, float radius) {
  box->border_radius = radius;
  box->use_border    = 1;
}

void ui_dropdown_set_border_radius(ui_dropdown* dropdown, float radius) {
  dropdown->border_radius = radius;
  dropdown->use_border    = 1;
}

uint16_t ui_dropdown_add_option(ui_dropdown* dropdown, const char* option) {
  if (dropdown->option_count == dropdown->option_capacity) {
    dropdown->options =
        (const char**)realloc((void*)dropdown->options,
                              sizeof(char*) * dropdown->option_capacity + 4);
    dropdown->option_capacity += 4;
    if (!dropdown->options) {
      DBG_E("Unable to expand memory for dropdown options\n");
      return 0;
    }
  }

  dropdown->options[dropdown->option_count] = option;
  ++dropdown->option_count;

  return dropdown->option_count - 1;
}

void ui_text_next(ui_text* text) {
  assert(text);
  if (text->use_reveal) {
    if (text->reveal < text->text + text->text_length) {
      ++text->reveal;
    }
  }
}

void ui_text_prev(ui_text* text) {
  assert(text);
  if (text->use_reveal) {
    if (text->reveal > text->text) {
      --text->reveal;
    }
  }
}

// TODO Mouse Scrolling w/ Click / Hover
void ui_dropdown_next(ui_dropdown* dropdown) {
  if (dropdown->cursor < dropdown->option_count - 1) {
    dropdown->cursor += 1;
  }

  int cursor_rel =
      dropdown->option_display - (dropdown->cursor - dropdown->start);
  if (cursor_rel < dropdown->bottom_scroll_pad) {
    if (dropdown->start < dropdown->option_count - dropdown->option_display) {
      dropdown->start += 1;
    } else {
      dropdown->start = dropdown->option_count - dropdown->option_display;
    }
  }
}

void ui_dropdown_prev(ui_dropdown* dropdown) {
  if (dropdown->cursor > 0) {
    dropdown->cursor--;
  }

  int cursor_rel = dropdown->cursor - dropdown->start;

  // printf("rel: %i\n", cursor_rel);
  if (cursor_rel < dropdown->top_scroll_pad) {
    if (dropdown->start > 0) {
      dropdown->start--;
      printf("Dropdown down.\n");
    } else {
      dropdown->start = 0;
    }
  }
}

void ui_dropdown_set_to_cursor(ui_dropdown* dropdown) {
  if (dropdown->selected != dropdown->cursor) {
    dropdown->has_change = 1;
  }

  dropdown->selected = dropdown->cursor;

  dropdown->start = dropdown->selected - dropdown->option_display / 2;

  if (dropdown->start < 0) {
    dropdown->start = 0;
  } else if (dropdown->start >
             dropdown->option_count - dropdown->option_display) {
    dropdown->start = dropdown->option_count - dropdown->option_display;
  }
}

void ui_dropdown_set(ui_dropdown* dropdown, uint16_t select) {
  if (select < dropdown->option_count - 1 && select >= 0) {
    if (select != dropdown->selected) {
      dropdown->has_change = 1;
    }
    dropdown->selected = select;

    dropdown->start = dropdown->selected - dropdown->option_display / 2;

    if (dropdown->start < 0) {
      dropdown->start = 0;
    } else if (dropdown->start >
               dropdown->option_count - dropdown->option_display) {
      dropdown->start = dropdown->option_count - dropdown->option_display;
    }
  }
}

int8_t ui_dropdown_has_change(ui_dropdown* dropdown) {
  int8_t dropdown_change = dropdown->has_change;
  dropdown->has_change   = 0;
  return dropdown_change;
}

ui_font ui_font_get(const char* font_name) {
  // Find the font via NanoVG
  return nvgFindFont(g_ui_ctx.nvg, font_name);
}

ui_font ui_font_create(unsigned char* data, int data_length, const char* name) {
  int font_id = nvgCreateFontMem(g_ui_ctx.nvg, name, data, data_length, 0);
  return font_id;
}

void ui_tree_remove_id(ui_tree* tree, uint32_t id) {
  ui_leaf* current = tree->start;
  while (current) {
    if (current->uid == id) {
      ui_leaf* prev = current->prev;
      ui_leaf* next = current->next;

      if (current == tree->end) {
        tree->end = prev;
      } else if (current == tree->start) {
        tree->start = next;
      }

      prev->next = next;
      next->prev = prev;
      break;
    }
    current = current->next;
  }
}

void ui_image_destroy(ui_img* img) {
  nvgDeleteImage(g_ui_ctx.nvg, img->handle);
}

void ui_text_bounds(ui_text* text, vec4 bounds) {
  if (text->use_box) {
    nvgTextBoxBounds(g_ui_ctx.nvg, text->position[0], text->position[1],
                     text->bounds[0], text->text, 0, bounds);
  } else {
    nvgTextBounds(g_ui_ctx.nvg, text->position[0], text->position[1],
                  text->text, 0, bounds);
  }
}

uint32_t ui_tree_select(ui_tree* tree, int32_t event_type, int is_mouse) {
  if (g_ui_ctx.use_mouse && is_mouse) {
    if (tree->mouse_hover) {
      tree->mouse_hover->event = event_type;

      if (tree->mouse_hover->element.type == UI_DROPDOWN) {
        ui_dropdown* dropdown = (ui_dropdown*)tree->mouse_hover->element.data;
        int16_t      selection =
            ui_element_contains(tree->mouse_hover->element, g_ui_ctx.mouse_pos);

        if (selection > 0 && dropdown->showing) {
          ui_dropdown_set(dropdown, selection);
        } else if (selection > 0 && !dropdown->showing) {
          dropdown->showing = 1;
        }
      }

      return tree->mouse_hover->uid;
    }
  }

  if (tree->cursor && !is_mouse) {
    tree->cursor->event = event_type;
    return tree->cursor->uid;
  }

  return 0;
}

int8_t ui_tree_is_active(ui_tree* tree, uint32_t id) {
  if (g_ui_ctx.use_mouse) {
    if (tree->mouse_hover->uid == id) {
      return 1;
    }
  }

  if (tree->cursor->uid == id) {
    return 1;
  }

  return 0;
}

uint32_t ui_tree_next(ui_tree* tree) {
  if (!tree || !tree->start) {
    // NOTE: all UI Element IDs are non-zero
    return 0;
  }

  if (!tree->cursor) {
    tree->cursor = tree->start;
  }

  if (tree->cursor) {
    if (tree->cursor->element.type == UI_DROPDOWN) {
      ui_dropdown* dropdown = (ui_dropdown*)tree->cursor->element.data;
      if (dropdown->showing) {
        ui_dropdown_next(dropdown);
        return dropdown->id;
      }
    }
  }

  ui_leaf* cursor = tree->cursor;
  ui_leaf* last   = 0;
  while (cursor) {
    if (cursor->next) {
      if (cursor->next->selectable) {
        tree->cursor = cursor->next;
        return cursor->next->uid;
      } else {
        cursor = cursor->next;
      }
    } else {
      if (tree->loop) {
        if (!last) {
          last   = cursor;
          cursor = tree->start;
        } else {
          return 0;
        }
      } else {
        return 0;
      }
    }
  }

  return 0;
}

uint32_t ui_tree_prev(ui_tree* tree) {
  if (!tree || !tree->start) {
    // NOTE: all UI Element IDs are non-zero
    return 0;
  }

  if (!tree->cursor) {
    tree->cursor = tree->end;
  }

  if (tree->cursor) {
    if (tree->cursor->element.type == UI_DROPDOWN) {
      ui_dropdown* dropdown = (ui_dropdown*)tree->cursor->element.data;
      if (dropdown->showing) {
        ui_dropdown_prev(dropdown);
        return dropdown->id;
      }
    }
  }

  ui_leaf* cursor = tree->cursor;
  ui_leaf* last   = 0;
  while (cursor) {
    if (cursor->prev) {
      if (cursor->prev->selectable) {
        tree->cursor = cursor->prev;
        return cursor->prev->uid;
      } else {
        cursor = cursor->prev;
      }
    } else {
      if (tree->loop) {
        if (!last) {
          last   = cursor;
          cursor = tree->end;
        } else {
          return 0;
        }
      } else {
        return 0;
      }
    }
  }

  return 0;
}


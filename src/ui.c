#include <astera/ui.h>

#include <astera/debug.h>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <nanovg/nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg/nanovg_gl.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct ui_ctx {
  vec2  size;
  float pixel_scale;

  NVGcontext* nvg;
  uint32_t    global_id; /* element assignment id */

  ui_attrib_map attribs;

  /* mouse info */
  vec2    mouse_pos;
  uint8_t use_mouse, antialias;
};

static NVGcolor ui_u_color(ui_color v) {
  return nvgRGBA((unsigned char)(v[0] * 255), (unsigned char)(v[1] * 255),
                 (unsigned char)(v[2] * 255), (unsigned char)(v[3] * 255));
}

uint32_t ui_element_get_uid(ui_element element) {
  if (element.data) {
    return *((uint32_t*)element.data);
  } else {
    return 0;
  }
}

void ui_color_dup(ui_color dst, ui_color const a) {
  for (int i = 0; i < 4; ++i)
    dst[i] = a[i];
}

void ui_color_clear(ui_color dst) {
  for (int i = 0; i < 4; ++i)
    dst[i] = -1.f;
}

uint8_t ui_color_valid(ui_color const a) {
  for (int i = 0; i < 4; ++i)
    if (a[i] >= 0.f)
      return 1;
  return 0;
}

ui_ctx* ui_ctx_create(vec2 screen_size, float pixel_scale, uint8_t use_mouse,
                      uint8_t antialias, uint8_t attribs) {
  ui_ctx* ctx = (ui_ctx*)malloc(sizeof(ui_ctx));

  memset(ctx, 0, sizeof(ui_ctx));

  vec2_dup(ctx->size, screen_size);

  ctx->antialias = antialias;

  if (antialias) {
    ctx->nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
  } else {
    ctx->nvg = nvgCreateGL3(NVG_STENCIL_STROKES);
  }

  ctx->pixel_scale = pixel_scale;
  ctx->use_mouse   = use_mouse;

  if (attribs) {
    ctx->attribs.capacity = UI_ATTRIB_LAST;
    ctx->attribs.attribs =
        malloc(sizeof(ui_attrib_storage) * ctx->attribs.capacity);
    memset(ctx->attribs.attribs, 0,
           sizeof(ui_attrib_storage) * ctx->attribs.capacity);
  } else {
    ctx->attribs.capacity = 0;
  }
  ctx->attribs.count = 0;

  return ctx;
}

uint8_t ui_ctx_is_mouse(ui_ctx* ctx) { return ctx->use_mouse; }

uint8_t ui_ctx_is_antialias(ui_ctx* ctx) { return ctx->antialias; }

void ui_ctx_set_mouse(ui_ctx* ctx, uint8_t mouse) { ctx->use_mouse = mouse; }

void ui_ctx_update(ui_ctx* ctx, vec2 mouse_pos) {
  if (ctx->use_mouse) {
    vec2_dup(ctx->mouse_pos, mouse_pos);
  }
}

void ui_ctx_resize(ui_ctx* ctx, vec2 screen_size) {
  vec2_dup(ctx->size, screen_size);
}

void ui_ctx_destroy(ui_ctx* ctx) {
  nvgDeleteGL3(ctx->nvg);

  if (ctx->attribs.attribs)
    free(ctx->attribs.attribs);
  free(ctx);
}

float ui_px_scale_width(ui_ctx* ctx, float px_width) {
  return px_width / ctx->size[0];
}

float ui_px_scale_height(ui_ctx* ctx, float px_height) {
  return px_height / ctx->size[1];
}

float ui_square_width(ui_ctx* ctx, float width) {
  float px_value = width * ctx->size[0];
  return px_value / ctx->size[1];
}

float ui_square_height(ui_ctx* ctx, float height) {
  float px_value = height * ctx->size[1];
  return px_value / ctx->size[0];
}

void ui_ctx_set_attribs_fixed(ui_ctx* ctx) {
  ui_attrib_map* map = &ctx->attribs;
  map->allow_resize  = 0;
}

void ui_ctx_set_attribs_capacity(ui_ctx* ctx, uint32_t capacity) {
  if (capacity > UI_ATTRIB_LAST) {
    capacity = UI_ATTRIB_LAST;
  }

  ui_attrib_map* map = &ctx->attribs;

  if (!map->allow_resize)
    return;

  if (map->capacity == capacity) {
    return;
  }

  if (map->count > capacity) {
    map->count = capacity;
  }

  map->attribs = (ui_attrib_storage*)realloc(
      map->attribs, sizeof(ui_attrib_storage) * capacity);
  map->capacity = capacity;
}

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

void ui_get_color(ui_color val, const char* v) {
  int len    = (int)strlen(v);
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

void ui_scale_offset_px(ui_ctx* ctx, vec2 dst, vec2 val, vec2 px) {
  vec2 tmp;
  tmp[0] = px[0] / ctx->size[0];
  tmp[1] = px[1] / ctx->size[1];
  vec2_add(dst, val, tmp);
}

void ui_px_to_scale(ui_ctx* ctx, vec2 dst, vec2 px) {
  vec2 tmp;
  tmp[0] = px[0] / ctx->size[0];
  tmp[1] = px[1] / ctx->size[1];
  vec2_dup(dst, tmp);
}

void ui_scale_to_px(ui_ctx* ctx, vec2 dst, vec2 scale) {
  vec2 tmp;
  tmp[0] = scale[0] * ctx->size[0];
  tmp[1] = scale[1] * ctx->size[1];
  vec2_dup(dst, tmp);
}

void ui_scale_to_px4f(ui_ctx* ctx, vec4 dst, vec4 scale) {
  dst[0] = scale[0] * ctx->size[0];
  dst[1] = scale[1] * ctx->size[1];
  dst[2] = scale[2] * ctx->size[0];
  dst[3] = scale[3] * ctx->size[1];
}

void ui_px_from_scale(vec2 dst, vec2 px, vec2 screen) {
  vec2_div(dst, px, screen);
}

void ui_scale_move_px(ui_ctx* ctx, vec2 dst, vec2 scale, vec2 px) {
  vec2 tmp;
  ui_px_to_scale(ctx, tmp, px);
  vec2_add(dst, tmp, scale);
}

void ui_px_move_scale(ui_ctx* ctx, vec2 dst, vec2 px, vec2 scale) {
  vec2 tmp;
  ui_scale_to_px(ctx, tmp, scale);
  vec2_add(dst, tmp, px);
}

void ui_ctx_scale_set(ui_ctx* ctx, vec2 size_px) {
  vec2_dup(ctx->size, size_px);
}

void ui_ctx_scale_get(ui_ctx* ctx, vec2 dst_px) { vec2_dup(dst_px, ctx->size); }

void ui_frame_start(ui_ctx* ctx) {
  nvgBeginFrame(ctx->nvg, ctx->size[0], ctx->size[1], ctx->pixel_scale);
}

static inline int8_t ui_is_type(int value, int type) {
  return ((value) & (type)) == type;
}

void ui_frame_end(ui_ctx* ctx) { nvgEndFrame(ctx->nvg); }

static uint16_t ui_attrib_size(ui_attrib_type type) {
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
    case UI_COLOR:
      return sizeof(ui_color);
    default:
      return sizeof(char);
  }
}

static ui_attrib_storage* _ui_attrib_get_add(ui_ctx* ctx, ui_attrib attrib,
                                             ui_attrib_type type) {
  ui_attrib_map* map = &ctx->attribs;
  for (uint32_t i = 0; i < ctx->attribs.count; ++i) {
    if (map->attribs[i].attrib == attrib) {
      return &map->attribs[i];
    }
  }

  ui_attrib_storage* storage = 0;

  if (map->count == map->capacity) {
    if (map->allow_resize) {
      ui_ctx_set_attribs_capacity(ctx, map->capacity + 4);

      storage = &map->attribs[map->count];

      storage->attrib = attrib;
      storage->type   = type;

      ++map->count;

      return storage;
    } else {
      return 0;
    }
  } else {
    storage         = &map->attribs[map->count];
    storage->attrib = attrib;
    storage->type   = type;
    ++map->count;

    return storage;
  }
}

void ui_attrib_set4f(ui_ctx* ctx, ui_attrib attrib, vec4 value) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_VEC4);
  vec4_dup(storage->data.v4, value);
}

void ui_attrib_setc(ui_ctx* ctx, ui_attrib attrib, ui_color color) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_COLOR);
  ui_color_dup(storage->data.c, color);
}

void ui_attrib_set3f(ui_ctx* ctx, ui_attrib attrib, vec3 value) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_VEC3);
  vec3_dup(storage->data.v3, value);
}

void ui_attrib_set2f(ui_ctx* ctx, ui_attrib attrib, vec2 value) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_VEC2);
  vec2_dup(storage->data.v2, value);
}

void ui_attrib_setf(ui_ctx* ctx, ui_attrib attrib, float value) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_FLOAT);
  storage->data.f            = value;
}

void ui_attrib_seti(ui_ctx* ctx, ui_attrib attrib, int32_t value) {
  ui_attrib_storage* storage = _ui_attrib_get_add(ctx, attrib, UI_FLOAT);
  storage->data.i            = value;
}

uint8_t ui_attrib_exists(ui_ctx* ctx, ui_attrib attrib) {
  for (uint32_t i = 0; i < ctx->attribs.count; ++i) {
    if (ctx->attribs.attribs[i].attrib == attrib) {
      return 1;
    }
  }
  return 0;
}

static ui_attrib_storage* ui_attrib_get(ui_ctx* ctx, ui_attrib attrib) {
  ui_attrib_map map = ctx->attribs;
  for (uint32_t i = 0; i < map.capacity; ++i) {
    if (map.attribs[i].attrib == attrib) {
      return &map.attribs[i];
    }
  }
  return 0;
}

int ui_attrib_geti(ui_ctx* ctx, ui_attrib attrib) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    return data->data.i;
  }
  return -1;
}

float ui_attrib_getf(ui_ctx* ctx, ui_attrib attrib) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    return data->data.f;
  }
  return 0.f;
}

void ui_attrib_get2f(ui_ctx* ctx, ui_attrib attrib, vec2 dst) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    vec2_dup(dst, data->data.v2);
    return;
  }

  dst[0] = -1.f;
}

void ui_attrib_get3f(ui_ctx* ctx, ui_attrib attrib, vec3 dst) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    vec3_dup(dst, data->data.v3);
    return;
  }
  dst[0] = -1.f;
}

void ui_attrib_get4f(ui_ctx* ctx, ui_attrib attrib, vec4 dst) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    vec4_dup(dst, data->data.v4);
    return;
  }

  dst[0] = -1.f;
}

void ui_attrib_getc(ui_ctx* ctx, ui_attrib attrib, ui_color dst) {
  ui_attrib_storage* data = ui_attrib_get(ctx, attrib);
  if (data) {
    ui_color_dup(dst, data->data.c);
    return;
  }

  dst[0] = -1.f;
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
          ui_is_type(text->align, UI_ALIGN_MIDDLE_X)) {
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
    case UI_PROGRESS: {
      ui_progress* progress = (ui_progress*)element.data;
      center[0] -= progress->size[0] * 0.5f;
      center[1] -= progress->size[1] * 0.5f;
      vec2_dup(progress->position, center);
    } break;
    case UI_SLIDER: {
      ui_slider* slider = (ui_slider*)element.data;
      center[0] -= slider->size[0] * 0.5f;
      center[1] -= slider->size[1] * 0.5f;
      vec2_dup(slider->position, center);
    } break;
    case UI_LINE: {
      ui_line* line = (ui_line*)element.data;

      vec2 delta = {0.f};
      delta[0]   = line->end[0] - line->start[0];
      delta[1]   = line->end[1] - line->start[1];

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
    case UI_IMG: {
      ui_img* image = (ui_img*)element.data;
      center[0] -= image->size[0] * 0.5f;
      center[1] -= image->size[1] * 0.5f;
      vec2_dup(image->position, center);
    } break;
  }
}

ui_tree ui_tree_create(uint16_t capacity) {
  ui_tree  tree;
  ui_leaf* raw = malloc(sizeof(ui_leaf) * (capacity + 1));
  memset(raw, 0, sizeof(ui_leaf) * (capacity + 1));
  tree.raw = raw;

  tree.draw_order = (ui_leaf**)malloc(sizeof(ui_leaf*) * (capacity + 1));
  memset(raw, 0, sizeof(ui_leaf*) * (capacity + 1));

  assert(raw != 0);

  tree.cursor_id      = 0;
  tree.mouse_hover_id = 0;

  tree.cursor_index      = 0;
  tree.mouse_hover_index = 0;

  tree.count    = 0;
  tree.capacity = capacity;
  tree.loop     = 0;
  return tree;
}

int16_t ui_element_contains(ui_ctx* ctx, ui_element element, vec2 point) {
  switch (element.type) {
    case UI_BOX: {
      ui_box* box = (ui_box*)element.data;
      vec2    box_position, box_size;

      ui_scale_to_px(ctx, box_position, box->position);
      ui_scale_to_px(ctx, box_size, box->size);

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
      ui_scale_to_px(ctx, dropdown_position, dropdown->position);
      ui_scale_to_px(ctx, dropdown_size, dropdown->size);

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
      ui_scale_to_px(ctx, option_position, option->position);
      ui_scale_to_px(ctx, option_size, option->size);

      if (option->use_img) {
        vec2 opt_img_size, opt_img_offset;
        ui_scale_to_px(ctx, opt_img_size, option->img_size);
        ui_scale_to_px(ctx, opt_img_offset, option->img_offset);

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
        nvgFontFaceId(ctx->nvg, option->font);
        nvgFontSize(ctx->nvg, option->font_size);

        if (ui_is_type(option->align, UI_ALIGN_CENTER)) {
          nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
        } else {
          nvgTextAlign(ctx->nvg, option->align);
        }

        vec4 opt_text_bounds;

        nvgTextBounds(ctx->nvg, option_position[0], option_position[1],
                      option->text, 0, &opt_text_bounds[0]);

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
      ui_scale_to_px(ctx, button_position, button->position);
      ui_scale_to_px(ctx, button_size, button->size);

      if (point[0] > button_position[0] &&
          point[0] < button_position[0] + button_size[0]) {
        if (point[1] > button_position[1] &&
            point[1] < button_position[1] + button_size[1]) {
          return 1;
        }
      }
    }
      return -1;
    case UI_SLIDER: {
      ui_slider* slider = (ui_slider*)element.data;
      vec2       slider_pos, slider_size;
      vec2       button_pos, button_size;

      ui_scale_to_px(ctx, slider_pos, slider->position);
      ui_scale_to_px(ctx, slider_size, slider->size);

      vec2_dup(button_pos, slider_pos);
      ui_scale_to_px(ctx, button_size, slider->button_size);
      button_pos[0] += slider->progress * slider_size[0];

      if (slider->holding) {
        if (slider->vertical_fill) {
          if (slider->flip) {
            slider->holding_progress =
                ((slider_pos[1] + slider_size[1]) - point[1]) / slider_size[1];
          } else {
            slider->holding_progress =
                (point[1] - slider_pos[1]) / slider_size[1];
          }
        } else {
          if (slider->flip) {
            slider->holding_progress =
                ((slider_pos[0] + slider_size[0]) - point[0]) / slider_size[0];
          } else {
            slider->holding_progress =
                (point[0] - slider_pos[0]) / slider_size[0];
          }
        }

        if (slider->holding_progress > 1.f)
          slider->holding_progress = 1.f;
        if (slider->holding_progress < 0.f)
          slider->holding_progress = 0.f;
        return 1;
      }

      /* button hovering */
      if (point[0] > button_pos[0] &&
          point[0] < button_pos[0] + button_size[0]) {
        if (point[1] > button_pos[1] &&
            point[1] < button_pos[1] + button_size[1]) {
          if (slider->vertical_fill) {
            if (slider->flip) {
              slider->holding_progress =
                  ((slider_pos[1] + slider_size[1]) - point[1]) /
                  slider_size[1];
            } else {
              slider->holding_progress =
                  (point[1] - slider_pos[1]) / slider_size[1];
            }
          } else {
            if (slider->flip) {
              slider->holding_progress =
                  ((slider_pos[0] + slider_size[0]) - point[0]) /
                  slider_size[0];
            } else {
              slider->holding_progress =
                  (point[0] - slider_pos[0]) / slider_size[0];
            }
          }

          if (slider->holding_progress > 1.f)
            slider->holding_progress = 1.f;
          if (slider->holding_progress < 0.f)
            slider->holding_progress = 0.f;

          slider->button_hover = 1;
          return 1;
        }
      }

      /* bar hovering */
      if (point[0] > slider_pos[0] &&
          point[0] < slider_pos[0] + slider_size[0]) {
        if (point[1] > slider_pos[1] &&
            point[1] < slider_pos[1] + slider_size[1]) {
          if (slider->vertical_fill) {
            if (slider->flip) {
              slider->holding_progress =
                  ((slider_pos[1] + slider_size[1]) - point[1]) /
                  slider_size[1];
            } else {
              slider->holding_progress =
                  (point[1] - slider_pos[1]) / slider_size[1];
            }
          } else {
            if (slider->flip) {
              slider->holding_progress =
                  ((slider_pos[0] + slider_size[0]) - point[0]) /
                  slider_size[0];
            } else {
              slider->holding_progress =
                  (point[0] - slider_pos[0]) / slider_size[0];
            }
          }

          if (slider->holding_progress > 1.f)
            slider->holding_progress = 1.f;
          if (slider->holding_progress < 0.f)
            slider->holding_progress = 0.f;

          return 1;
        }
      }

      slider->button_hover = 0;
    }
      return -1;
    case UI_TEXT: {
      ui_text* text = (ui_text*)element.data;
      nvgFontFaceId(ctx->nvg, text->font);
      nvgFontSize(ctx->nvg, text->size);

      if (ui_is_type(text->align, UI_ALIGN_CENTER)) {
        nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
      } else {
        nvgTextAlign(ctx->nvg, text->align);
      }

      vec4 text_bounds;
      vec2 text_position, text_internal_bounds;
      ui_scale_to_px(ctx, text_position, text->position);
      ui_scale_to_px(ctx, text_internal_bounds, text->bounds);

      if (text->use_box) {
        nvgTextBoxBounds(ctx->nvg, text_position[0], text_position[1],
                         text_internal_bounds[0], text->text,
                         (text->use_reveal) ? text->reveal : 0, text_bounds);
      } else {
        nvgTextBounds(ctx->nvg, text_position[0], text_position[1], text->text,
                      (text->use_reveal) ? text->reveal : 0, text_bounds);
      }

      if (point[0] > text_bounds[0] && point[0] < text_bounds[2]) {
        if (point[1] > text_bounds[1] && point[1] < text_bounds[3]) {
          return 1;
        }
      }
    }
      return -1;
    case UI_IMG: {
      ui_img* img = (ui_img*)element.data;
      vec2    img_position, img_size;
      ui_scale_to_px(ctx, img_position, img->position);
      ui_scale_to_px(ctx, img_size, img->size);

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

/* Run a check over mouse / cursor usage (mouse only) */
uint32_t ui_tree_check(ui_ctx* ctx, ui_tree* tree) {
  if (!ctx->use_mouse) {
    return 0;
  }

  vec2 mouse_pos;
  vec2_dup(mouse_pos, ctx->mouse_pos);

  int hover_priority = -1;

  int potential_index = -1;
  int potential_uid   = -1;

  for (uint32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor = &tree->raw[i];

    if (cursor->selectable) {
      int16_t hovered =
          ui_element_contains(ctx, cursor->element, ctx->mouse_pos);
      int8_t current_priority = cursor->priority;

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
        potential_index = i;
        potential_uid   = cursor->uid;
        hover_priority  = current_priority;
      }
    }
  }

  if (potential_index != -1) {
    if (tree->selected_index) {
      if (tree->selected_index == (uint32_t)potential_index) {
        tree->mouse_hover_id    = potential_uid;
        tree->mouse_hover_index = potential_index;
      } else {
        tree->mouse_hover_id    = 0;
        tree->mouse_hover_index = 0;
      }
    } else {
      tree->mouse_hover_id    = potential_uid;
      tree->mouse_hover_index = potential_index;
    }
    return tree->mouse_hover_id;
  }

  tree->mouse_hover_id    = 0;
  tree->mouse_hover_index = 0;
  return 0;
}

void ui_text_draw(ui_ctx* ctx, ui_text* text) {
  nvgBeginPath(ctx->nvg);
  nvgFontSize(ctx->nvg, text->size);
  nvgFontFaceId(ctx->nvg, text->font);

  if (ui_is_type(text->align, UI_ALIGN_CENTER)) {
    nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
  } else {
    nvgTextAlign(ctx->nvg, text->align);
  }

  if (text->use_spacing) {
    nvgTextLetterSpacing(ctx->nvg, text->spacing);
    nvgTextLineHeight(ctx->nvg, text->line_height);
  }

  vec2 draw_pos;
  ui_scale_to_px(ctx, draw_pos, text->position);

  if (text->use_box) {
    vec2 text_bounds;
    ui_scale_to_px(ctx, text_bounds, text->bounds);

    if (text->use_shadow) {
      nvgFontBlur(ctx->nvg, text->shadow_size);
      nvgFillColor(ctx->nvg, ui_u_color(text->shadow));
      nvgTextBox(ctx->nvg, draw_pos[0] - (text_bounds[0] * 0.5f), draw_pos[1],
                 text_bounds[0], text->text, text->reveal);

      nvgFontBlur(ctx->nvg, 0.f);
    }

    nvgFillColor(ctx->nvg, ui_u_color(text->color));
    nvgTextBox(ctx->nvg, draw_pos[0] - (text_bounds[0] * 0.5f), draw_pos[1],
               text_bounds[0], text->text, text->reveal);
  } else {
    if (text->use_shadow) {
      nvgFontBlur(ctx->nvg, text->shadow_size);
      nvgFillColor(ctx->nvg, ui_u_color(text->shadow));
      nvgText(ctx->nvg, draw_pos[0], draw_pos[1], text->text, text->reveal);

      nvgFontBlur(ctx->nvg, 0.f);
    }

    nvgFillColor(ctx->nvg, ui_u_color(text->color));
    nvgText(ctx->nvg, draw_pos[0], draw_pos[1], text->text, text->reveal);
  }
}

void ui_box_draw(ui_ctx* ctx, ui_box* box, int8_t focused) {
  vec2 box_position, box_size;
  ui_scale_to_px(ctx, box_position, box->position);
  ui_scale_to_px(ctx, box_size, box->size);

  nvgBeginPath(ctx->nvg);

  if (box->border_size > 0.f) {
    if (box->border_radius != 0.f) {
      nvgRoundedRect(ctx->nvg, box_position[0], box_position[1], box_size[0],
                     box_size[1], box->border_radius);
    } else {
      nvgRect(ctx->nvg, box_position[0], box_position[1], box_size[0],
              box_size[1]);
    }

    nvgStrokeWidth(ctx->nvg, box->border_size);
    if (focused) {
      nvgStrokeColor(ctx->nvg, ui_u_color(box->hover_border_color));
      nvgFillColor(ctx->nvg, ui_u_color(box->hover_bg));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(box->border_color));
      nvgFillColor(ctx->nvg, ui_u_color(box->bg));
    }
    nvgStroke(ctx->nvg);
  } else {
    if (focused) {
      nvgFillColor(ctx->nvg, ui_u_color(box->hover_bg));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(box->bg));
    }
  }

  if (box->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, box_position[0], box_position[1], box_size[0],
                   box_size[1], box->border_radius);
  } else {
    nvgRect(ctx->nvg, box_position[0], box_position[1], box_size[0],
            box_size[1]);
  }

  nvgFill(ctx->nvg);
}

void ui_button_draw(ui_ctx* ctx, ui_button* button, int8_t focused) {
  nvgBeginPath(ctx->nvg);

  vec2 button_position, button_size;
  vec4 button_padding, button_padded_size;

  ui_scale_to_px(ctx, button_position, button->position);
  ui_scale_to_px(ctx, button_size, button->size);
  ui_scale_to_px4f(ctx, button_padding, button->padding);

  button_padded_size[0] =
      button_padding[0] + button_padding[2] + button_size[0];

  button_padded_size[1] =
      button_padding[1] + button_padding[3] + button_size[1];

  if (button->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, button_position[0] - button_padding[0],
                   button_position[1] - button_padding[1],
                   button_padded_size[0], button_padded_size[1],
                   button->border_radius);
  } else {
    nvgRect(ctx->nvg, button_position[0] - button_padding[0],
            button_position[1] - button_padding[1], button_padded_size[0],
            button_padded_size[1]);
  }

  if (button->border_size > 0.f) {
    nvgStrokeWidth(ctx->nvg, button->border_size);
    if (focused) {
      nvgStrokeColor(ctx->nvg, ui_u_color(button->hover_border_color));
      nvgFillColor(ctx->nvg, ui_u_color(button->hover_bg));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(button->border_color));
      nvgFillColor(ctx->nvg, ui_u_color(button->bg));
    }
    nvgStroke(ctx->nvg);
  }

  if (focused) {
    nvgFillColor(ctx->nvg, ui_u_color(button->hover_bg));
  } else {
    nvgFillColor(ctx->nvg, ui_u_color(button->bg));
  }

  nvgFill(ctx->nvg);

  if (button->text) {
    if (focused) {
      nvgFillColor(ctx->nvg, ui_u_color(button->hover_color));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(button->color));
    }

    nvgFontSize(ctx->nvg, button->font_size);
    nvgFontFaceId(ctx->nvg, button->font);

    if (ui_is_type(button->align, UI_ALIGN_CENTER)) {
      nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
    } else {
      nvgTextAlign(ctx->nvg, button->align);
    }

    nvgTextLineHeight(ctx->nvg, button->font_size);

    vec2 offset = {0.f, 0.f};

    vec4 text_bounds = {0.f};
    nvgTextBounds(ctx->nvg, 0.f, 0.f, button->text, 0, text_bounds);
    vec2 text_size = {text_bounds[2] - text_bounds[0],
                      text_bounds[3] - text_bounds[1]};

    if (ui_is_type(button->align, UI_ALIGN_LEFT)) {
      offset[0] = 0.f;
    } else if (ui_is_type(button->align, UI_ALIGN_MIDDLE_X) ||
               ui_is_type(button->align, UI_ALIGN_CENTER)) {
      offset[0] = (button_size[0] / 2.f);
    } else if (ui_is_type(button->align, UI_ALIGN_RIGHT)) {
      offset[0] = button_size[0];
    }

    if (ui_is_type(button->align, UI_ALIGN_TOP)) {
      offset[1] = 0;
    } else if (ui_is_type(button->align, UI_ALIGN_MIDDLE_Y) ||
               ui_is_type(button->align, UI_ALIGN_CENTER)) {
      offset[1] = (button_size[1] * 0.5f);
    } else if (ui_is_type(button->align, UI_ALIGN_BOTTOM)) {
      offset[1] = button_size[1];
    } else {
      offset[1] = text_size[1];
    }

    nvgText(ctx->nvg, button_position[0] + offset[0],
            button_position[1] + offset[1], button->text, 0);
  }
}

void ui_progress_draw(ui_ctx* ctx, ui_progress* progress, int8_t focused) {
  vec2 progress_pos, progress_size, fill_size;

  ui_scale_to_px(ctx, progress_pos, progress->position);
  ui_scale_to_px(ctx, progress_size, progress->size);

  vec2_dup(fill_size, progress_size);
  if (progress->vertical_fill) {
    fill_size[0] = progress_size[0];
    fill_size[1] = progress_size[1] * progress->progress;
  } else {
    fill_size[0] = progress_size[0] * progress->progress;
    fill_size[1] = progress_size[1];
  }

  nvgBeginPath(ctx->nvg);

  /* border */
  if (progress->border_size != 0.f) {
    if (progress->border_radius != 0.f) {
      nvgRoundedRect(ctx->nvg, progress_pos[0], progress_pos[1],
                     progress_size[0], progress_size[1],
                     progress->border_radius);

    } else {
      nvgRect(ctx->nvg, progress_pos[0], progress_pos[1], progress_size[0],
              progress_size[1]);
    }

    if (focused || progress->active) {
      nvgStrokeColor(ctx->nvg, ui_u_color(progress->active_border_color));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(progress->border_color));
    }

    nvgStrokeWidth(ctx->nvg, progress->border_size);
    nvgStroke(ctx->nvg);
  }

  /* background bar */
  if (progress->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, progress_pos[0], progress_pos[1], progress_size[0],
                   progress_size[1], progress->border_radius);

  } else {
    nvgRect(ctx->nvg, progress_pos[0], progress_pos[1], progress_size[0],
            progress_size[1]);
  }

  if (focused || progress->active) {
    nvgFillColor(ctx->nvg, ui_u_color(progress->active_bg));
  } else {
    nvgFillColor(ctx->nvg, ui_u_color(progress->bg));
  }

  nvgFill(ctx->nvg);

  if (progress->progress > 0.01f) {
    nvgBeginPath(ctx->nvg);
    /* internal bar */
    if (progress->border_radius != 0.f) {
      if (progress->flip) {
        nvgRoundedRect(ctx->nvg,
                       progress_pos[0] + progress_size[0] - fill_size[0] +
                           progress->fill_padding,
                       progress_pos[1] + progress_size[1] - fill_size[1] +
                           progress->fill_padding,
                       fill_size[0] - (progress->fill_padding * 2),
                       fill_size[1] - (progress->fill_padding * 2),
                       progress->border_radius);
      } else {
        nvgRoundedRect(ctx->nvg, progress_pos[0] + progress->fill_padding,
                       progress_pos[1] + progress->fill_padding,
                       fill_size[0] - (progress->fill_padding * 2),
                       fill_size[1] - (progress->fill_padding * 2),
                       progress->border_radius);
      }
    } else {
      if (progress->flip) {
        nvgRect(ctx->nvg,
                progress_pos[0] + progress_size[0] + progress->fill_padding -
                    fill_size[0],
                progress_pos[1] + progress_size[1] + progress->fill_padding -
                    fill_size[1],
                fill_size[0] - (progress->fill_padding * 2),
                fill_size[1] - (progress->fill_padding * 2));
      } else {
        nvgRect(ctx->nvg, progress_pos[0] + progress->fill_padding,
                progress_pos[1] + progress->fill_padding,
                fill_size[0] - (progress->fill_padding * 2),
                fill_size[1] - (progress->fill_padding * 2));
      }
    }

    if (focused || progress->active) {
      nvgFillColor(ctx->nvg, ui_u_color(progress->active_fg));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(progress->fg));
    }

    nvgFill(ctx->nvg);
  }
}

void ui_slider_draw(ui_ctx* ctx, ui_slider* slider, int8_t focused) {
  /* Update the slider */
  float progress = slider->holding_progress;
  if (slider->holding) {
    /* slider stepping */
    if (slider->steps > 0) {
      float step_perc = 1.f / slider->steps;
      int   step      = (int)(progress / step_perc);
      float rem       = progress - (step * step_perc);
      if (rem >= (step_perc * 0.5f)) {
        step += 1;
      }

      float next = step_perc * step;

      if (slider->progress != next) {
        slider->has_change = 1;

        slider->progress = next;
        if (slider->progress > 1.f) {
          slider->progress = 1.f;
        } else if (slider->progress < 0.f) {
          slider->progress = 0.f;
        }

        slider->value =
            slider->min_value +
            ((slider->progress) * (slider->max_value - slider->min_value));
      }
    } else {
      slider->progress = slider->holding_progress;
      slider->value =
          ((slider->max_value - slider->min_value) * slider->progress) +
          slider->min_value;
    }
  }

  vec2 slider_pos, slider_size, slider_button_size, fill_size, button_offset;
  ui_scale_to_px(ctx, slider_pos, slider->position);
  ui_scale_to_px(ctx, slider_size, slider->size);
  ui_scale_to_px(ctx, slider_button_size, slider->button_size);

  vec2_clear(button_offset);
  vec2_clear(fill_size);

  int8_t draw_button = !slider->always_hide_button;

  if (draw_button && slider->auto_hide_button) {
    if (slider->active || slider->holding || focused) {
      draw_button = 1;
    } else {
      draw_button = 0;
    }
  }

  vec2_dup(fill_size, slider_size);
  if (slider->vertical_fill) {
    fill_size[0] = slider_size[0] - (slider->fill_padding * 2.f);
    fill_size[1] = (slider_size[1] * slider->progress);

    if (fill_size[1] >= (slider_size[1] - slider->fill_padding * 2)) {
      fill_size[1] = slider->size[1] - slider->fill_padding * 2.f;
    }

    if (draw_button) {
      if (slider->button_circle) {
        button_offset[0] = slider_size[1];
        if (button_offset[1] < (slider_button_size[1])) {
          button_offset[1] = slider_button_size[1];
        } else if (button_offset[1] >
                   (slider_size[1] - (slider_button_size[1]))) {
          button_offset[1] = slider_size[1] - (slider_button_size[1]);
        }

      } else {
        button_offset[0] = slider_size[0] * 0.5f;
        if (button_offset[1] < (slider_button_size[1] * 0.5f)) {
          button_offset[1] = slider_button_size[1] * 0.5f;
        } else if (button_offset[1] >
                   (slider_size[1] - (slider_button_size[1] * 0.5f))) {
          button_offset[1] = slider_size[1] - (slider_button_size[1] * 0.5f);
        }
      }

      button_offset[1] =
          slider->fill_padding +
          ((slider_size[1] - (slider->fill_padding * 2)) * slider->progress);
    }
  } else {
    fill_size[0] = (slider_size[0] * slider->progress);
    fill_size[1] = slider_size[1] - (slider->fill_padding * 2.f);

    if (fill_size[0] >= slider_size[0] - (slider->fill_padding * 2)) {
      fill_size[0] = slider_size[0] - (slider->fill_padding * 2);
    }

    if (draw_button) {
      button_offset[0] = slider->fill_padding +
                         (((slider_size[0] - (slider->fill_padding * 2)) +
                           (slider_button_size[0] * 0.5f)) *
                          slider->progress);

      if (slider->button_circle) {
        button_offset[1] = slider_size[1];

        if (button_offset[0] < slider_button_size[0]) {
          button_offset[0] = slider_button_size[0];
        } else if (button_offset[0] >
                   (slider_size[0] - (slider_button_size[0] * 0.5f))) {
          button_offset[0] = slider_size[0];
        }
      } else {
        button_offset[1] = slider_size[1] * 0.5f;

        if (button_offset[0] < (slider_button_size[0]) * 0.5f) {
          button_offset[0] = slider_button_size[0] * 0.5f;
        } else if (button_offset[0] >
                   (slider_size[0] - (slider_button_size[0] * 0.5f))) {
          button_offset[0] = slider_size[0] - (slider_button_size[0] * 0.5f);
        }
      }
    }
  }

  nvgBeginPath(ctx->nvg);

  /* border */
  if (slider->border_size != 0.f) {
    if (slider->border_radius != 0.f) {
      nvgRoundedRect(ctx->nvg, slider_pos[0], slider_pos[1], slider_size[0],
                     slider_size[1], slider->border_radius);
    } else {
      nvgRect(ctx->nvg, slider_pos[0], slider_pos[1], slider_size[0],
              slider_size[1]);
    }

    if (focused ||
        ((slider->active || slider->holding) && slider->active_border_color)) {
      nvgStrokeColor(ctx->nvg, ui_u_color(slider->active_border_color));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(slider->border_color));
    }

    nvgStrokeWidth(ctx->nvg, slider->border_size);
    nvgStroke(ctx->nvg);
  }

  /* background bar */
  if (slider->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, slider_pos[0], slider_pos[1], slider_size[0],
                   slider_size[1], slider->border_radius);

  } else {
    nvgRect(ctx->nvg, slider_pos[0], slider_pos[1], slider_size[0],
            slider_size[1]);
  }

  if (focused || slider->active || slider->holding) {
    nvgFillColor(ctx->nvg, ui_u_color(slider->active_bg));
  } else {
    nvgFillColor(ctx->nvg, ui_u_color(slider->bg));
  }

  nvgFill(ctx->nvg);

  /* internal bar */
  if (slider->progress > 0.01f && fill_size[0] > 0.f && fill_size[1] > 0.f) {
    nvgBeginPath(ctx->nvg);
    if (slider->border_radius != 0.f) {
      if (slider->flip) {
        nvgRoundedRect(ctx->nvg,
                       slider_pos[0] + slider_size[0] - fill_size[0] +
                           slider->fill_padding,
                       slider_pos[1] + slider_size[1] - fill_size[1] +
                           slider->fill_padding,
                       fill_size[0], fill_size[1], slider->border_radius);
      } else {
        nvgRoundedRect(ctx->nvg, slider_pos[0] + slider->fill_padding,
                       slider_pos[1] + slider->fill_padding, fill_size[0],
                       fill_size[1], slider->border_radius);
      }
    } else {
      if (slider->flip) {
        nvgRect(ctx->nvg,
                slider_pos[0] + slider_size[0] + slider->fill_padding -
                    fill_size[0],
                slider_pos[1] + slider_size[1] + slider->fill_padding -
                    fill_size[1],
                fill_size[0], fill_size[1]);
      } else {
        nvgRect(ctx->nvg, slider_pos[0] + slider->fill_padding,
                slider_pos[1] + slider->fill_padding, fill_size[0],
                fill_size[1]);
      }
    }

    if (focused || slider->active || slider->holding) {
      nvgFillColor(ctx->nvg, ui_u_color(slider->active_fg));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(slider->fg));
    }

    nvgFill(ctx->nvg);
  }

  /* button */
  if (draw_button) {
    /* button border */
    if (slider->button_border_size != 0.f) {
      nvgBeginPath(ctx->nvg);

      if (slider->button_circle) {
        nvgCircle(
            ctx->nvg,
            slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
            slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
            slider_button_size[1] * 0.5f);
      } else {
        if (slider->button_border_radius != 0.f) {
          nvgRoundedRect(
              ctx->nvg,
              slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
              slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
              slider_button_size[0], slider_button_size[1],
              slider->button_border_radius);
        } else {
          nvgRect(
              ctx->nvg,
              slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
              slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
              slider_button_size[0], slider_button_size[1]);
        }
      }

      nvgStrokeWidth(ctx->nvg, slider->button_border_size);
      if (focused || slider->button_hover || slider->holding) {
        nvgStrokeColor(ctx->nvg,
                       ui_u_color(slider->active_button_border_color));
      } else {
        nvgStrokeColor(ctx->nvg, ui_u_color(slider->button_border_color));
      }
      nvgStroke(ctx->nvg);
    }

    /* button background */
    nvgBeginPath(ctx->nvg);

    if (slider->button_circle) {
      nvgCircle(
          ctx->nvg,
          slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
          slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
          slider_button_size[1] * 0.5f);
    } else {
      if (slider->button_border_radius != 0.f) {
        nvgRoundedRect(
            ctx->nvg,
            slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
            slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
            slider_button_size[0], slider_button_size[1],
            slider->button_border_radius);
      } else {
        nvgRect(
            ctx->nvg,
            slider_pos[0] + button_offset[0] - (slider_button_size[0] * 0.5f),
            slider_pos[1] + button_offset[1] - (slider_button_size[1] * 0.5f),
            slider_button_size[0], slider_button_size[1]);
      }
    }

    if (focused || slider->button_hover || slider->holding) {
      nvgFillColor(ctx->nvg, ui_u_color(slider->active_button_color));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(slider->button_color));
    }

    nvgFill(ctx->nvg);
  }
}

float ui_dropdown_max_font_size(ui_ctx* ctx, ui_dropdown dropdown) {
  if (dropdown.option_count == 0) {
    ASTERA_FUNC_DBG("Dropdown doesn't contain any options.\n");
    return -1.f;
  }

  uint32_t longest_option_len = 0;
  int32_t  longest_option     = -1;
  for (uint32_t i = 0; i < dropdown.option_count; ++i) {
    if (!dropdown.options[i])
      continue;

    uint32_t option_len = (uint32_t)strlen(dropdown.options[i]);
    if (option_len > longest_option_len) {
      longest_option     = (uint32_t)i;
      longest_option_len = option_len;
    }
  }

  if (longest_option == -1) {
    longest_option = 0;
  }

  char* start = dropdown.options[longest_option];
  char* end   = start + longest_option_len;

  float current_size = dropdown.font_size;

  vec4 text_bounds;
  vec4_clear(text_bounds);
  vec2 dropdown_size;
  ui_scale_to_px(ctx, dropdown_size, dropdown.size);

  nvgFontFaceId(ctx->nvg, dropdown.font);

  if (ui_is_type(dropdown.align, UI_ALIGN_CENTER)) {
    nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
  } else {
    nvgTextAlign(ctx->nvg, dropdown.align);
  }

  nvgTextLineHeight(ctx->nvg, dropdown_size[1]);

  float bounds[4];

  while (1) {
    nvgFontSize(ctx->nvg, current_size);
    nvgTextBounds(ctx->nvg, 0.f, 0.f, start, end, bounds);

    float width  = bounds[2] - bounds[0];
    float height = bounds[3] - bounds[1];

    if (width >= dropdown_size[0] || height >= dropdown_size[1]) {
      break;
    } else {
      current_size += 1.f;
    }
  }

  return current_size;
}

void ui_dropdown_draw(ui_ctx* ctx, ui_dropdown* dropdown, int8_t focused) {
  nvgBeginPath(ctx->nvg);

  vec2 dropdown_size, dropdown_position;
  vec2 option_size;
  ui_scale_to_px(ctx, dropdown_size, dropdown->size);
  ui_scale_to_px(ctx, dropdown_position, dropdown->position);
  ui_scale_to_px(ctx, option_size, dropdown->size);

  if (dropdown->showing) {
    dropdown_size[1] *= dropdown->option_display;
  }

  if (dropdown->border_size != 0.f) {
    nvgStrokeWidth(ctx->nvg, dropdown->border_size);

    if (focused) {
      nvgStrokeColor(ctx->nvg, ui_u_color(dropdown->hover_border_color));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(dropdown->border_color));
    }

    if (dropdown->border_radius != 0.f) {
      nvgRoundedRect(ctx->nvg, dropdown_position[0], dropdown_position[1],
                     dropdown_size[0], dropdown_size[1],
                     dropdown->border_radius);
    } else {
      nvgRect(ctx->nvg, dropdown_position[0], dropdown_position[1],
              dropdown_size[0], dropdown_size[1]);
    }

    nvgStroke(ctx->nvg);
  }

  if (focused) {
    nvgFillColor(ctx->nvg, ui_u_color(dropdown->hover_bg));
  } else {
    nvgFillColor(ctx->nvg, ui_u_color(dropdown->bg));
  }

  if (dropdown->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, dropdown_position[0], dropdown_position[1],
                   dropdown_size[0], dropdown_size[1], dropdown->border_radius);
  } else {
    nvgRect(ctx->nvg, dropdown_position[0], dropdown_position[1],
            dropdown_size[0], dropdown_size[1]);
  }

  nvgFill(ctx->nvg);

  /* Showing options */
  nvgFontSize(ctx->nvg, dropdown->font_size);
  nvgFontFaceId(ctx->nvg, dropdown->font);

  if (ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
    nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
  } else {
    nvgTextAlign(ctx->nvg, dropdown->align);
  }

  if (dropdown->showing) {
    int start = dropdown->start;
    for (uint16_t i = 0; i < dropdown->option_display; ++i) {
      int cursor_index = i + start;

      if (cursor_index == dropdown->cursor ||
          cursor_index == dropdown->selected ||
          cursor_index == dropdown->mouse_cursor) {
        if (focused) {
          nvgFillColor(ctx->nvg, ui_u_color(dropdown->hover_select_bg));
        } else {
          nvgFillColor(ctx->nvg, ui_u_color(dropdown->select_bg));
        }

        if (i + start == dropdown->option_display &&
            dropdown->border_radius != 0.f) {
          nvgRoundedRectVarying(ctx->nvg, dropdown_position[0],
                                dropdown_position[1] + (dropdown_size[1] * i),
                                dropdown_size[0], option_size[1], 0.f, 0.f,
                                dropdown->border_radius,
                                dropdown->border_radius);
        } else {
          nvgRect(ctx->nvg, dropdown_position[0],
                  dropdown_position[1] + (option_size[1] * i), dropdown_size[0],
                  option_size[1]);
        }

        nvgFill(ctx->nvg);

        /* Draw overlaying text */
        nvgFillColor(ctx->nvg, ui_u_color(dropdown->hover_color));

        vec2 text_offset = {0.f, option_size[1] * i};

        if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_X) ||
                   ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
          text_offset[0] += option_size[0] / 2.f;
        } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
          text_offset[0] += option_size[0];
        }

        if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_Y) ||
                   ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
          text_offset[1] += (option_size[1] / 2.f);
        } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
          text_offset[1] += option_size[1];
        } else {
          text_offset[1] += option_size[1];
        }

        if (start + i >= dropdown->option_count) {
          printf("out of bounds cursor %i\n", (start + i));
          continue;
        } else if (!dropdown->options[start + i]) {
          printf("Empty string, cursor: %i\n", (start + i));
          continue;
        }

        nvgText(ctx->nvg, dropdown_position[0] + text_offset[0],
                dropdown_position[1] + text_offset[1],
                dropdown->options[cursor_index], 0);
      } else {
        if (start + i >= dropdown->option_count) {
          printf("out of bounds cursor %i\n", (start + i));
          continue;
        } else if (!dropdown->options[start + i]) {
          printf("Empty string, cursor: %i\n", (start + i));
          continue;
        }

        nvgFillColor(ctx->nvg, ui_u_color(dropdown->color));
        vec2 text_offset = {0.f, option_size[1] * i};

        if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_X) ||
                   ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
          text_offset[0] += option_size[0] / 2.f;
        } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
          text_offset[0] += option_size[0];
        }

        if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
        } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_Y) ||
                   ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
          text_offset[1] += (option_size[1] / 2.f);
        } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
          text_offset[1] += option_size[1];
        } else {
          text_offset[1] += option_size[1];
        }

        nvgText(ctx->nvg, dropdown_position[0] + text_offset[0],
                dropdown_position[1] + text_offset[1],
                dropdown->options[start + i], 0);
      }
    }
  } else {
    if (focused) {
      nvgFillColor(ctx->nvg, ui_u_color(dropdown->hover_color));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(dropdown->color));
    }

    vec2 text_offset = {0.f, 0.f};

    if (ui_is_type(dropdown->align, UI_ALIGN_LEFT)) {
    } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_X) ||
               ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
      text_offset[0] = option_size[0] / 2.f;
    } else if (ui_is_type(dropdown->align, UI_ALIGN_RIGHT)) {
      text_offset[0] = option_size[0];
    }

    if (ui_is_type(dropdown->align, UI_ALIGN_TOP)) {
    } else if (ui_is_type(dropdown->align, UI_ALIGN_MIDDLE_Y) ||
               ui_is_type(dropdown->align, UI_ALIGN_CENTER)) {
      text_offset[1] = (option_size[1] / 2.f);
    } else if (ui_is_type(dropdown->align, UI_ALIGN_BOTTOM)) {
      text_offset[1] = option_size[1];
    } else {
      text_offset[1] = option_size[1];
    }

    nvgText(ctx->nvg, dropdown_position[0] + text_offset[0],
            dropdown_position[1] + text_offset[1],
            dropdown->options[dropdown->selected], 0);
  }
}

void ui_line_draw(ui_ctx* ctx, ui_line* line) {
  if (!line) {
    return;
  }

  vec2 line_start, line_end;
  ui_scale_to_px(ctx, line_start, line->start);
  ui_scale_to_px(ctx, line_end, line->end);

  nvgBeginPath(ctx->nvg);
  nvgMoveTo(ctx->nvg, line_start[0], line_start[1]);
  nvgLineTo(ctx->nvg, line_end[0], line_end[1]);
  nvgStrokeWidth(ctx->nvg, line->thickness);
  nvgStrokeColor(ctx->nvg, ui_u_color(line->color));
  nvgStroke(ctx->nvg);
}

void ui_option_draw(ui_ctx* ctx, ui_option* option, int8_t focused) {
  if (!option) {
    return;
  }

  if (option->use_img) {
    nvgBeginPath(ctx->nvg);
    NVGpaint img_paint =
        nvgImagePattern(ctx->nvg, 0.f, 0.f, option->img_size[0],
                        option->img_size[1], 0.f, option->img_handle, 1.0);
    nvgRect(ctx->nvg, option->img_offset[0] + option->position[0],
            option->img_offset[1] + option->position[1], option->img_size[0],
            option->img_size[1]);
    nvgFillPaint(ctx->nvg, img_paint);
    nvgFill(ctx->nvg);
  }

  if (option->use_color) {
    if (focused) {
      nvgFillColor(ctx->nvg, ui_u_color(option->hover_bg));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(option->bg));
    }
  }

  if (option->use_text) {
    nvgBeginPath(ctx->nvg);
    nvgFontFaceId(ctx->nvg, option->font);
    nvgFontSize(ctx->nvg, option->font_size);

    if (ui_is_type(option->align, UI_ALIGN_CENTER)) {
      nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
    } else {
      nvgTextAlign(ctx->nvg, option->align);
    }

    if (focused) {
      nvgFillColor(ctx->nvg, ui_u_color(option->hover_color));
    } else {
      nvgFillColor(ctx->nvg, ui_u_color(option->color));
    }

    nvgText(ctx->nvg, option->position[0], option->position[1], option->text,
            0);
  }
}

void ui_img_draw(ui_ctx* ctx, ui_img* img, int8_t focused) {
  if (!img) {
    return;
  }

  vec2 img_position, img_size;
  ui_scale_to_px(ctx, img_position, img->position);
  ui_scale_to_px(ctx, img_size, img->size);

  /* Draw border */
  if (img->border_size != 0.f) {
    nvgBeginPath(ctx->nvg);
    if (focused) {
      nvgStrokeColor(ctx->nvg, ui_u_color(img->hover_border_color));
    } else {
      nvgStrokeColor(ctx->nvg, ui_u_color(img->border_color));
    }

    nvgStrokeWidth(ctx->nvg, img->border_size);
    nvgStroke(ctx->nvg);
  }

  /* Draw picture */
  nvgBeginPath(ctx->nvg);
  NVGpaint img_paint =
      nvgImagePattern(ctx->nvg, img_position[0], img_position[1], img_size[0],
                      img_size[1], 0.f, img->handle, 1.0f);

  if (img->border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, img_position[0], img_position[1], img_size[0],
                   img_size[1], img->border_radius);
  } else {
    nvgRect(ctx->nvg, img_position[0], img_position[1], img_size[0],
            img_size[1]);
  }

  nvgFillPaint(ctx->nvg, img_paint);
  nvgFill(ctx->nvg);
}

void ui_im_text_draw(ui_ctx* ctx, vec2 pos, float font_size, ui_font font,
                     char* text) {
  vec2 text_pos;
  ui_scale_to_px(ctx, text_pos, pos);

  nvgBeginPath(ctx->nvg);
  nvgFontSize(ctx->nvg, font_size);
  ui_color im_text_color = {1.f, 1.f, 1.f, 1.f};
  nvgFillColor(ctx->nvg, ui_u_color(im_text_color));
  nvgFontFaceId(ctx->nvg, font);
  nvgTextAlign(ctx->nvg, UI_ALIGN_LEFT);
  nvgText(ctx->nvg, text_pos[0], text_pos[1], text, 0);
}

void ui_im_text_draw_aligned(ui_ctx* ctx, vec2 pos, float font_size,
                             ui_font font, int align, char* text) {
  vec2 text_pos;
  ui_scale_to_px(ctx, text_pos, pos);

  nvgBeginPath(ctx->nvg);
  nvgFontSize(ctx->nvg, font_size);
  ui_color im_text_color = {1.f, 1.f, 1.f, 1.f};
  nvgFillColor(ctx->nvg, ui_u_color(im_text_color));
  nvgFontFaceId(ctx->nvg, font);

  if (ui_is_type(align, UI_ALIGN_CENTER)) {
    nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
  } else {
    nvgTextAlign(ctx->nvg, align);
  }

  nvgText(ctx->nvg, text_pos[0], text_pos[1], text, 0);
}

void ui_im_box_draw(ui_ctx* ctx, vec2 pos, vec2 size, ui_color color) {
  vec2 box_pos, box_size;
  ui_scale_to_px(ctx, box_pos, pos);
  ui_scale_to_px(ctx, box_size, size);
  nvgBeginPath(ctx->nvg);
  nvgFillColor(ctx->nvg, ui_u_color(color));
  nvgRect(ctx->nvg, box_pos[0], box_pos[1], box_size[0], box_size[1]);
  nvgFill(ctx->nvg);
}

void ui_im_circle_draw(ui_ctx* ctx, vec2 pos, float radius, float thickness,
                       ui_color color) {
  vec2 circ_pos;
  ui_scale_to_px(ctx, circ_pos, pos);

  nvgBeginPath(ctx->nvg);
  nvgStrokeWidth(ctx->nvg, thickness);
  nvgStrokeColor(ctx->nvg, ui_u_color(color));
  nvgCircle(ctx->nvg, circ_pos[0], circ_pos[1], radius);
  nvgStroke(ctx->nvg);
}

void ui_im_line_draw(ui_ctx* ctx, vec2 start, vec2 end, float thickness,
                     ui_color color) {
  vec2 line_start, line_end;
  ui_scale_to_px(ctx, line_start, start);
  ui_scale_to_px(ctx, line_end, end);

  nvgBeginPath(ctx->nvg);

  nvgMoveTo(ctx->nvg, line_start[0], line_start[1]);
  nvgLineTo(ctx->nvg, line_end[0], line_end[1]);
  nvgStrokeWidth(ctx->nvg, thickness);
  nvgStrokeColor(ctx->nvg, ui_u_color(color));
  nvgLineCap(ctx->nvg, NVG_ROUND);

  nvgStroke(ctx->nvg);
}

void ui_im_img_draw(ui_ctx* ctx, ui_img img, vec2 pos, vec2 size) {
  vec2 img_position, img_size;
  ui_scale_to_px(ctx, img_position, pos);
  ui_scale_to_px(ctx, img_size, size);

  /* Draw border */
  if (img.border_size != 0.f) {
    nvgBeginPath(ctx->nvg);
    nvgStrokeColor(ctx->nvg, ui_u_color(img.border_color));

    nvgStrokeWidth(ctx->nvg, img.border_size);
    nvgStroke(ctx->nvg);
  }

  /* Draw picture */
  nvgBeginPath(ctx->nvg);
  NVGpaint img_paint =
      nvgImagePattern(ctx->nvg, img_position[0], img_position[1], img_size[0],
                      img_size[1], 0.f, img.handle, 1.0f);

  if (img.border_radius != 0.f) {
    nvgRoundedRect(ctx->nvg, img_position[0], img_position[1], img_size[0],
                   img_size[1], img.border_radius);
  } else {
    nvgRect(ctx->nvg, img_position[0], img_position[1], img_size[0],
            img_size[1]);
  }

  nvgFillPaint(ctx->nvg, img_paint);
  nvgFill(ctx->nvg);
}

static int ui_text_fits(ui_ctx* ctx, ui_text text, float size, vec2 scaled_pos,
                        vec2 bounds, int allow_reveal) {
  nvgFontSize(ctx->nvg, size);
  vec4 text_bounds;

  if (text.use_box) {
    nvgTextBoxBounds(
        ctx->nvg, scaled_pos[0], scaled_pos[1], text.bounds[0], text.text,
        (text.use_reveal && allow_reveal) ? text.reveal : 0, text_bounds);
  } else {
    nvgTextBounds(ctx->nvg, scaled_pos[0], scaled_pos[1], text.text,
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

float ui_text_max_size(ui_ctx* ctx, ui_text text, vec2 bounds,
                       int allow_reveal) {
  float current_size = text.size;

  vec2 text_position;
  ui_scale_to_px(ctx, text_position, text.position);

  nvgFontFaceId(ctx->nvg, text.font);

  if (ui_is_type(text.align, UI_ALIGN_CENTER)) {
    nvgTextAlign(ctx->nvg, UI_ALIGN_MIDDLE_X | UI_ALIGN_MIDDLE_Y);
  } else {
    nvgTextAlign(ctx->nvg, text.align);
  }

  if (text.use_spacing) {
    nvgTextLetterSpacing(ctx->nvg, text.spacing);
    nvgTextLineHeight(ctx->nvg, text.line_height);
  }

  if (!ui_text_fits(ctx, text, current_size, text_position, bounds,
                    allow_reveal)) {
    current_size = 1.f;
  }

  while (1) {
    if (!ui_text_fits(ctx, text, current_size, text_position, bounds,
                      allow_reveal)) {
      current_size -= 1.f;
      break;
    } else {
      current_size += 1.f;
    }
  }

  return current_size;
}

void ui_tree_destroy(ui_ctx* ctx, ui_tree* tree) {
  for (uint32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor = &tree->raw[i];
    if (cursor->element.data) {
      switch (cursor->element.type) {
        case UI_IMG:
          ui_img_destroy(ctx, (ui_img*)cursor->element.data);
          break;
        case UI_DROPDOWN:
          ui_dropdown_destroy(ctx, (ui_dropdown*)cursor->element.data);
          break;
        case UI_OPTION:
          ui_option_destroy(ctx, (ui_option*)cursor->element.data);
          break;
        case UI_BUTTON:
          ui_button_destroy((ui_button*)cursor->element.data);
          break;
        case UI_TEXT:
          ui_text_destroy((ui_text*)cursor->element.data);
          break;
        default:
          break;
      }
    }
  }

  free(tree->raw);
}

uint32_t ui_tree_add(ui_ctx* ctx, ui_tree* tree, void* data,
                     ui_element_type type, int8_t priority, int8_t selectable,
                     int16_t layer) {
  if (!data || !tree) {
    return 0;
  }

  if (tree->count == tree->capacity - 1) {
    ASTERA_FUNC_DBG("No free space in tree.\n");
    return 0;
  }

  ui_leaf* leaf_ptr = &tree->raw[tree->count];
  if (!leaf_ptr) {
    ASTERA_FUNC_DBG("Invalid leaf pointer.\n");
    return 0;
  }

  ++ctx->global_id;
  uint32_t uid    = ctx->global_id;
  leaf_ptr->uid   = uid;
  leaf_ptr->index = tree->count;
  leaf_ptr->layer = layer;

  switch (type) {
    case UI_TEXT: {
      ui_text* text = (ui_text*)data;
      text->id      = uid;
    } break;
    case UI_BOX: {
      ui_box* box = (ui_box*)data;
      box->id     = uid;
    } break;
    case UI_BUTTON: {
      ui_button* button = (ui_button*)data;
      button->id        = uid;
    } break;
    case UI_SLIDER: {
      ui_slider* slider = (ui_slider*)data;
      slider->id        = uid;
    } break;
    case UI_PROGRESS: {
      ui_progress* progress = (ui_progress*)data;
      progress->id          = uid;
    } break;
    case UI_LINE: {
      ui_line* line = (ui_line*)data;
      line->id      = uid;
    } break;
    case UI_DROPDOWN: {
      ui_dropdown* dropdown = (ui_dropdown*)data;
      dropdown->id          = uid;
    } break;
    case UI_OPTION: {
      ui_option* option = (ui_option*)data;
      option->id        = uid;
    } break;
    case UI_IMG: {
      ui_img* img = (ui_img*)data;
      img->id     = uid;
    } break;
  }

  leaf_ptr->selectable = selectable;
  leaf_ptr->priority   = (priority >= 127) ? 126 : priority;

  leaf_ptr->element.data = data;
  leaf_ptr->element.type = type;

  ++tree->count;
  return uid;
}

void ui_tree_set_cursor_to(ui_tree* tree, uint32_t id) {
  if (!tree) {
    ASTERA_FUNC_DBG("no tree passed.\n");
    return;
  }

  for (uint32_t i = 0; i < tree->count; ++i) {
    if (tree->raw[i].uid == id && tree->raw[i].selectable) {
      tree->cursor_index = i;
      tree->cursor_id    = id;
      break;
    }
  }
}

void ui_tree_print(ui_tree* tree) {
  if (!tree) {
    ASTERA_FUNC_DBG("No tree passed.\n");
    return;
  }

  ASTERA_DBG("Tree: ");

  if (tree->count == 0) {
    ASTERA_DBG("Empty");
  }

  for (int32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor = &tree->raw[i];

    ASTERA_DBG("[%i]", cursor->index);

    switch (cursor->element.type) {
      case UI_TEXT:
        ASTERA_DBG("Text");
        break;
      case UI_BOX:
        ASTERA_DBG("Box");
        break;
      case UI_OPTION:
        ASTERA_DBG("Option");
        break;
      case UI_BUTTON:
        ASTERA_DBG("Button");
        break;
      case UI_SLIDER:
        ASTERA_DBG("Slider");
        break;
      case UI_PROGRESS:
        ASTERA_DBG("Progress");
        break;
      case UI_DROPDOWN:
        ASTERA_DBG("Dropdown");
        break;
      case UI_LINE:
        ASTERA_DBG("Line");
        break;
      case UI_IMG:
        ASTERA_DBG("Image");
        break;
      default:
        ASTERA_DBG("Unk");
        break;
    }

    if (i < tree->count - 1) {
      ASTERA_DBG(" -> ");
    }
  }

  ASTERA_DBG("\n");
}

/* Returns -1 if no element with uid found */
int32_t ui_tree_check_event(ui_tree* tree, uint32_t uid) {
  if (!tree) {
    ASTERA_FUNC_DBG("invalid tree passed.\n");
    return 0;
  }

  for (uint32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor = &tree->raw[i];
    if (cursor->uid == uid) {
      int32_t event_type = cursor->event;
      cursor->event      = 0;
      return event_type;
    }
  }

  return -1;
}

void ui_tree_reset(ui_tree* tree) {
  tree->mouse_hover_id    = 0;
  tree->cursor_id         = 0;
  tree->mouse_hover_index = 0;
  tree->cursor_index      = 0;
  tree->selected_index    = 0;
  ui_tree_next(tree);
}

uint32_t ui_tree_get_cursor_id(ui_tree* tree) { return tree->cursor_id; }

static int compare_leaf(const void* a, const void* b) {
  ui_leaf** leaf_a = (ui_leaf**)a;
  ui_leaf** leaf_b = (ui_leaf**)b;

  ui_leaf* l_a = *leaf_a;
  ui_leaf* l_b = *leaf_b;

  if (l_a->layer == l_b->layer)
    return 0;
  else if (l_a->layer < l_b->layer)
    return -1;
  else
    return 1;
}

void ui_tree_draw(ui_ctx* ctx, ui_tree* tree) {
  if (!tree || !ctx) {
    ASTERA_FUNC_DBG("Invalid context or tree passed.\n");
    return;
  }

  for (uint32_t i = 0; i < tree->count; ++i) {
    tree->draw_order[i] = &tree->raw[i];
  }

  qsort(tree->draw_order, tree->count, sizeof(ui_leaf*), compare_leaf);

  uint32_t cursor_id      = tree->cursor_id;
  uint32_t mouse_hover_id = 0;

  if (ctx->use_mouse) {
    mouse_hover_id = tree->mouse_hover_id;
  }

  for (uint32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor  = tree->draw_order[i];
    int8_t   focused = 0;

    if (ctx->use_mouse) {
      focused = cursor->uid == cursor_id || cursor->uid == mouse_hover_id;
    } else {
      focused = cursor->uid == cursor_id;
    }

    if (cursor->element.data) {
      ui_element element = cursor->element;

      switch (element.type) {
        case UI_TEXT:
          ui_text_draw(ctx, (ui_text*)element.data);
          break;
        case UI_IMG:
          ui_img_draw(ctx, (ui_img*)element.data, focused);
          break;
        case UI_BOX:
          ui_box_draw(ctx, (ui_box*)element.data, focused);
          break;
        case UI_PROGRESS:
          ui_progress_draw(ctx, (ui_progress*)element.data, focused);
          break;
        case UI_BUTTON:
          ui_button_draw(ctx, (ui_button*)element.data, focused);
          break;
        case UI_SLIDER:
          ui_slider_draw(ctx, (ui_slider*)element.data, focused);
          break;
        case UI_LINE:
          ui_line_draw(ctx, (ui_line*)element.data);
          break;
        case UI_DROPDOWN:
          ui_dropdown_draw(ctx, (ui_dropdown*)element.data, focused);
          break;
        case UI_OPTION:
          ui_option_draw(ctx, (ui_option*)element.data, focused);
          break;
      }
    }
  }
}

ui_text ui_text_create(ui_ctx* ctx, vec2 pos, char* string, float font_size,
                       ui_font font_id, int alignment) {
  ui_text text = (ui_text){0};

  vec2_dup(text.position, pos);

  if (font_size) {
    text.size = font_size;
  } else {
    text.size = ui_attrib_getf(ctx, UI_TEXT_FONT_SIZE);
    if (text.size == -1.f)
      text.size = 16.f;
  }

  text.text = string;

  if (font_id)
    text.font = font_id;
  else {
    int _font = ui_attrib_geti(ctx, UI_TEXT_FONT);
    if (_font == -1) {
      _font = ui_attrib_geti(ctx, UI_DEFAULT_FONT);
      if (_font == -1)
        text.font = 0;
      else
        text.font = _font;
    } else {
      text.font = _font;
    }
  }

  if (alignment != -1)
    text.align = alignment;
  else {
    text.align = ui_attrib_geti(ctx, UI_TEXT_ALIGN);
    if (text.align == -1)
      text.align = 0;
  }

  text.reveal = 0;

  if (string) {
    text.text_length = (int)strlen(string);
  } else {
    text.text_length = 0;
  }

  ui_attrib_getc(ctx, UI_TEXT_COLOR, text.color);
  if (text.color[0] == -1.f)
    ui_color_clear(text.color);

  ui_attrib_getc(ctx, UI_TEXT_SHADOW, text.shadow);
  if (text.shadow[0] == -1.f)
    ui_color_clear(text.shadow);
  else
    text.use_shadow = 1;

  text.shadow_size = ui_attrib_getf(ctx, UI_TEXT_SHADOW_SIZE);
  if (text.shadow_size == -1.f) {
    text.shadow_size = 0.f;
    text.use_shadow  = 0;
  } else if (text.shadow_size > 0.f) {
    text.use_shadow = 1;
  }

  text.line_height = ui_attrib_getf(ctx, UI_TEXT_LINE_HEIGHT);
  if (text.line_height == -1.f) {
    text.line_height = 0.f;
  }

  text.spacing = ui_attrib_getf(ctx, UI_TEXT_SPACING);
  if (text.spacing == -1.f) {
    text.spacing     = 0.f;
    text.use_spacing = 0;
  } else {
    text.use_spacing = 1;
  }

  return text;
}

ui_button ui_button_create(ui_ctx* ctx, vec2 pos, vec2 size, char* text,
                           int text_alignment, float font_size) {
  ui_button button = (ui_button){0};

  if (pos)
    vec2_dup(button.position, pos);

  if (size)
    vec2_dup(button.size, size);
  else {
    ui_attrib_get2f(ctx, UI_BUTTON_SIZE, button.size);
    if (button.size[0] == -1.f)
      vec2_clear(button.size);
  }

  if (text_alignment) {
    button.align = text_alignment;
  } else {
    button.align = ui_attrib_geti(ctx, UI_BUTTON_TEXT_ALIGN);
    if (button.align == -1)
      button.align = 0;
  }

  if (font_size != 0.f) {
    button.font_size = font_size;
  } else {
    button.font_size = ui_attrib_getf(ctx, UI_BUTTON_FONT_SIZE);
    if (button.font_size == -1.f)
      button.font_size = 16.f;
  }

  button.text = text;

  ui_attrib_get4f(ctx, UI_BUTTON_PADDING, button.padding);
  if (button.padding[0] == -1.f)
    vec4_clear(button.padding);

  int font = ui_attrib_geti(ctx, UI_BUTTON_FONT);
  if (button.font == 0) {
    if (font == -1) {
      font = ui_attrib_geti(ctx, UI_DEFAULT_FONT);
    }

    if (font != -1)
      button.font = font;
  }

  ui_attrib_getc(ctx, UI_BUTTON_COLOR, button.color);
  if (button.color[0] == -1.f)
    ui_color_clear(button.color);

  ui_attrib_getc(ctx, UI_BUTTON_BG, button.bg);
  if (button.bg[0] == -1.f)
    ui_color_clear(button.bg);

  ui_attrib_getc(ctx, UI_BUTTON_COLOR_HOVER, button.hover_color);
  if (button.hover_color[0] == -1.f)
    ui_color_clear(button.hover_color);

  ui_attrib_getc(ctx, UI_BUTTON_BG_HOVER, button.hover_bg);
  if (button.hover_bg[0] == -1.f)
    ui_color_clear(button.hover_bg);

  button.border_radius = ui_attrib_getf(ctx, UI_BUTTON_BORDER_RADIUS);
  if (button.border_radius == -1.f)
    button.border_radius = 0.f;

  ui_attrib_getc(ctx, UI_BUTTON_BORDER_COLOR, button.border_color);
  if (button.border_color[0] == -1.f)
    ui_color_clear(button.border_color);

  ui_attrib_getc(ctx, UI_BUTTON_BORDER_COLOR_HOVER, button.hover_border_color);
  if (button.hover_border_color[0] == -1.f)
    ui_color_clear(button.hover_border_color);

  button.border_size = ui_attrib_getf(ctx, UI_BUTTON_BORDER_SIZE);
  if (button.border_size == -1.f)
    button.border_size = 0.f;

  return button;
}

ui_progress ui_progress_create(ui_ctx* ctx, vec2 pos, vec2 size,
                               float progress) {
  ui_progress _progress = (ui_progress){0};

  vec2_dup(_progress.position, pos);

  if (size) {
    vec2_dup(_progress.size, size);
  } else {
    ui_attrib_get2f(ctx, UI_PROGRESS_SIZE, _progress.size);
    if (_progress.size[0] == -1.f)
      vec2_clear(_progress.size);
  }

  _progress.fill_padding = ui_attrib_getf(ctx, UI_PROGRESS_FILL_PADDING);
  if (_progress.fill_padding == -1.f)
    _progress.fill_padding = 0.f;

  ui_attrib_getc(ctx, UI_PROGRESS_BG, _progress.bg);
  if (_progress.bg[0] == -1.f)
    ui_color_clear(_progress.bg);

  ui_attrib_getc(ctx, UI_PROGRESS_FG, _progress.fg);
  if (_progress.fg[0] == -1.f)
    ui_color_clear(_progress.fg);

  ui_attrib_getc(ctx, UI_PROGRESS_BORDER_COLOR, _progress.border_color);
  if (_progress.border_color[0] == -1.f)
    ui_color_clear(_progress.border_color);

  ui_attrib_getc(ctx, UI_PROGRESS_ACTIVE_BG, _progress.active_bg);
  if (_progress.active_bg[0] == -1.f)
    ui_color_clear(_progress.active_bg);

  ui_attrib_getc(ctx, UI_PROGRESS_ACTIVE_FG, _progress.active_fg);
  if (_progress.active_fg[0] == -1.f)
    ui_color_clear(_progress.active_fg);

  ui_attrib_getc(ctx, UI_PROGRESS_ACTIVE_BORDER_COLOR,
                 _progress.active_border_color);
  if (_progress.active_border_color[0] == -1.f)
    ui_color_clear(_progress.active_border_color);

  _progress.border_radius = ui_attrib_getf(ctx, UI_PROGRESS_BORDER_RADIUS);
  if (_progress.border_radius == -1.f)
    _progress.border_radius = 0.f;

  _progress.border_size = ui_attrib_getf(ctx, UI_PROGRESS_BORDER_SIZE);
  if (_progress.border_size == -1.f)
    _progress.border_size = 0.f;

  int vertical = ui_attrib_geti(ctx, UI_PROGRESS_VERTICAL_FILL);
  if (vertical != -1)
    _progress.vertical_fill = (int8_t)vertical;

  _progress.progress = progress;
  _progress.active   = 0;

  return _progress;
}

ui_slider ui_slider_create(ui_ctx* ctx, vec2 pos, vec2 size, vec2 button_size,
                           int round_button, float value, float min, float max,
                           int steps) {
  ui_slider slider = (ui_slider){0};

  vec2_dup(slider.position, pos);

  if (size)
    vec2_dup(slider.size, size);
  else {
    ui_attrib_get2f(ctx, UI_SLIDER_SIZE, slider.size);
    if (slider.size[0] == -1.f)
      vec2_clear(slider.size);
  }

  if (button_size) {
    vec2_dup(slider.button_size, button_size);
  } else {
    ui_attrib_get2f(ctx, UI_SLIDER_BUTTON_SIZE, slider.button_size);
    if (slider.button_size[0] == -1.f)
      vec2_clear(slider.button_size);
  }

  if (round_button != -1) {
    slider.button_circle = (int8_t)round_button;
  } else {
    int bc = ui_attrib_geti(ctx, UI_SLIDER_BUTTON_CIRCLE);
    if (slider.button_circle == -1) {
      slider.button_circle = 0;
    } else {
      slider.button_circle = (int8_t)bc;
    }
  }

  slider.steps     = (int16_t)steps;
  slider.min_value = min;
  slider.max_value = max;
  slider.value     = value;

  int8_t ahb = (int8_t)ui_attrib_geti(ctx, UI_SLIDER_ALWAYS_HIDE_BUTTON);
  if (ahb != -1) {
    slider.always_hide_button = ahb;
  }

  ui_attrib_getc(ctx, UI_SLIDER_BG, slider.bg);
  if (slider.bg[0] == -1.f)
    ui_color_clear(slider.bg);

  ui_attrib_getc(ctx, UI_SLIDER_FG, slider.fg);
  if (slider.fg[0] == -1.f)
    ui_color_clear(slider.fg);

  ui_attrib_getc(ctx, UI_SLIDER_BORDER_COLOR, slider.border_color);
  if (slider.border_color[0] == -1.f)
    ui_color_clear(slider.border_color);

  ui_attrib_getc(ctx, UI_SLIDER_ACTIVE_BG, slider.active_bg);
  if (slider.active_bg[0] == -1.f)
    ui_color_clear(slider.active_bg);

  ui_attrib_getc(ctx, UI_SLIDER_ACTIVE_FG, slider.active_fg);
  if (slider.active_fg[0] == -1.f)
    ui_color_clear(slider.active_fg);

  ui_attrib_getc(ctx, UI_SLIDER_ACTIVE_BORDER_COLOR,
                 slider.active_border_color);
  if (slider.active_border_color[0] == -1.f)
    ui_color_clear(slider.active_border_color);

  slider.border_radius = ui_attrib_getf(ctx, UI_SLIDER_BORDER_RADIUS);
  if (slider.border_radius == -1.f)
    slider.border_radius = 0.f;

  slider.border_size = ui_attrib_getf(ctx, UI_SLIDER_BORDER_SIZE);
  if (slider.border_size == -1.f)
    slider.border_size = 0.f;

  int vertical_fill = ui_attrib_geti(ctx, UI_SLIDER_VERTICAL_FILL);
  if (vertical_fill != -1)
    slider.vertical_fill = (int8_t)vertical_fill;
  else
    slider.vertical_fill = 0;

  slider.button_border_size = ui_attrib_getf(ctx, UI_SLIDER_BUTTON_BORDER_SIZE);
  if (slider.button_border_size == -1.f)
    slider.button_border_size = 0.f;

  ui_attrib_getc(ctx, UI_SLIDER_BUTTON_COLOR, slider.button_color);
  if (slider.button_color[0] == -1.f)
    ui_color_clear(slider.button_color);

  ui_attrib_getc(ctx, UI_SLIDER_BUTTON_ACTIVE_COLOR,
                 slider.active_button_color);
  if (slider.active_button_color[0] == -1.f)
    ui_color_clear(slider.active_button_color);

  ui_attrib_getc(ctx, UI_SLIDER_BUTTON_BORDER_COLOR,
                 slider.button_border_color);
  if (slider.button_border_color[0] == -1.f)
    ui_color_clear(slider.button_border_color);

  ui_attrib_getc(ctx, UI_SLIDER_BUTTON_ACTIVE_BORDER_COLOR,
                 slider.active_button_border_color);
  if (slider.active_button_border_color[0] == -1.f)
    ui_color_clear(slider.active_button_border_color);

  slider.button_border_radius =
      ui_attrib_getf(ctx, UI_SLIDER_BUTTON_BORDER_RADIUS);
  if (slider.button_border_radius == -1.f)
    slider.button_border_radius = 0.f;

  slider.button_border_size = ui_attrib_getf(ctx, UI_SLIDER_BUTTON_BORDER_SIZE);
  if (slider.button_border_size == -1.f)
    slider.button_border_size = 0.f;

  int flip = ui_attrib_geti(ctx, UI_SLIDER_FLIP);
  if (flip != -1)
    slider.flip = (int8_t)flip;
  else
    slider.flip = 0;

  slider.progress = (value - min) / (max - min);

  return slider;
}

ui_line ui_line_create(ui_ctx* ctx, vec2 start, vec2 end, ui_color color,
                       float thickness) {
  ui_line line = (ui_line){0};

  vec2_dup(line.start, start);
  vec2_dup(line.end, end);
  ui_color_dup(line.color, color);

  if (color) {
    ui_color_dup(line.color, color);
  } else {
    ui_attrib_getc(ctx, UI_LINE_COLOR, line.color);
    if (line.color[0] == -1.f)
      ui_color_clear(line.color);
  }

  if (line.thickness != -1.f) {
    line.thickness = thickness;
  } else {
    ui_attrib_getf(ctx, UI_LINE_THICKNESS);
    if (line.thickness == -1.f)
      line.thickness = 0.f;
  }

  return line;
}

ui_dropdown ui_dropdown_create(ui_ctx* ctx, vec2 pos, vec2 size, char** options,
                               uint16_t option_count) {
  ui_dropdown dropdown = (ui_dropdown){0};

  if (option_count > 0) {
    char** _options = (char**)malloc(sizeof(char*) * (option_count + 1));
    for (uint16_t i = 0; i < option_count; ++i) {
      int str_len = (int)strlen(options[i]);

      _options[i] = (char*)malloc(sizeof(char) * (str_len + 1));
      memcpy(_options[i], options[i], sizeof(char) * (str_len + 1));

      _options[i][str_len] = 0;
    }

    _options[option_count] = 0;

    dropdown.options         = (char**)_options;
    dropdown.option_count    = option_count;
    dropdown.option_capacity = option_count + 1;
  } else {
    dropdown.options         = (char**)malloc(sizeof(char*) * 8);
    dropdown.option_count    = 0;
    dropdown.option_capacity = 8;

    if (!dropdown.options) {
      ASTERA_FUNC_DBG("Unable to allocate space for dropdown options\n");
      return (ui_dropdown){0};
    }
  }

  dropdown.selected = 0;
  dropdown.cursor   = 0;
  vec2_dup(dropdown.position, pos);

  if (vec2_nonzero(size)) {
    vec2_dup(dropdown.size, size);
  } else {
    ui_attrib_get2f(ctx, UI_DROPDOWN_SIZE, dropdown.size);

    if (dropdown.size[0] == -1.f) {
      vec2_clear(dropdown.size);
    }
  }

  dropdown.border_radius = ui_attrib_getf(ctx, UI_DROPDOWN_BORDER_RADIUS);

  if (dropdown.border_radius == -1.f)
    dropdown.border_radius = 0.f;

  dropdown.border_size = ui_attrib_getf(ctx, UI_DROPDOWN_BORDER_SIZE);

  if (dropdown.border_size == -1.f)
    dropdown.border_size = 0.f;

  ui_attrib_getc(ctx, UI_DROPDOWN_BORDER_COLOR, dropdown.border_color);
  if (dropdown.border_color[0] == -1.f)
    ui_color_clear(dropdown.border_color);

  ui_attrib_getc(ctx, UI_DROPDOWN_BORDER_COLOR_HOVER,
                 dropdown.hover_border_color);
  if (dropdown.hover_border_color[0] == -1.f)
    ui_color_clear(dropdown.hover_border_color);

  dropdown.font_size = ui_attrib_getf(ctx, UI_DROPDOWN_FONT_SIZE);
  if (dropdown.font_size == -1.f)
    dropdown.font_size = 16.f;

  dropdown.align = ui_attrib_geti(ctx, UI_DROPDOWN_ALIGN);

  if (dropdown.align == -1) {
    dropdown.align = UI_ALIGN_CENTER;
  }

  int font = ui_attrib_geti(ctx, UI_DROPDOWN_FONT);
  if (font == -1) {
    font = ui_attrib_geti(ctx, UI_DEFAULT_FONT);

    if (font == -1)
      dropdown.font = 0;
    else
      dropdown.font = font;
  } else {
    dropdown.font = font;
  }

  ui_attrib_getc(ctx, UI_DROPDOWN_COLOR, dropdown.color);
  if (dropdown.color[0] == -1.f) {
    ASTERA_FUNC_DBG("Invalid dropdown color\n");
    ui_color_clear(dropdown.color);
  }

  ui_attrib_getc(ctx, UI_DROPDOWN_COLOR_HOVER, dropdown.hover_color);
  if (dropdown.hover_color[0] == -1.f)
    ui_color_clear(dropdown.hover_color);

  ui_attrib_getc(ctx, UI_DROPDOWN_BG, dropdown.bg);
  if (dropdown.bg[0] == -1.f) {
    ui_color_clear(dropdown.bg);
    ASTERA_FUNC_DBG("Invalid dropdown bg\n");
  }

  ui_attrib_getc(ctx, UI_DROPDOWN_BG_HOVER, dropdown.hover_bg);
  if (dropdown.hover_bg[0] == -1.f)
    ui_color_clear(dropdown.hover_bg);

  ui_attrib_getc(ctx, UI_DROPDOWN_SELECT_COLOR, dropdown.select_color);
  if (dropdown.select_color[0] == -1.f)
    ui_color_clear(dropdown.select_color);

  ui_attrib_getc(ctx, UI_DROPDOWN_SELECT_BG, dropdown.select_bg);
  if (dropdown.select_bg[0] == -1.f)
    ui_color_clear(dropdown.select_bg);

  ui_attrib_getc(ctx, UI_DROPDOWN_SELECT_COLOR_HOVER,
                 dropdown.hover_select_color);
  if (dropdown.hover_select_color[0] == -1.f)
    ui_color_clear(dropdown.select_color);

  ui_attrib_getc(ctx, UI_DROPDOWN_SELECT_BG_HOVER, dropdown.hover_select_bg);
  if (dropdown.hover_select_bg[0] == -1.f)
    ui_color_clear(dropdown.select_bg);

  dropdown.showing = 0;

  return dropdown;
}

ui_option ui_option_create(ui_ctx* ctx, vec2 pos, vec2 size, const char* text,
                           float font_size, int32_t text_alignment) {
  ui_option option = (ui_option){0};

  vec2_dup(option.position, pos);

  if (vec2_nonzero(size)) {
    vec2_dup(option.size, size);
  } else {
    ui_attrib_get2f(ctx, UI_OPTION_SIZE, option.size);
    if (option.size[0] == -1.f)
      vec2_clear(option.size);
  }

  int32_t option_img_attrib = ui_attrib_geti(ctx, UI_OPTION_IMAGE);
  if (option_img_attrib > -1) {
    option.img_handle = option_img_attrib;
    option.use_img    = 1;
  } else {
    option.img_handle = -1;
    option.use_img    = 0;
  }

  vec2 option_img_size_attrib;
  ui_attrib_get2f(ctx, UI_OPTION_IMAGE_SIZE, option_img_size_attrib);
  if (option_img_size_attrib[0] != -1.f) {
    vec2_dup(option.img_size, option_img_size_attrib);
  } else {
    vec2_clear(option.img_size);
  }

  int32_t _font = ui_attrib_geti(ctx, UI_OPTION_FONT);
  if (_font != -1) {
    option.font = _font;
  } else {
    _font = ui_attrib_geti(ctx, UI_DEFAULT_FONT);
    if (_font != -1)
      option.font = _font;

    option.font = 0;
  }

  if (font_size != -1.f) {
    option.font_size = font_size;
  } else {
    option.font_size = ui_attrib_getf(ctx, UI_OPTION_FONT_SIZE);
    if (option.font_size == -1.f)
      option.font_size = 16.f;
  }

  if (text_alignment != -1) {
    option.align = text_alignment;
  } else {
    option.align = ui_attrib_geti(ctx, UI_OPTION_TEXT_ALIGN);
    if (option.align == -1)
      option.align = 0;
  }

  if (text) {
    option.text     = (char*)text;
    option.use_text = 1;
  } else {
    option.use_text = 0;
  }

  ui_attrib_get2f(ctx, UI_OPTION_IMAGE_OFFSET, option.img_offset);
  if (option.img_offset[0] == -1.f)
    vec2_clear(option.img_offset);

  ui_attrib_getc(ctx, UI_OPTION_BG, option.bg);
  if (option.bg[0] == -1.f)
    ui_color_clear(option.bg);

  ui_attrib_getc(ctx, UI_OPTION_COLOR, option.color);
  if (option.color[0] == -1.f)
    ui_color_clear(option.color);

  ui_attrib_getc(ctx, UI_OPTION_BG_HOVER, option.hover_bg);
  if (option.hover_bg[0] == -1.f)
    ui_color_clear(option.hover_bg);

  ui_attrib_getc(ctx, UI_OPTION_COLOR_HOVER, option.hover_color);
  if (option.hover_color[0] == -1.f)
    ui_color_clear(option.hover_color);

  option.state     = 0;
  option.use_color = 0;

  return option;
}

ui_box ui_box_create(ui_ctx* ctx, vec2 pos, vec2 size) {
  ui_box box = {0};
  vec2_dup(box.position, pos);

  if (vec2_nonzero(size))
    vec2_dup(box.size, size);
  else {
    ui_attrib_get2f(ctx, UI_BOX_SIZE, box.size);
    if (box.size[0] == -1.f)
      vec2_clear(box.size);
  }

  ui_attrib_getc(ctx, UI_BOX_BG, box.bg);
  if (box.bg[0] == -1.f)
    ui_color_clear(box.bg);

  ui_attrib_getc(ctx, UI_BOX_BORDER_COLOR, box.border_color);
  if (box.border_color[0] == -1.f)
    ui_color_clear(box.border_color);

  ui_attrib_getc(ctx, UI_BOX_BG_HOVER, box.hover_bg);
  if (box.hover_bg[0] == -1.f)
    ui_color_clear(box.hover_bg);

  ui_attrib_getc(ctx, UI_BOX_BORDER_COLOR_HOVER, box.hover_border_color);
  if (box.hover_border_color[0] == -1.f)
    ui_color_clear(box.hover_border_color);

  box.border_size = ui_attrib_getf(ctx, UI_BOX_BORDER_SIZE);
  if (box.border_size == -1.f)
    box.border_size = 0.f;

  box.border_radius = ui_attrib_getf(ctx, UI_BOX_BORDER_RADIUS);
  if (box.border_radius == -1.f)
    box.border_radius = 0.f;

  return box;
}

ui_img ui_img_create(ui_ctx* ctx, unsigned char* data, int data_length,
                     ui_img_flags flags, vec2 pos, vec2 size) {
  if (!data || !data_length) {
    ASTERA_FUNC_DBG("No data passed.\n");
    return (ui_img){0};
  }

  int32_t image_handle = nvgCreateImageMem(ctx->nvg, flags, data, data_length);
  ui_img  img          = (ui_img){.handle = image_handle};
  vec2_dup(img.position, pos);
  vec2_dup(img.size, size);
  return img;
}

void ui_dropdown_set_colors(ui_dropdown* dropdown, ui_color bg,
                            ui_color hover_bg, ui_color fg, ui_color hover_fg,
                            ui_color border_color, ui_color hover_border_color,
                            ui_color select_bg, ui_color select_fg,
                            ui_color hover_select_bg,
                            ui_color hover_select_fg) {
  if (bg) {
    ui_color_dup(dropdown->bg, bg);
  }

  if (hover_bg) {
    ui_color_dup(dropdown->hover_bg, hover_bg);
  }

  if (fg) {
    ui_color_dup(dropdown->color, fg);
  }

  if (hover_fg) {
    ui_color_dup(dropdown->hover_color, hover_fg);
  }

  if (border_color) {
    ui_color_dup(dropdown->border_color, border_color);
  }

  if (hover_border_color) {
    ui_color_dup(dropdown->hover_border_color, hover_border_color);
  }

  if (select_bg) {
    ui_color_dup(dropdown->select_bg, select_bg);
  }

  if (hover_select_bg) {
    ui_color_dup(dropdown->hover_select_bg, hover_select_bg);
  }

  if (select_fg) {
    ui_color_dup(dropdown->select_color, select_fg);
  }

  if (hover_select_fg) {
    ui_color_dup(dropdown->hover_select_color, hover_select_fg);
  }
}

void ui_box_set_colors(ui_box* box, ui_color bg, ui_color hover_bg,
                       ui_color border_color, ui_color hover_border_color) {
  if (bg) {
    ui_color_dup(box->bg, bg);
  }

  if (hover_bg) {
    ui_color_dup(box->hover_bg, hover_bg);
  }

  if (border_color) {
    ui_color_dup(box->border_color, border_color);
  }

  if (hover_border_color) {
    ui_color_dup(box->hover_border_color, hover_border_color);
  }
}

void ui_text_set_colors(ui_text* text, ui_color color, ui_color shadow) {
  if (color) {
    ui_color_dup(text->color, color);
  }

  if (shadow) {
    ui_color_dup(text->shadow, shadow);
  }
}

void ui_button_set_colors(ui_button* button, ui_color bg, ui_color hover_bg,
                          ui_color fg, ui_color hover_fg, ui_color border_color,
                          ui_color hover_border_color) {
  if (bg) {
    ui_color_dup(button->bg, bg);
  }

  if (hover_bg) {
    ui_color_dup(button->hover_bg, hover_bg);
  }

  if (fg) {
    ui_color_dup(button->color, fg);
  }

  if (hover_fg) {
    ui_color_dup(button->hover_color, hover_fg);
  }

  if (border_color) {
    ui_color_dup(button->border_color, border_color);
  }

  if (hover_border_color) {
    ui_color_dup(button->hover_border_color, hover_border_color);
  }
}

void ui_progress_set_colors(ui_progress* progress, ui_color bg,
                            ui_color active_bg, ui_color fg, ui_color active_fg,
                            ui_color border_color,
                            ui_color active_border_color) {
  if (!progress) {
    ASTERA_FUNC_DBG("no progress passed.\n");
    return;
  }

  if (bg) {
    ui_color_dup(progress->bg, bg);
  }

  if (active_bg) {
    ui_color_dup(progress->active_bg, active_bg);
  }

  if (fg) {
    ui_color_dup(progress->fg, fg);
  }

  if (active_fg) {
    ui_color_dup(progress->active_fg, active_fg);
  }

  if (border_color) {
    ui_color_dup(progress->border_color, border_color);
  }

  if (active_border_color) {
    ui_color_dup(progress->active_border_color, active_border_color);
  }
}

void ui_slider_set_colors(ui_slider* slider, ui_color bg, ui_color active_bg,
                          ui_color fg, ui_color active_fg,
                          ui_color border_color, ui_color active_border_color,
                          ui_color button_color, ui_color active_button_color,
                          ui_color button_border_color,
                          ui_color active_button_border_color) {
  if (!slider) {
    return;
  }

  if (bg) {
    ui_color_dup(slider->bg, bg);
  }

  if (active_bg) {
    ui_color_dup(slider->active_bg, active_bg);
  }

  if (fg) {
    ui_color_dup(slider->fg, fg);
  }

  if (active_fg) {
    ui_color_dup(slider->active_fg, fg);
  }

  if (border_color) {
    ui_color_dup(slider->border_color, border_color);
  }

  if (active_border_color) {
    ui_color_dup(slider->active_border_color, active_border_color);
  }

  if (button_color) {
    ui_color_dup(slider->button_color, button_color);
  }

  if (active_button_color) {
    ui_color_dup(slider->active_button_color, active_button_color);
  }

  if (button_border_color) {
    ui_color_dup(slider->button_border_color, button_border_color);
  }

  if (active_button_border_color) {
    ui_color_dup(slider->active_button_border_color,
                 active_button_border_color);
  }
}

void ui_line_set_colors(ui_line* line, ui_color color) {
  if (color) {
    ui_color_dup(line->color, color);
  }
}

void ui_option_set_colors(ui_option* option, ui_color bg, ui_color hover_bg,
                          ui_color fg, ui_color hover_fg) {
  if (bg) {
    ui_color_dup(option->bg, bg);
  }

  if (hover_bg) {
    ui_color_dup(option->hover_bg, hover_bg);
  }

  if (fg) {
    ui_color_dup(option->color, fg);
  }

  if (hover_fg) {
    ui_color_dup(option->hover_color, hover_fg);
  }
}

void ui_img_set_colors(ui_img* img, ui_color border_color,
                       ui_color hover_border_color) {
  if (border_color) {
    ui_color_dup(img->border_color, border_color);
  }

  if (hover_border_color) {
    ui_color_dup(img->hover_border_color, hover_border_color);
  }
}

void ui_img_set_border_radius(ui_img* img, float radius) {
  img->border_radius = radius;
}

void ui_button_set_border_radius(ui_button* button, float radius) {
  button->border_radius = radius;
}

void ui_box_set_border_radius(ui_box* box, float radius) {
  box->border_radius = radius;
}

void ui_dropdown_set_border_radius(ui_dropdown* dropdown, float radius) {
  dropdown->border_radius = radius;
}

uint16_t ui_dropdown_add_option(ui_dropdown* dropdown, const char* option) {
  if (dropdown->option_count == dropdown->option_capacity) {
    dropdown->options =
        (char**)realloc((void*)dropdown->options,
                        sizeof(char*) * dropdown->option_capacity + 4);
    dropdown->option_capacity += 4;
    if (!dropdown->options) {
      ASTERA_FUNC_DBG("Unable to expand memory for dropdown options\n");
      return 0;
    }
  }

  dropdown->options[dropdown->option_count] = (char*)option;
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

uint16_t ui_dropdown_scroll_down(ui_dropdown* dropdown, uint16_t amount) {
  uint16_t distance =
      dropdown->option_count - (dropdown->start + dropdown->option_display);
  uint16_t move = (distance > amount) ? amount : distance;
  dropdown->start += move;
  return amount;
}

uint16_t ui_dropdown_scroll_up(ui_dropdown* dropdown, uint16_t amount) {
  uint16_t distance = dropdown->start;
  uint16_t move     = (distance > amount) ? amount : distance;
  dropdown->start -= move;
  return move;
}

void ui_dropdown_next(ui_dropdown* dropdown) {
  if (dropdown->cursor < dropdown->option_count - 1) {
    dropdown->cursor += 1;
  }

  uint16_t rel = dropdown->cursor - dropdown->start;

  if (dropdown->option_display - (rel) <= dropdown->bottom_scroll_pad) {
    ui_dropdown_scroll_down(dropdown, 1);
  }
}

void ui_dropdown_prev(ui_dropdown* dropdown) {
  if (dropdown->cursor > 0) {
    dropdown->cursor--;
  }

  int rel = (dropdown->cursor - dropdown->start);

  if (rel < dropdown->top_scroll_pad) {
    ui_dropdown_scroll_up(dropdown, 1);
  }
}

void ui_dropdown_set_to_cursor(ui_dropdown* dropdown) {
  if (dropdown->cursor >= dropdown->option_capacity) {
    printf("Preemptive\n");
    dropdown->cursor = dropdown->option_capacity - 1;
    return;
  }

  if (dropdown->selected != dropdown->cursor) {
    dropdown->has_change = 1;
  }

  dropdown->selected = dropdown->cursor;
  int32_t new_start  = dropdown->selected - (dropdown->option_display / 2);

  if (new_start > dropdown->option_count - dropdown->option_display) {
    new_start = dropdown->option_count - dropdown->option_display;
  } else if (new_start < 0) {
    new_start = 0;
  }

  dropdown->start = (uint16_t)new_start;
}

void ui_dropdown_set_scroll(ui_dropdown* dropdown, uint8_t top_scroll_pad,
                            uint8_t bottom_scroll_pad) {
  dropdown->top_scroll_pad    = top_scroll_pad;
  dropdown->bottom_scroll_pad = bottom_scroll_pad;
}

void ui_dropdown_set(ui_dropdown* dropdown, uint16_t select) {
  if (select > dropdown->option_capacity) {
    return;
  }

  if (select != dropdown->selected) {
    dropdown->has_change = 1;
  }

  dropdown->selected = select;

  uint16_t half_length = dropdown->option_display / 2, new_start = 0;

  if (select > (half_length + 1)) {
    new_start = select - (half_length - 1);
  } else if (select > dropdown->option_count - (half_length)) {
    new_start = dropdown->option_count - dropdown->option_display;
  } else if (select < half_length) {
    new_start = 0;
  }

  dropdown->start = (uint16_t)new_start;
}

int8_t ui_dropdown_has_change(ui_dropdown* dropdown) {
  int8_t dropdown_change = dropdown->has_change;
  dropdown->has_change   = 0;
  return dropdown_change;
}

float ui_slider_next_step(ui_slider* slider) {
  if (slider->progress >= 1.f) {
    slider->progress = 1.f;
    return slider->max_value;
  } else if (slider->progress < 0.f) {
    slider->progress = 0.f;
  }

  if (slider->steps) {
    float step_amount = 1.f / slider->steps;
    slider->progress += step_amount;
  } else {
    slider->progress += 0.05f;
  }

  if (slider->progress > 1.f)
    slider->progress = 1.f;

  slider->value = (slider->progress * (slider->max_value - slider->min_value)) +
                  slider->min_value;

  return slider->value;
}

float ui_slider_prev_step(ui_slider* slider) {
  if (slider->progress <= 0.f) {
    slider->progress = 0.f;
    return slider->min_value;
  } else if (slider->progress >= 1.f) {
    slider->progress = 1.f;
  }

  if (slider->steps) {
    float step_amount = 1.f / slider->steps;
    slider->progress -= step_amount;
  } else {
    slider->progress -= 0.05f;
  }

  if (slider->progress < 0.f)
    slider->progress = 0.f;

  slider->value = (slider->progress * (slider->max_value - slider->min_value)) +
                  slider->min_value;

  return slider->value;
}

ui_font ui_font_get(ui_ctx* ctx, const char* font_name) {
  /* Find the font via NanoVG */
  return nvgFindFont(ctx->nvg, font_name);
}

ui_font ui_font_create(ui_ctx* ctx, unsigned char* data, int data_length,
                       const char* name) {
  int font_id = nvgCreateFontMem(ctx->nvg, name, data, data_length, 0);
  return font_id;
}

void ui_tree_remove_id(ui_tree* tree, uint32_t id) {
  int32_t remove_index = -1;
  for (uint32_t i = 0; i < tree->count; ++i) {
    if (tree->raw[i].uid == id) {
      remove_index = (int32_t)i;
      break;
    }
  }

  if (remove_index) {
    uint32_t cursor_id      = tree->cursor_id;
    uint32_t mouse_hover_id = tree->mouse_hover_id;

    for (int32_t i = remove_index; i < tree->count - 1; ++i) {
      tree->raw[i] = tree->raw[i + 1];

      ui_leaf* leaf = &tree->raw[i];
      leaf->index   = i;
    }

    tree->raw[tree->count - 1] = (ui_leaf){0};
    --tree->count;

    if (cursor_id == id) {
      tree->cursor_id = 0;
      cursor_id       = 0;
    }

    if (mouse_hover_id == id) {
      tree->mouse_hover_id = 0;
      mouse_hover_id       = 0;
    }

    for (uint32_t i = 0; i < tree->count; ++i) {
      if (tree->raw[i].uid == cursor_id) {
        tree->cursor_index = i;
      }

      if (tree->raw[i].uid == mouse_hover_id) {
        tree->mouse_hover_index = i;
      }
    }
  }
}

void ui_img_destroy(ui_ctx* ctx, ui_img* img) {
  nvgDeleteImage(ctx->nvg, img->handle);
}

void ui_dropdown_destroy(ui_ctx* ctx, ui_dropdown* dropdown) {
  for (uint16_t i = 0; i < dropdown->option_count; ++i) {
    free(dropdown->options[i]);
  }
  free(dropdown->options);
}

void ui_button_destroy(ui_button* button) {
  if (button->text) {
    free(button->text);
  }
}

void ui_option_destroy(ui_ctx* ctx, ui_option* option) {
  if (option->text) {
    free(option->text);
  }

  if (option->use_img) {
    nvgDeleteImage(ctx->nvg, option->img_handle);
  }
}

void ui_text_destroy(ui_text* text) {
  if (text->text) {
    free(text->text);
  }
}

void ui_text_bounds(ui_ctx* ctx, ui_text* text, vec4 bounds) {
  if (text->use_box) {
    nvgTextBoxBounds(ctx->nvg, text->position[0], text->position[1],
                     text->bounds[0], text->text, 0, bounds);
  } else {
    nvgTextBounds(ctx->nvg, text->position[0], text->position[1], text->text, 0,
                  bounds);
  }
}

/* Return 0 if no cursor set for respective type (mouse / non-mouse)*/
uint32_t ui_tree_select(ui_ctx* ctx, ui_tree* tree, int32_t event_type,
                        int is_mouse) {
  if (ctx->use_mouse && is_mouse) {
    for (uint32_t i = 0; i < tree->count; ++i) {
      ui_leaf* leaf = &tree->raw[i];

      if (leaf->uid == tree->mouse_hover_id || leaf->uid == tree->cursor_id) {
        continue;
      }
    }

    if (tree->mouse_hover_id) {
      if (tree->selected_index) {
        if (tree->mouse_hover_index == tree->selected_index) {
          tree->raw[tree->mouse_hover_index].event = event_type;

          if (tree->raw[tree->mouse_hover_index].element.type == UI_DROPDOWN) {
            ui_dropdown* dropdown =
                (ui_dropdown*)tree->raw[tree->mouse_hover_index].element.data;

            int16_t selection = ui_element_contains(
                ctx, tree->raw[tree->mouse_hover_index].element,
                ctx->mouse_pos);

            printf("%i\n", selection);
            if (selection > -1 && dropdown->showing) {
              ui_dropdown_set(
                  dropdown,
                  (uint16_t)((tree->mouse_hover_index - 1) + dropdown->start));
              dropdown->showing = 0;
            } else if (selection > 0 && !dropdown->showing) {
              dropdown->showing = 1;
            }
          }

          if (tree->raw[tree->mouse_hover_index].element.type == UI_SLIDER) {
            ui_slider* slider =
                (ui_slider*)tree->raw[tree->mouse_hover_index].element.data;
            if (event_type == 1) {
              slider->holding = 1;
            } else if (event_type == 0) {
              slider->holding      = 0;
              tree->selected_index = 0;
            }
          }

          return tree->mouse_hover_id;
        }
      } else {
        tree->raw[tree->mouse_hover_index].event = event_type;

        if (tree->raw[tree->mouse_hover_index].element.type == UI_DROPDOWN) {
          ui_dropdown* dropdown =
              (ui_dropdown*)tree->raw[tree->mouse_hover_index].element.data;
          int16_t selection = ui_element_contains(
              ctx, tree->raw[tree->mouse_hover_index].element, ctx->mouse_pos);

          if (selection > -1 && dropdown->showing) {
            ui_dropdown_set(dropdown, (uint16_t)(tree->mouse_hover_index +
                                                 dropdown->start));
          } else if (selection > 0 && !dropdown->showing) {
            dropdown->showing = 1;
          }
        }

        if (tree->raw[tree->mouse_hover_index].element.type == UI_SLIDER) {
          ui_slider* slider =
              (ui_slider*)tree->raw[tree->mouse_hover_index].element.data;
          if (event_type == 1) {
            slider->holding      = 1;
            tree->selected_index = tree->mouse_hover_index;
          } else if (event_type == 0) {
            slider->holding      = 0;
            tree->selected_index = 0;
          }
        }

        return tree->mouse_hover_id;
      }
    }
  } else if (tree->cursor_id != 0) {
    ui_leaf* leaf = &tree->raw[tree->cursor_index];

    if (leaf->element.type == UI_DROPDOWN) {
      ui_dropdown* dropdown = (ui_dropdown*)leaf->element.data;
      if (event_type != 0) {
        if (dropdown->showing) {
          ui_dropdown_set_to_cursor(dropdown);
          dropdown->showing = 0;
        } else {
          dropdown->showing = 1;
        }
      }
    }

    tree->raw[tree->cursor_index].event = event_type;

    return tree->cursor_id;
  }

  return 0;
}

/* NOTE: You're not gonna mouse click something with this function
 * Return 0 if no element with `id` found */
uint32_t ui_tree_select_id(ui_tree* tree, uint32_t id, int32_t event_type) {
  if (!tree) {
    ASTERA_FUNC_DBG("unable to operate on null or invalid tree\n");
    return 0;
  }

  for (uint32_t i = 0; i < tree->count; ++i) {
    ui_leaf* cursor = &tree->raw[i];

    if (cursor->uid == id) {
      cursor->event = event_type;
      return cursor->uid;
    }
  }

  return 0;
}

int8_t ui_tree_is_active(ui_ctx* ctx, ui_tree* tree, uint32_t id) {
  if (ctx->use_mouse) {
    if (tree->mouse_hover_id == id) {
      return 1;
    }
  }

  if (tree->cursor_id == id) {
    return 1;
  }

  return 0;
}

uint32_t ui_tree_scroll_down(ui_tree* tree, uint16_t amount, uint8_t is_mouse) {
  ui_leaf* cursor;
  if (is_mouse) {
    cursor = &tree->raw[tree->mouse_hover_index];
  } else {
    cursor = &tree->raw[tree->cursor_index];
  }

  if (!cursor) {
    return 0;
  }

  if (cursor->element.type == UI_DROPDOWN) {
    ui_dropdown_scroll_down((ui_dropdown*)cursor->element.data, amount);
  }

  return cursor->uid;
}

uint32_t ui_tree_scroll_up(ui_tree* tree, uint16_t amount, uint8_t is_mouse) {
  ui_leaf* cursor;
  if (is_mouse) {
    cursor = &tree->raw[tree->mouse_hover_index];
  } else {
    cursor = &tree->raw[tree->cursor_index];
  }

  if (!cursor) {
    return 0;
  }

  if (cursor->element.type == UI_DROPDOWN) {
    ui_dropdown_scroll_up((ui_dropdown*)cursor->element.data, amount);
  }

  return cursor->uid;
}

/* Return 0 if invalid operation (all ui id's are non-zero)*/
uint32_t ui_tree_next(ui_tree* tree) {
  if (!tree || tree->count < 2) {
    ASTERA_FUNC_DBG("no or empty tree passed.\n");
    return 0;
  }

  ui_leaf* cursor = &tree->raw[tree->cursor_index];

  if (!cursor) {
    ASTERA_FUNC_DBG("unable to get cursor from array.\n");
    return 0;
  }

  if (cursor->element.type == UI_DROPDOWN) {
    ui_dropdown* dropdown = (ui_dropdown*)cursor->element.data;
    if (dropdown->showing) {
      ui_dropdown_next(dropdown);
      return dropdown->id;
    }
  }

  uint32_t start_index = tree->cursor_index + 1;
  if (start_index >= tree->count && tree->loop) {
    start_index = 0;
  }

  for (uint32_t i = start_index; i < tree->count; ++i) {
    ui_leaf* current = &tree->raw[i];
    if (current->selectable) {
      tree->cursor_id    = current->uid;
      tree->cursor_index = i;
      return cursor->uid;
    }
  }

  if (tree->loop) {
    for (uint32_t i = 0; i < start_index; ++i) {
      ui_leaf* current = &tree->raw[i];
      if (current->selectable) {
        tree->cursor_id    = (uint32_t)current->uid;
        tree->cursor_index = i;
        return cursor->uid;
      }
    }
  }

  return 0;
}

/* Return 0 if unable to find a valid target (all uid's are non-zero)*/
uint32_t ui_tree_prev(ui_tree* tree) {
  if (!tree || tree->count < 2) {
    ASTERA_FUNC_DBG("no or empty tree passed.\n");
    return 0;
  }

  /* check for unset cursor */
  if (tree->cursor_index == 0) {
    for (uint32_t i = 0; i < tree->count; ++i) {
      if (tree->raw[i].selectable) {
        tree->cursor_index = i;
        tree->cursor_id    = tree->raw[i].uid;

        return tree->cursor_id;
      }
    }
  }

  ui_leaf* cursor = &tree->raw[tree->cursor_index];
  if (!cursor) {
    ASTERA_FUNC_DBG("Unable to get cursor from array.\n");
    return 0;
  }

  if (cursor->element.type == UI_DROPDOWN) {
    ui_dropdown* dropdown = (ui_dropdown*)cursor->element.data;
    if (dropdown->showing) {
      ui_dropdown_prev(dropdown);
      return dropdown->id;
    }
  }

  int32_t closest = -1;
  for (uint32_t i = 0; i < tree->cursor_index; ++i) {
    if (tree->raw[i].selectable) {
      closest = i;
    }
  }

  if (closest != -1) {
    tree->cursor_id    = tree->raw[closest].uid;
    tree->cursor_index = closest;
    return tree->cursor_id;
  }

  if (tree->loop) {
    for (uint32_t i = tree->cursor_index + 1; i < tree->count; ++i) {
      if (tree->raw[i].selectable) {
        closest = i;
      }
    }

    if (closest) {
      tree->cursor_id    = tree->raw[closest].uid;
      tree->cursor_index = (uint16_t)closest;
      return tree->cursor_id;
    }
  }

  return 0;
}


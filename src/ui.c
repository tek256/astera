#include "ui.h"

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg/nanovg_gl.h>

#include <nanovg/nanovg.c>

#if !defined(UI_DEFAULT_ATTRIB_CAPACITY)
#define UI_DEFAULT_ATTRIB_CAPACITY 24
#endif

static ui_ctx g_ui_ctx;

static NVGcolor ui_vec3_color(vec3 v) {
  return nvgRGBA(v[0] * 255.f, v[1] * 255.f, v[2] * 255.f, 255);
}

static NVGcolor ui_vec4_color(vec4 v) {
  return nvgRGBA(v[0] * 255.f, v[1] * 255.f, v[2] * 255.f, v[3] * 255.f);
}

uint32_t ui_element_get_uid(ui_element element) {
  if (element.data) {
    return *((uint32_t *)element.data);
  } else {
    return 0;
  }
}

uint8_t ui_init(vec2 size, float pixel_scale, int use_mouse) {
  vec2_dup(g_ui_ctx.size, size);
  g_ui_ctx.nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

  g_ui_ctx.pixel_scale = pixel_scale;
  g_ui_ctx.use_mouse = use_mouse;

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

void ui_frame_start() {
  nvgBeginFrame(g_ui_ctx.nvg, g_ui_ctx.size[0], g_ui_ctx.size[1],
                g_ui_ctx.pixel_scale);
}

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

void ui_attrib_set(ui_attrib attrib, void *value, ui_attrib_type type) {
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

  void *new_data = malloc(size);
  memcpy(new_data, value, size);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = type;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set3f(ui_attrib attrib, float x, float y, float z) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC3) {
      vec3 *data_value = ((vec3 *)map.attribs[i].data);
      *data_value[0] = x;
      *data_value[1] = y;
      *data_value[2] = z;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void *new_data = malloc(sizeof(vec3));
  vec3 *data_value = ((vec3 *)map.attribs[map.count].data);
  *data_value[0] = x;
  *data_value[1] = y;
  *data_value[2] = z;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC3;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set3fv(ui_attrib attrib, vec3 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC3) {
      vec3 *data_value = ((vec3 *)map.attribs[i].data);
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

  void *new_data = malloc(sizeof(vec3));
  vec3 *data_value = (vec3 *)map.attribs[map.count].data;
  vec3_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC3;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set4f(ui_attrib attrib, float x, float y, float z, float w) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC4) {
      vec4 *data_value = ((vec4 *)map.attribs[i].data);
      *data_value[0] = x;
      *data_value[1] = y;
      *data_value[2] = z;
      *data_value[3] = w;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void *new_data = malloc(sizeof(vec4));
  vec4 *data_value = ((vec4 *)map.attribs[map.count].data);
  *data_value[0] = x;
  *data_value[1] = y;
  *data_value[2] = z;
  *data_value[3] = w;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC4;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set4fv(ui_attrib attrib, vec4 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC4) {
      vec4 *data_value = (vec4 *)map.attribs[i].data;
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

  void *new_data = malloc(sizeof(vec4));
  vec4 *data_value = (vec4 *)new_data;
  vec4_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC4;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set2f(ui_attrib attrib, float x, float y) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC2) {
      vec2 *data_value = (vec2 *)map.attribs[i].data;
      *data_value[0] = x;
      *data_value[1] = y;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void *new_data = malloc(sizeof(vec2));
  vec2 *data_value = (vec2 *)new_data;
  *data_value[0] = x;
  *data_value[1] = y;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC2;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_set2fv(ui_attrib attrib, vec2 value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_VEC2) {
      vec2 *data_value = (vec2 *)map.attribs[i].data;
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

  void *new_data = malloc(sizeof(vec2));
  vec2 *data_value = (vec2 *)new_data;
  vec2_dup(*data_value, value);

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_VEC2;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_setf(ui_attrib attrib, float value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_FLOAT) {
      float *data_value = (float *)map.attribs[i].data;
      *data_value = value;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void *new_data = malloc(sizeof(float));
  float *data_value = (float *)new_data;
  *data_value = value;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_FLOAT;
  map.attribs[map.count].data = new_data;
  map.count++;
}

void ui_attrib_seti(ui_attrib attrib, int32_t value) {
  ui_attrib_map map = g_ui_ctx.attribs;

  for (int i = 0; i < map.count; ++i) {
    if (map.attribs[i].attrib == attrib && map.attribs[i].type == UI_INT) {
      int32_t *data_value = (int32_t *)map.attribs[i].data;
      *data_value = value;
      return;
    }
  }

  if (map.count == map.capacity - 1) {
    return;
  }

  if (map.attribs[map.count].data) {
    free(map.attribs[map.count].data);
  }

  void *new_data = malloc(sizeof(int32_t));
  int32_t *data_value = (int32_t *)new_data;
  *data_value = value;

  map.attribs[map.count].attrib = attrib;
  map.attribs[map.count].type = UI_INT;
  map.attribs[map.count].data = new_data;
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
    if (map.attribs[i].attrib = attrib) {
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
  return *((int *)storage.data);
}

float ui_attrib_getf(ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_FLOAT) {
    return -1.f;
  }
  return *((float *)storage.data);
}

void ui_attrib_get2f(vec2 dst, ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC2) {
    dst[0] = -1.f;
    return;
  }
  vec2_dup(dst, *(vec2 *)storage.data);
}

void ui_attrib_get3f(vec3 dst, ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC3) {
    dst[0] = -1.f;
    return;
  }
  vec3_dup(dst, *(vec3 *)storage.data);
}

void ui_attrib_get4f(vec4 dst, ui_attrib attrib) {
  ui_attrib_storage storage = ui_attrib_get(attrib);
  if (storage.type != UI_VEC4) {
    dst[0] = -1.f;
    return;
  }
  vec4_dup(dst, *(vec4 *)storage.data);
}

ui_tree ui_tree_create(ui_element *root, uint16_t capacity) {
  ui_tree tree;
  ui_leaf *raw = malloc(sizeof(ui_leaf) * capacity);

  raw[0] = (ui_leaf){ui_element_get_uid(*root), 0, 0};

  tree.start = &raw[0];
  tree.end = &raw[0];
  tree.cursor = &raw[0];

  tree.count = 1;
  tree.capacity = capacity;
  return tree;
}

static char ui_text_buffer[128];
static int ui_text_buffer_cap = 128;

void ui_test_text(ui_font font) {
  nvgBeginPath(g_ui_ctx.nvg);
  nvgFontSize(g_ui_ctx.nvg, 32.f);
  nvgFontFaceId(g_ui_ctx.nvg, font);
  nvgTextAlign(g_ui_ctx.nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

  nvgFontBlur(g_ui_ctx.nvg, 15.f);
  nvgFillColor(g_ui_ctx.nvg, nvgRGBA(255, 0, 0, 255));
  nvgText(g_ui_ctx.nvg, 100.f, 50.f, "I wonder how this will, look.", 0);

  nvgFontBlur(g_ui_ctx.nvg, 0);
  nvgFillColor(g_ui_ctx.nvg, nvgRGBA(255, 255, 255, 255));
  nvgText(g_ui_ctx.nvg, 100.f, 50.f, "I wonder how this will, look.", 0);
}

void ui_text_draw(ui_text *text) {
  int string_length;
  char *string_data;

  if (text->use_reveal) {
    if (text->reveal_length == 0) {
      return;
    } else if (text->reveal_length == text->text_length) {
      string_length = text->text_length;
      string_data = text->text;
    } else {
      string_length = text->reveal_length;
      if (string_length < ui_text_buffer_cap - 1) {
        strncpy(ui_text_buffer, text->text, string_length);
      } else {
        strncpy(ui_text_buffer, text->text, ui_text_buffer_cap - 1);
        string_length = ui_text_buffer_cap - 1;
      }
      ui_text_buffer[string_length] = '\0';
      string_data = ui_text_buffer;
    }
  }

  nvgBeginPath(g_ui_ctx.nvg);
  nvgFontSize(g_ui_ctx.nvg, text->size);
  nvgFontFaceId(g_ui_ctx.nvg, text->font);
  nvgTextAlign(g_ui_ctx.nvg, text->alignment);

  if (text->use_spacing) {
    nvgTextLetterSpacing(g_ui_ctx.nvg, text->spacing);
    nvgTextLineHeight(g_ui_ctx.nvg, text->line_height);
  }

  vec2 draw_pos;
  vec2_dup(draw_pos, text->position);

  if (text->use_box) {
    if (text->use_shadow) {
      nvgFontBlur(g_ui_ctx.nvg, text->shadow_size);
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->shadow));
      nvgTextBox(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->bounds[0],
                 string_data, 0);

      nvgFontBlur(g_ui_ctx.nvg, 0.f);
    }

    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->color));
    nvgTextBox(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], text->bounds[0],
               string_data, 0);

  } else {
    if (text->use_shadow) {
      nvgFontBlur(g_ui_ctx.nvg, text->shadow_size);
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->shadow));
      nvgText(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], string_data, 0);

      nvgFontBlur(g_ui_ctx.nvg, 0.f);
    }

    nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(text->color));
    nvgText(g_ui_ctx.nvg, draw_pos[0], draw_pos[1], string_data, 0);
  }
}

void ui_box_draw(ui_box *box, int8_t focused) {
  nvgBeginPath(g_ui_ctx.nvg);
  if (box->use_border) {
    if (box->border_radius != 0.f) {
      nvgRoundedRect(g_ui_ctx.nvg, box->position[0], box->position[1],
                     box->size[0], box->size[1], box->border_radius);
    } else {
      nvgRect(g_ui_ctx.nvg, box->position[0], box->position[1], box->size[0],
              box->size[1]);
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

    nvgRect(g_ui_ctx.nvg, box->position[0], box->position[1], box->size[0],
            box->size[1]);
    nvgFill(g_ui_ctx.nvg);
  }
}

void ui_button_draw(ui_button *button, int8_t focused) {
  nvgBeginPath(g_ui_ctx.nvg);
  if (button->use_border) {
    if (button->border_radius != 0.f) {
      nvgRoundedRect(g_ui_ctx.nvg, button->position[0], button->position[1],
                     button->size[0], button->size[1], button->border_radius);
    } else {
      nvgRect(g_ui_ctx.nvg, button->position[0], button->position[1],
              button->size[0], button->size[1]);
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

    nvgFill(g_ui_ctx.nvg);
  } else {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_bg));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->bg));
    }
    nvgRect(g_ui_ctx.nvg, button->position[0], button->position[1],
            button->size[0], button->size[1]);
    nvgFill(g_ui_ctx.nvg);
  }

  if (strlen(button->text)) {
    if (focused) {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->color));
    } else {
      nvgFillColor(g_ui_ctx.nvg, ui_vec4_color(button->hover_color));
    }

    nvgFontSize(g_ui_ctx.nvg, button->font_size);
    nvgFontFaceId(g_ui_ctx.nvg, button->font);
    nvgTextAlign(g_ui_ctx.nvg, button->text_alignment);
    nvgTextLineHeight(g_ui_ctx.nvg, button->size[0]);

    nvgText(g_ui_ctx.nvg, button->position[0], button->position[1],
            button->text, 0);
  }
}

void ui_dropdown_draw(ui_dropdown *dropdown, int8_t focused) {}
void ui_line_draw(ui_line *line) {}
void ui_option_draw(ui_option *option, int8_t focused) {}

void ui_tree_destroy(ui_tree *tree) {
  ui_leaf *cursor = tree->start;
  while (cursor) {
    if (cursor->element.data) {
      switch (cursor->element.type) {
      case UI_IMAGE:
        ui_image_destroy((ui_img *)cursor->element.data);
        break;
      case UI_DROPDOWN:
        ui_dropdown_destroy((ui_dropdown *)cursor->element.data);
        break;
      case UI_TEXT:
        ui_text_destroy((ui_text *)cursor->element.data);
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

uint32_t ui_tree_add(ui_tree *tree, void *data, ui_element_type type) {
  if (!data || !tree) {
    return 0;
  }

  // Tree Resizing
  if (tree->count == tree->capacity - 1) {
    return 0;
  }

  ui_leaf *leaf = &tree->raw[tree->count];

  // Non Zero IDs
  ++g_ui_ctx.global_id;
  uint32_t uid = g_ui_ctx.global_id;
  leaf->uid = uid;

  // Pass UID to the element data
  uint32_t *data_id = (uint32_t *)data;
  *data_id = uid;

  leaf->element.data = data;
  leaf->element.type = type;

  leaf->next = NULL;
  leaf->prev = tree->end;
  tree->end = leaf;

  ++tree->count;
  return uid;
}

uint32_t ui_tree_get_cursor_id(ui_tree *tree) { return tree->cursor->uid; }

void ui_tree_draw(ui_tree tree) {
  ui_leaf *cursor = tree.start;
  while (cursor) {
    int8_t focused = tree.cursor == cursor;
    if (cursor->element.data) {
      ui_element element = cursor->element;
      switch (element.type) {
      case UI_TEXT:
        ui_text_draw((ui_text *)element.data);
        break;
      case UI_BOX:
        ui_box_draw((ui_box *)element.data, focused);
        break;
      case UI_BUTTON:
        ui_button_draw((ui_button *)element.data, focused);
        break;
      case UI_LINE:
        ui_line_draw((ui_line *)element.data);
        break;
      case UI_DROPDOWN:
        ui_dropdown_draw((ui_dropdown *)element.data, focused);
        break;
      case UI_OPTION:
        ui_option_draw((ui_option *)element.data, focused);
        break;
      }
    }

    cursor = cursor->next;
  }
}

uint16_t ui_select_next(ui_tree *tree) {
  if (tree->cursor->next) {
    tree->cursor = tree->cursor->next;
  } else {
    tree->cursor = tree->start;
  }

  return tree->cursor->uid;
}

ui_text ui_text_create(vec2 pos, char *string, float font_size, ui_font font_id,
                       int alignment) {
  ui_text text;

  vec2_dup(text.position, pos);
  text.text = string;
  text.size = font_size;
  text.font = font_id;
  text.alignment = alignment;

  vec4 color, shadow;
  ui_attrib_get4f(color, UI_TEXT_COLOR);
  ui_attrib_get4f(shadow, UI_TEXT_SHADOW);
  float shadow_size = ui_attrib_getf(UI_TEXT_SHADOW_SIZE);
  float spacing = ui_attrib_getf(UI_TEXT_SPACING);
  float line_height = ui_attrib_getf(UI_TEXT_LINE_HEIGHT);

  if (color[0] > 0.f) {
    vec4_dup(text.color, color);
  } else {
    text.color[0] = 1.f;
    text.color[1] = 1.f;
    text.color[2] = 1.f;
  }

  if (spacing > 0.f) {
    text.spacing = spacing;
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
    text.use_shadow = 1;
  }

  /*  uint32_t id;
   ui_font font;

   char *text;
   int text_length, reveal_length;
   int alignment;

   vec2 position, bounds;
   vec3 color, shadow;

   float size, spacing, line_height, shadow_size;

   int use_shadow : 1;
   int use_spacing : 1;
   int use_reveal : 1;
   int use_box : 1;
   *
   */

  vec2_clear(text.bounds);

  text.use_box = 0;
  text.use_reveal = 0;
  text.use_shadow = 0;
  text.use_spacing = 0;

  return text;
}

ui_button ui_button_create(vec2 pos, vec2 size, char *text, ui_font font_id);
ui_line ui_line_create(vec2 start, vec2 end, float thickness);
ui_dropdown ui_dropdown_create(vec2 pos, vec2 size, char **options,
                               int option_count);
ui_option ui_option_create(vec2 pos, vec2 size, int state);
ui_box ui_box_create(vec2 pos, vec2 size);

ui_img ui_image_create(asset_t *image_data, ui_img_flags flags, vec2 pos,
                       vec2 size) {
  int32_t image_handle = nvgCreateImageMem(
      g_ui_ctx.nvg, flags, image_data->data, image_data->data_length);
  ui_img img;
  img.handle = image_handle;
  vec2_dup(img.position, pos);
  vec2_dup(img.size, size);
  return img;
}

ui_font ui_font_get(const char *font_name) {
  // Find the font via NanoVG
  return nvgFindFont(g_ui_ctx.nvg, font_name);
}

ui_font ui_font_create(asset_t *asset, const char *name) {
  int font_id =
      nvgCreateFontMem(g_ui_ctx.nvg, name, asset->data, asset->data_length, 0);
  return font_id;
}

void ui_tree_remove_id(ui_tree *tree, uint32_t id) {
  ui_leaf *current = tree->start;
  while (current) {
    if (current->uid == id) {
      ui_leaf *prev = current->prev;
      ui_leaf *next = current->next;

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

void ui_image_destroy(ui_img *img) {
  nvgDeleteImage(g_ui_ctx.nvg, img->handle);
}

void ui_button_destroy(ui_button *button) { free(button->text); }

void ui_text_destroy(ui_text *text) { free(text->text); }

void ui_dropdown_destroy(ui_dropdown *dropdown) {}

void ui_text_bounds(ui_text *text, vec4 bounds) {
  if (text->use_box) {
    nvgTextBoxBounds(g_ui_ctx.nvg, text->position[0], text->position[1],
                     text->bounds[0], text->text, 0, bounds);
  } else {
    nvgTextBounds(g_ui_ctx.nvg, text->position[0], text->position[1],
                  text->text, 0, bounds);
  }
}


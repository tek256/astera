#ifndef UI_H
#define UI_H

#include <misc/linmath.h>

#include "sys.h"

typedef struct {
  mat4x4 model;
  vec4 bounds;
  vec4 tex_bounds;
} ui_char;

typedef struct {
  ui_char *chars;
  int char_cap;
  int char_count;

  char *str;
  int str_len;

  int visible;
  float max_width;
  float max_height;

  mat4x4 model;

  unsigned int vao, vbo, vto, vboi;

  time_s time;
} ui_text;

void ui_text_update(ui_text *text);
void ui_text_destroy(ui_text *text);
ui_text ui_text_create(char *str);
void ui_text_draw(ui_text *text);

#endif

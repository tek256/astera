// TODO: Clipping / Scissor effects

#ifndef ASTERA_UI_HEADER
#define ASTERA_UI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <linmath.h>
#include <stdint.h>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <astera/export.h>

#define UI_IS_ALIGN(value, offset) ((value) & (1 << (offset)))
#define UI_IS_TYPE(value, type) (((value & (type)) == type)

typedef enum {
  UI_ALIGN_LEFT   = 1 << 0,
  UI_ALIGN_CENTER = 1 << 1,
  UI_ALIGN_RIGHT  = 1 << 2,

  UI_ALIGN_TOP      = 1 << 3,
  UI_ALIGN_MIDDLE   = 1 << 4,
  UI_ALIGN_BOTTOM   = 1 << 5,
  UI_ALIGN_BASELINE = 1 << 6,
} ui_align;

typedef uint32_t ui_font;

typedef struct {
  uint32_t id;
  ui_font  font;

  char *text, *reveal;
  int   text_length;
  int   align;

  vec2 position, bounds;
  vec4 color, shadow;

  float size, spacing, line_height, shadow_size;

  int use_shadow : 1;
  int use_spacing : 1;
  int use_reveal : 1;
  int use_box : 1;
} ui_text;

typedef struct {
  uint32_t id;
  char*    text;
  int32_t  align;
  ui_font  font;
  float    font_size;

  vec2 position, size;

  vec4  bg, hover_bg;
  vec4  color, hover_color;
  vec4  border_color, hover_border_color;
  float border_size, border_radius;

  int use_border : 1;
} ui_button;

typedef struct {
  uint32_t id;
  vec2     position, size;

  vec4  bg, hover_bg;
  vec4  border_color, hover_border_color;
  float border_size, border_radius;

  int use_border : 1;
} ui_box;

typedef struct {
  uint32_t     id;
  const char** options;
  uint16_t     option_count, option_display, option_capacity;
  uint16_t     start; // the start index for scrolling
  uint16_t     top_scroll_pad,
      bottom_scroll_pad; // the points in which scrolling will start
  uint16_t selected, cursor, mouse_cursor;

  ui_font font;
  float   font_size;
  int     align;

  vec2 position, size;
  vec4 bg, hover_bg;
  vec4 color, hover_color;
  vec4 border_color, hover_border_color;

  vec4 select_bg, select_color;
  vec4 hover_select_bg, hover_select_color;

  float border_size, border_radius;

  int use_border : 1;
  int showing : 1;
  int has_change : 1;
} ui_dropdown;

typedef struct {
  uint32_t id;
  vec2     start, end;
  vec4     color;
  float    thickness;
} ui_line;

typedef struct {
  uint32_t id;

  int32_t img_handle;
  vec2    img_size, img_offset;

  char*   text;
  ui_font font;
  float   font_size;
  int     align;

  vec2 position, size;
  vec4 bg_color, hover_bg_color;
  vec4 color, hover_color;

  uint8_t state;
  int     bg : 1;
  int     use_text : 1;
  int     use_img : 1;
} ui_option;

typedef enum {
  IMG_GENERATE_MIPMAPS = 1 << 0,
  IMG_REPEATX          = 1 << 1,
  IMG_REPEATY          = 1 << 2,
  IMG_FLIPY            = 1 << 3,
  IMG_PREMUL           = 1 << 4,
  IMG_NEAREST          = 1 << 5,
} ui_img_flags;

typedef struct {
  uint32_t id;
  vec2     position, size;
  int32_t  handle;
} ui_img;

typedef enum {
  UI_TEXT_ALIGN,
  UI_TEXT_COLOR,
  UI_TEXT_SHADOW,
  UI_TEXT_SHADOW_SIZE,
  UI_TEXT_LINE_HEIGHT,
  UI_TEXT_SPACING,

  UI_BUTTON_FONT,
  UI_BUTTON_COLOR,
  UI_BUTTON_BG,
  UI_BUTTON_COLOR_HOVER,
  UI_BUTTON_BG_HOVER,

  UI_BUTTON_BORDER_RADIUS,
  UI_BUTTON_BORDER_COLOR,
  UI_BUTTON_BORDER_COLOR_HOVER,
  UI_BUTTON_BORDER_SIZE,

  UI_DROPDOWN_BORDER_RADIUS,
  UI_DROPDOWN_BORDER_SIZE,
  UI_DROPDOWN_BORDER_COLOR,
  UI_DROPDOWN_BORDER_COLOR_HOVER,

  UI_DROPDOWN_FONT_SIZE,
  UI_DROPDOWN_FONT,
  UI_DROPDOWN_COLOR,
  UI_DROPDOWN_COLOR_HOVER,
  UI_DROPDOWN_BG,
  UI_DROPDOWN_BG_HOVER,

  UI_DROPDOWN_SELECT_COLOR,
  UI_DROPDOWN_SELECT_BG,
  UI_DROPDOWN_SELECT_COLOR_HOVER,
  UI_DROPDOWN_SELECT_BG_HOVER,

  UI_OPTION_IMAGE,
  UI_OPTION_IMAGE_SIZE,
  UI_OPTION_FONT,
} ui_attrib;

typedef enum {
  UI_NONE = 0,
  UI_INT,
  UI_FLOAT,
  UI_VEC2,
  UI_VEC3,
  UI_VEC4
} ui_attrib_type;

typedef struct {
  ui_attrib      attrib;
  ui_attrib_type type;
  void*          data;
} ui_attrib_storage;

typedef struct {
  ui_attrib_storage* attribs;
  int                count, capacity;
} ui_attrib_map;

typedef enum {
  UI_TEXT = 0,
  UI_BOX,
  UI_BUTTON,
  UI_LINE,
  UI_DROPDOWN,
  UI_OPTION,
  UI_IMAGE
} ui_element_type;

typedef struct {
  void*           data;
  ui_element_type type;
} ui_element;

struct ui_leaf;
typedef struct ui_leaf {
  uint32_t        uid;
  int8_t          selectable;
  ui_element      element;
  struct ui_leaf *next, *prev;
  int32_t         event;
} ui_leaf;

typedef struct {
  ui_leaf *start, *end;
  ui_leaf *cursor, *mouse_hover, *raw;
  uint16_t capacity, count;
  int      loop : 1;
} ui_tree;

ASTERA_API uint8_t ui_init(vec2 size, float pixel_scale, int use_mouse);
ASTERA_API void    ui_update(vec2 mouse_pos);
ASTERA_API void    ui_destroy();

ASTERA_API void ui_get_color(vec4 val, const char* v);

// Convert Pixels to Screen Size (Context defined)
ASTERA_API void ui_px_to_scale(vec2 dst, vec2 px);
// Convert Screen Scale to Pixels
ASTERA_API void ui_scale_to_px(vec2 dst, vec2 scale);

// Function to calculate new scale from pixel size within 'screen' size
ASTERA_API void ui_px_from_scale(vec2 dst, vec2 px, vec2 screen);

// Check if value (int) contains 1 or 0 at offset of alignment (binary)
ASTERA_API int8_t ui_is_align(int value, ui_align alignment);
// Check if value contains bitflag of type
ASTERA_API int8_t ui_is_type(int value, int type);

// Start the NanoVG Frame
ASTERA_API void ui_frame_start();
// End the NanoVG Frame
ASTERA_API void ui_frame_end();

// Set Attributes
ASTERA_API void ui_attrib_set(ui_attrib attrib, void* value, ui_attrib_type type);
ASTERA_API void ui_attrib_set3f(ui_attrib attrib, float x, float y, float z);
ASTERA_API void ui_attrib_set3fv(ui_attrib attrib, vec3 value);
ASTERA_API void ui_attrib_set4f(ui_attrib attrib, float x, float y, float z, float w);
ASTERA_API void ui_attrib_set4fv(ui_attrib attrib, vec4 value);
ASTERA_API void ui_attrib_set2f(ui_attrib attrib, float x, float y);
ASTERA_API void ui_attrib_set2fv(ui_attrib attrib, vec2 value);
ASTERA_API void ui_attrib_setf(ui_attrib attrib, float value);
ASTERA_API void ui_attrib_seti(ui_attrib attrib, int32_t value);

ui_attrib_storage ui_attrib_get(ui_attrib attrib);
int               ui_attrib_geti(ui_attrib attrib);
float             ui_attrib_getf(ui_attrib attrib);
void              ui_attrib_get2f(ui_attrib attrib, vec2 dst);
void              ui_attrib_get3f(ui_attrib attrib, vec3 dst);
void              ui_attrib_get4f(ui_attrib attrib, vec4 dst);

int8_t ui_attrib_exists(ui_attrib attrib);

// Get the font ID by name
ui_font ui_font_get(const char* font_name);
// Create a font to use
ui_font ui_font_create(unsigned char* data, int data_length, const char* name);

// Create a UI Text field
ui_text ui_text_create(vec2 pos, char* string, float font_size, ui_font font_id,
                       int alignment);
// Create a UI Button
ui_button   ui_button_create(vec2 pos, vec2 size, char* text,
                             int32_t text_alignment, float font_size);
ui_line     ui_line_create(vec2 start, vec2 end, vec4 color, float thickness);
ui_dropdown ui_dropdown_create(vec2 pos, vec2 size, char** options,
                               int option_count);
ui_option   ui_option_create(const char* text, float font_size,
                             int32_t text_alignment, vec2 pos, vec2 size);
ui_box      ui_box_create(vec2 pos, vec2 size, vec4 color, vec4 hover_color);
ui_img ui_image_create(unsigned char* data, int data_len, ui_img_flags flags,
                       vec2 pos, vec2 size);

void ui_text_next(ui_text* text);
void ui_text_prev(ui_text* text);

uint16_t ui_dropdown_add_option(ui_dropdown* dropdown, const char* option);
int8_t   ui_dropdown_contains(ui_dropdown* dropdown, vec2 pos);
void     ui_dropdown_set_to_cursor(ui_dropdown* dropdown);
void     ui_dropdown_set(ui_dropdown* dropdown, uint16_t select);
void     ui_dropdown_next(ui_dropdown* dropdown);
void     ui_dropdown_prev(ui_dropdown* dropdown);
int8_t   ui_dropdown_has_change(ui_dropdown* dropdown);

void ui_image_destroy(ui_img* img);

void ui_text_bounds(ui_text* text, vec4 bounds);

void ui_text_draw(ui_text* text);
void ui_box_draw(ui_box* box, int8_t focused);
void ui_button_draw(ui_button* button, int8_t focused);
void ui_dropdown_draw(ui_dropdown* dropdown, int8_t focused);
void ui_line_draw(ui_line* line);
void ui_option_draw(ui_option* option, int8_t focused);
void ui_image_draw(ui_img* img);

void ui_im_text_draw(vec2 pos, float font_size, ui_font font, char* text);

float ui_text_max_size(ui_text text, vec2 bounds, int allow_reveal);
float ui_dropdown_max_font_size(ui_dropdown dropdown);

int16_t ui_element_contains(ui_element element, vec2 point);
int32_t ui_element_event(ui_tree* tree, uint32_t uid);

ui_tree  ui_tree_create(uint16_t capacity);
uint32_t ui_tree_check(ui_tree* tree);
void     ui_tree_destroy(ui_tree* tree);
uint32_t ui_tree_add(ui_tree* tree, void* data, ui_element_type type,
                     int8_t selectable);

void ui_tree_print(ui_tree* tree);

uint32_t ui_tree_get_cursor_id(ui_tree* tree);
int8_t   ui_tree_is_active(ui_tree* tree, uint32_t id);
uint32_t ui_tree_select(ui_tree* tree, int32_t event_type, int is_mouse);
uint32_t ui_tree_next(ui_tree* tree);
uint32_t ui_tree_prev(ui_tree* tree);

void ui_tree_draw(ui_tree tree);

#ifdef __cplusplus
}
#endif

#endif

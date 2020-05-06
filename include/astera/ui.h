/* USAGE:
 * For all the positions passed into ui should be normalized for screen space,
 * i.e 0 -> 1, you can use the functions ui_px_to_scale and inversely
 * ui_scale_to_px in order to translate between them. Scale is set within the
 * ui_ctx struct, you can modify & get a copy of it from both ui_ctx_scale_set
 * and ui_ctx_scale_get NOTE: Scale is the term used to denote normalized size
 */

// TODO: Clipping / Scissor effects
// TODO: Finish Dropdown Scrolling

#ifndef ASTERA_UI_HEADER
#define ASTERA_UI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/export.h>

#include <astera/linmath.h>
#include <stdint.h>

#define UI_IS_ALIGN(value, offset) ((value) & (1 << (offset)))
#define UI_IS_TYPE(value, type)    (((value & (type)) == type))

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
  vec4 bg, hover_bg;
  vec4 color, hover_color;

  uint8_t state;
  int     use_color : 1;
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

  vec4  border_color, hover_border_color;
  float border_size, border_radius;
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
  int8_t          selectable, priority;
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

// Adjust a scaled position by pixels
ASTERA_API void ui_scale_move_px(vec2 dst, vec2 scale, vec2 px);

// Adjust a pixel position by scale
ASTERA_API void ui_px_move_scale(vec2 dst, vec2 px, vec2 scale);

// Function to calculate new scale from pixel size within 'screen' size
ASTERA_API void ui_px_from_scale(vec2 dst, vec2 px, vec2 screen);

// Set the UI's screensize (px)
ASTERA_API void ui_ctx_scale_set(vec2 size_px);

// Get the UI's screensize (px)
ASTERA_API void ui_ctx_scale_get(vec2 dst_px);

// Check if value contains bitflag of type
ASTERA_API int8_t ui_is_type(int value, int type);

// Start the NanoVG Frame
ASTERA_API void ui_frame_start();
// End the NanoVG Frame
ASTERA_API void ui_frame_end();

ASTERA_API void ui_attrib_set(ui_attrib attrib, void* value,
                              ui_attrib_type type);
ASTERA_API void ui_attrib_set3f(ui_attrib attrib, float x, float y, float z);
ASTERA_API void ui_attrib_set3fv(ui_attrib attrib, vec3 value);
ASTERA_API void ui_attrib_set4f(ui_attrib attrib, float x, float y, float z,
                                float w);
ASTERA_API void ui_attrib_set4fv(ui_attrib attrib, vec4 value);
ASTERA_API void ui_attrib_set2f(ui_attrib attrib, float x, float y);
ASTERA_API void ui_attrib_set2fv(ui_attrib attrib, vec2 value);
ASTERA_API void ui_attrib_setf(ui_attrib attrib, float value);
ASTERA_API void ui_attrib_seti(ui_attrib attrib, int32_t value);

ASTERA_API ui_attrib_storage ui_attrib_get(ui_attrib attrib);
ASTERA_API int               ui_attrib_geti(ui_attrib attrib);
ASTERA_API float             ui_attrib_getf(ui_attrib attrib);
ASTERA_API void              ui_attrib_get2f(ui_attrib attrib, vec2 dst);
ASTERA_API void              ui_attrib_get3f(ui_attrib attrib, vec3 dst);
ASTERA_API void              ui_attrib_get4f(ui_attrib attrib, vec4 dst);

ASTERA_API int8_t ui_attrib_exists(ui_attrib attrib);

ASTERA_API ui_font ui_font_get(const char* font_name);
ASTERA_API ui_font ui_font_create(unsigned char* data, int data_length,
                                  const char* name);

ASTERA_API ui_text   ui_text_create(vec2 pos, char* string, float font_size,
                                    ui_font font_id, int alignment);
ASTERA_API ui_button ui_button_create(vec2 pos, vec2 size, char* text,
                                      int32_t text_alignment, float font_size);

ASTERA_API ui_line     ui_line_create(vec2 start, vec2 end, vec4 color,
                                      float thickness);
ASTERA_API ui_dropdown ui_dropdown_create(vec2 pos, vec2 size, char** options,
                                          int option_count);
ASTERA_API ui_option   ui_option_create(const char* text, float font_size,
                                        int32_t text_alignment, vec2 pos,
                                        vec2 size);
ASTERA_API ui_box      ui_box_create(vec2 pos, vec2 size, vec4 color,
                                     vec4 hover_color);
ASTERA_API ui_img      ui_image_create(unsigned char* data, int data_len,
                                       ui_img_flags flags, vec2 pos, vec2 size);

ASTERA_API void ui_dropdown_set_colors(ui_dropdown* dropdown, vec4 bg,
                                       vec4 hover_bg, vec4 fg, vec4 hover_fg,
                                       vec4 border_color,
                                       vec4 hover_border_color, vec4 select_bg,
                                       vec4 select_color, vec4 hover_select_bg,
                                       vec4 hover_select_color);
ASTERA_API void ui_box_set_colors(ui_box* box, vec4 bg, vec4 hover_bg,
                                  vec4 border_color, vec4 hover_border_color);
ASTERA_API void ui_text_set_colors(ui_text* text, vec4 color, vec4 shadow);
ASTERA_API void ui_button_set_colors(ui_button* button, vec4 bg, vec4 hover_bg,
                                     vec4 fg, vec4 hover_fg, vec4 border_color,
                                     vec4 hover_border_color);
ASTERA_API void ui_line_set_colors(ui_line* line,
                                   vec4     color); // well that's easy
ASTERA_API void ui_img_set_colors(ui_img* img, vec4 border_color,
                                  vec4 hover_border_color);

ASTERA_API void ui_option_set_colors(ui_option* option, vec4 bg, vec4 hover_bg,
                                     vec4 fg, vec4 hover_fg);

ASTERA_API void ui_img_set_border_radius(ui_img* img, float radius);
ASTERA_API void ui_button_set_border_radius(ui_button* button, float radius);
ASTERA_API void ui_box_set_border_radius(ui_box* box, float radius);
ASTERA_API void ui_dropdown_set_border_radius(ui_dropdown* dropdown,
                                              float        radius);

ASTERA_API void ui_text_next(ui_text* text);
ASTERA_API void ui_text_prev(ui_text* text);

ASTERA_API uint16_t ui_dropdown_add_option(ui_dropdown* dropdown,
                                           const char*  option);
ASTERA_API int8_t   ui_dropdown_contains(ui_dropdown* dropdown, vec2 pos);
ASTERA_API void     ui_dropdown_set_to_cursor(ui_dropdown* dropdown);
ASTERA_API void     ui_dropdown_set(ui_dropdown* dropdown, uint16_t select);
ASTERA_API void     ui_dropdown_next(ui_dropdown* dropdown);
ASTERA_API void     ui_dropdown_prev(ui_dropdown* dropdown);
ASTERA_API int8_t   ui_dropdown_has_change(ui_dropdown* dropdown);

ASTERA_API void ui_image_destroy(ui_img* img);

ASTERA_API void ui_text_bounds(ui_text* text, vec4 bounds);

ASTERA_API void ui_text_draw(ui_text* text);
ASTERA_API void ui_box_draw(ui_box* box, int8_t focused);
ASTERA_API void ui_button_draw(ui_button* button, int8_t focused);
ASTERA_API void ui_dropdown_draw(ui_dropdown* dropdown, int8_t focused);
ASTERA_API void ui_line_draw(ui_line* line);
ASTERA_API void ui_option_draw(ui_option* option, int8_t focused);
ASTERA_API void ui_image_draw(ui_img* img, int8_t focused);

ASTERA_API void ui_im_text_draw_aligned(vec2 pos, float font_size, ui_font font,
                                        int alignment, char* text);

ASTERA_API void ui_im_text_draw(vec2 pos, float font_size, ui_font font,
                                char* text);
ASTERA_API void ui_im_box_draw(vec2 pos, vec2 size, vec4 color);
ASTERA_API void ui_im_circle_draw(vec2 pos, float radius, vec4 color);

ASTERA_API float ui_text_max_size(ui_text text, vec2 bounds, int allow_reveal);
ASTERA_API float ui_dropdown_max_font_size(ui_dropdown dropdown);

ASTERA_API int16_t ui_element_contains(ui_element element, vec2 point);
ASTERA_API int32_t ui_element_event(ui_tree* tree, uint32_t uid);

ASTERA_API ui_element ui_element_get(void* data, int type);
ASTERA_API void       ui_element_center_to(ui_element element, vec2 point);

ASTERA_API ui_tree  ui_tree_create(uint16_t capacity);
ASTERA_API uint32_t ui_tree_check(ui_tree* tree);
ASTERA_API void     ui_tree_destroy(ui_tree* tree);
ASTERA_API uint32_t ui_tree_add(ui_tree* tree, void* data, ui_element_type type,
                                int8_t priority, int8_t selectable);

ASTERA_API void ui_tree_print(ui_tree* tree);

ASTERA_API uint32_t ui_tree_get_cursor_id(ui_tree* tree);
ASTERA_API int8_t   ui_tree_is_active(ui_tree* tree, uint32_t id);
ASTERA_API uint32_t ui_tree_select(ui_tree* tree, int32_t event_type,
                                   int is_mouse);

ASTERA_API uint32_t ui_tree_select_id(ui_tree* tree, uint32_t id,
                                      int32_t event_type);

ASTERA_API uint32_t ui_tree_next(ui_tree* tree);
ASTERA_API uint32_t ui_tree_prev(ui_tree* tree);

ASTERA_API void ui_tree_draw(ui_tree* tree);

#ifdef __cplusplus
}
#endif

#endif

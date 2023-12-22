/* USAGE:
 * For all the positions passed into ui should be normalized for screen space,
 * i.e 0 -> 1, you can use the functions ui_px_to_scale and inversely
 * ui_scale_to_px in order to translate between them. Scale is set within the
 * ui_ctx struct, you can modify & get a copy of it from both ui_ctx_scale_set
 * and ui_ctx_scale_get
 * NOTE: Scale is the term used to denote normalized size */

/* TODO:
 * - Refactor ui_tree/ctx functions into more sensible names/usages
 * - Button text shadow option
 */

#ifndef ASTERA_UI_HEADER
#define ASTERA_UI_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/linmath.h>
#include <stdint.h>

typedef enum {
  UI_ALIGN_LEFT     = 1 << 0,
  UI_ALIGN_MIDDLE_X = 1 << 1,
  UI_ALIGN_RIGHT    = 1 << 2,

  UI_ALIGN_TOP      = 1 << 3,
  UI_ALIGN_MIDDLE_Y = 1 << 4,
  UI_ALIGN_BOTTOM   = 1 << 5,

  UI_ALIGN_CENTER = 1 << 6
} ui_align;

typedef float ui_color[4];

typedef uint32_t ui_font;

typedef struct {
  uint32_t id;
  ui_font  font;

  char *text, *reveal;
  int   text_length;
  int   align;

  vec2     position, bounds;
  ui_color color, shadow;

  float size, spacing, line_height, shadow_size;

  uint8_t use_shadow, use_spacing, use_reveal, use_box;
} ui_text;

typedef struct {
  uint32_t id;
  char*    text;
  int32_t  align;
  ui_font  font;
  float    font_size;

  vec2 position, size;
  vec4 padding;

  ui_color bg, hover_bg;
  ui_color color, hover_color;
  ui_color border_color, hover_border_color;
  float    border_size, border_radius;
} ui_button;

typedef struct {
  /* id - the internal unique UI ID for this element */
  uint32_t id;
  /* position - the position of the box (in relative size)
   * size - the size of the box (in relative size) */
  vec2 position, size;

  /* bg - the background color
   * hover_bg - the background color when hovered (0 just defaults to bg) */
  ui_color bg, hover_bg;
  /* border_color - the color of the border if drawn
   * hover_border_color - the color of the border when hovered if drawn */
  ui_color border_color, hover_border_color;

  /* border_size - the size of the border in px (0 = not drawn)
   * border_radius - the rounding of the corners in px
   *                  (not affected by border_size) */
  float border_size, border_radius;
} ui_box;

typedef struct {
  uint32_t id;
  /* options - the array of strings for option strings
   data - optional data pointer to other list */
  char** options;
  void*  data;

  /* option_count - the number of total options
   * option_capacity - the max number of options stored in `options` */
  uint16_t option_count, option_capacity;

  /* -- SCROLLING --
   * option_display - the number of options to display
   * start - the start index for the scroll offset */
  uint16_t option_display, start;
  /* top_scroll_pad - # of options to leave at the top when scrolling
   * bottom_scroll_pad - # of options to leave at the bottom when scrolling */
  uint16_t top_scroll_pad, bottom_scroll_pad;

  /* selected - current selected option
   * cursor - the current hovered option
   * mouse_cursor - the current mouse hovered option */
  uint16_t selected, cursor, mouse_cursor;

  ui_font font;
  float   font_size;
  int     align;

  vec2     position, size;
  ui_color bg, hover_bg;
  ui_color color, hover_color;
  ui_color border_color, hover_border_color;

  ui_color select_bg, select_color;
  ui_color hover_select_bg, hover_select_color;

  float border_size, border_radius;

  /* has_change - if there has been a change
   * use_mouse - if to use the mouse_cursor over cursor */
  uint8_t showing, has_change, use_mouse;
} ui_dropdown;

typedef struct {
  uint32_t id;
  vec2     start, end;
  ui_color color;
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

  vec2     position, size;
  ui_color bg, hover_bg;
  ui_color color, hover_color;

  uint8_t state, use_color, use_text, use_img;
} ui_option;

typedef enum {
  UI_BUTT_CAP,
  UI_ROUND_CAP,
  UI_SQUARE_CAP,
  UI_BEVEL_CAP,
  UI_MITER_CAP
} ui_line_cap;

typedef struct {
  uint32_t id;
  vec2     position, size;
  float    fill_padding; /* padding amount between border & fill */
  ui_color active_bg, active_fg, active_border_color;
  ui_color bg, fg, border_color;

  float border_radius, border_size;
  float progress;

  int8_t vertical_fill;
  int8_t flip;
  int8_t active;
} ui_progress;

typedef struct {
  uint32_t id;
  vec2     position, size, button_size;
  float    fill_padding; /* padding amount between border & fill */
  ui_color active_bg, active_fg, active_border_color;
  ui_color bg, fg, border_color;
  ui_color button_color, active_button_color;
  ui_color button_border_color, active_button_border_color;

  float border_radius, border_size;
  float button_border_size, button_border_radius;
  float progress;

  int8_t vertical_fill;
  int8_t flip;
  int8_t active, holding; /* holding is if the button is being dragged */
  int8_t button_fill, button_hover;
  int8_t button_circle;
  int8_t auto_hide_button;
  int8_t always_hide_button;

  float holding_progress;

  float   value, min_value, max_value;
  int16_t steps, has_change;
} ui_slider;

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

  ui_color border_color, hover_border_color;
  float    border_size, border_radius;
} ui_img;

typedef enum {
  UI_DEFAULT_FONT, /* Global fallback font */

  UI_TEXT_FONT,
  UI_TEXT_FONT_SIZE,
  UI_TEXT_ALIGN,
  UI_TEXT_COLOR,
  UI_TEXT_SHADOW,
  UI_TEXT_SHADOW_SIZE,
  UI_TEXT_LINE_HEIGHT,
  UI_TEXT_SPACING,

  UI_BOX_SIZE,
  UI_BOX_BG,
  UI_BOX_BORDER_COLOR,
  UI_BOX_BG_HOVER,
  UI_BOX_BORDER_COLOR_HOVER,
  UI_BOX_BORDER_SIZE,
  UI_BOX_BORDER_RADIUS,

  UI_BUTTON_SIZE,
  UI_BUTTON_PADDING,
  UI_BUTTON_FONT,
  UI_BUTTON_FONT_SIZE,
  UI_BUTTON_TEXT_ALIGN,
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
  UI_DROPDOWN_SIZE,
  UI_DROPDOWN_ALIGN,
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

  UI_OPTION_SIZE,
  UI_OPTION_IMAGE,
  UI_OPTION_IMAGE_SIZE,
  UI_OPTION_IMAGE_OFFSET,
  UI_OPTION_FONT,
  UI_OPTION_FONT_SIZE,
  UI_OPTION_TEXT_ALIGN,
  UI_OPTION_BG,
  UI_OPTION_BG_HOVER,
  UI_OPTION_COLOR,
  UI_OPTION_COLOR_HOVER,

  UI_LINE_THICKNESS,
  UI_LINE_COLOR,

  UI_PROGRESS_SIZE,
  UI_PROGRESS_FILL_PADDING,
  UI_PROGRESS_BG,
  UI_PROGRESS_FG,
  UI_PROGRESS_BORDER_COLOR,
  UI_PROGRESS_ACTIVE_BG,
  UI_PROGRESS_ACTIVE_FG,
  UI_PROGRESS_ACTIVE_BORDER_COLOR,
  UI_PROGRESS_BORDER_RADIUS,
  UI_PROGRESS_BORDER_SIZE,
  UI_PROGRESS_VERTICAL_FILL,

  UI_SLIDER_SIZE,
  UI_SLIDER_FILL_PADDING,
  UI_SLIDER_BG,
  UI_SLIDER_FG,
  UI_SLIDER_BORDER_COLOR,
  UI_SLIDER_ACTIVE_BG,
  UI_SLIDER_ACTIVE_FG,
  UI_SLIDER_ACTIVE_BORDER_COLOR,
  UI_SLIDER_BORDER_RADIUS,
  UI_SLIDER_BORDER_SIZE,
  UI_SLIDER_VERTICAL_FILL,
  UI_SLIDER_BUTTON_SIZE,
  UI_SLIDER_BUTTON_CIRCLE,
  UI_SLIDER_BUTTON_COLOR,
  UI_SLIDER_BUTTON_ACTIVE_COLOR,
  UI_SLIDER_BUTTON_BORDER_COLOR,
  UI_SLIDER_BUTTON_ACTIVE_BORDER_COLOR,
  UI_SLIDER_BUTTON_BORDER_SIZE,
  UI_SLIDER_BUTTON_BORDER_RADIUS,
  UI_SLIDER_FLIP,
  UI_SLIDER_AUTO_HIDE_BUTTON,
  UI_SLIDER_ALWAYS_HIDE_BUTTON,

  UI_ATTRIB_LAST,
} ui_attrib;

typedef enum {
  UI_NONE = 0,
  UI_INT,
  UI_FLOAT,
  UI_VEC2,
  UI_VEC3,
  UI_VEC4,
  UI_COLOR
} ui_attrib_type;

typedef struct {
  ui_attrib      attrib;
  ui_attrib_type type;
  union {
    vec2     v2;
    vec3     v3;
    vec4     v4;
    ui_color c;
    float    f;
    int      i;
  } data;
} ui_attrib_storage;

typedef struct {
  ui_attrib_storage* attribs;
  uint32_t           count, capacity;
  uint8_t            allow_resize;
} ui_attrib_map;

typedef enum {
  UI_TEXT = 0,
  UI_BOX,
  UI_BUTTON,
  UI_LINE,
  UI_DROPDOWN,
  UI_OPTION,
  UI_PROGRESS,
  UI_SLIDER,
  UI_IMG
} ui_element_type;

typedef struct {
  void*           data;
  ui_element_type type;
} ui_element;

typedef struct {
  uint32_t   uid;
  int32_t    index;
  int8_t     selectable, priority;
  ui_element element;
  int32_t    event;
  int16_t    layer;
} ui_leaf;

typedef struct {
  ui_leaf*  raw;
  ui_leaf** draw_order;
  uint16_t  capacity, count;

  int32_t mouse_hover_id, cursor_id;
  int32_t mouse_hover_index, cursor_index, selected_index;
  int     loop;
} ui_tree;

typedef struct ui_ctx ui_ctx;

/* Output the state of a tree */
void ui_debug_tree(ui_tree* tree);

/* Duplicate a color from a to dst
 * dst - the destination of the color
 * a - the color to duplicate */
void ui_color_dup(ui_color dst, ui_color const a);

/* Clear a color (set to black/0)
 * dst - the color to change */
void ui_color_clear(ui_color dst);

/* Check if a ui color is valud (non-negative)
 * a - the color to check
 * returns: valid = 1, invalid = 0 */
uint8_t ui_color_valid(ui_color const a);

/* Create a ui context
 * screen_size - resolution of the UI (in px)
 * pixel_scale - the scale of pixels to draw
 * use_mouse - if the UI should allow mouse input
 * antialias - if the UI should use antialiasing
 * attribs - if the UI should track and allow strorage of attrbiutes
 * returns: malloc'd pointer to ui context */
ui_ctx* ui_ctx_create(vec2 screen_size, float pixel_scale, uint8_t use_mouse,
                      uint8_t antialias, uint8_t attribs);

/* Update a ui context NOTE: Only needed if using mouse
 * ctx - context to update
 * mouse_pos - position of the mouse cursor */
void ui_ctx_update(ui_ctx* ctx, vec2 mouse_pos);

/* Resize the internal system to a screen size
 * ctx - context to resize
 * screen_size - the resolution to resize to in px */
void ui_ctx_resize(ui_ctx* ctx, vec2 screen_size);

/* Destroy a ui context
 * ctx - the context to destroy */
void ui_ctx_destroy(ui_ctx* ctx);

/* Square size to match same px count as scale width
 * ctx - context to square to
 * width - the width in scale to square to in height
 * returns: equal px amount height in scale units */
float ui_square_width(ui_ctx* ctx, float width);

/* Square size to match same px count as scale height
 * ctx - context to square to
 * height - the height in scale to square to in width
 * returns: equal px amount width in scale units */
float ui_square_height(ui_ctx* ctx, float height);

/* Convert width to scale
 * ctx - context to square to
 * width - the width in px to convert
 * returns: scale adjusted value */
float ui_px_scale_width(ui_ctx* ctx, float width);

/* Convert height to scale
 * ctx - context to square to
 * width - the height in px to convert
 * returns: scale adjusted value */
float ui_px_scale_height(ui_ctx* ctx, float height);

/* Set the attributes to fixed
 * ctx - the context to affect */
void ui_ctx_set_attribs_fixed(ui_ctx* ctx);

/* Set the max capacity of the attributes map
 * ctx - the context to use
 * capacity - the max number of attributes
 *            (will be clamped to max possible attributes, UI_ATTRIB_LAST) */
void ui_ctx_set_attribs_capacity(ui_ctx* ctx, uint32_t capacity);

/* Check if the UI Context is using mouse input
 * ctx - the context to check
 * returns: if mouse input is being used */
uint8_t ui_ctx_is_mouse(ui_ctx* ctx);

/* Check if the UI Context is using antialiasing
 * ctx - the context to check
 * returns: if antialiasing is being used */
uint8_t ui_ctx_is_antialias(ui_ctx* ctx);

/* Set if the UI context should use mouse input
 * ctx - the context to modify
 * mouse - if mouse input should be used (1 = yes, 0 = no) */
void ui_ctx_set_mouse(ui_ctx* ctx, uint8_t mouse);

/* Get a color from hex code (i.e #FFF = white)
 * NOTE: the `#` symbol will be skipped,
 *       works with both 3 & 6 length hex codes
 * val - the value to set (destination of the color)
 * v - the hex code string */
void ui_get_color(ui_color val, const char* v);

/* Add px offset to scaled position (convert & add)
 * ctx - the context to use for scale
 * dst - the destination
 * val - the value to add
 * px - the px value to offset by */
void ui_scale_offset_px(ui_ctx* ctx, vec2 dst, vec2 val, vec2 px);

/* Convert a pixel size / position to scale (based on the ctx size)
 * ctx - the context to use for scale
 * dst - the destiniation of the converstion
 * px - the pixel size to convert */
void ui_px_to_scale(ui_ctx* ctx, vec2 dst, vec2 px);

/* Convert Screen Scale to Pixels */
void ui_scale_to_px(ui_ctx* ctx, vec2 dst, vec2 scale);

/* Convert Screen Scale to Pixels (4f)
 * min x max, [min_x, min_y, max_x, max_y] */
void ui_scale_to_px4f(ui_ctx* ctx, vec4 dst, vec4 scale);

/* Adjust a scaled position by pixels */
void ui_scale_move_px(ui_ctx* ctx, vec2 dst, vec2 scale, vec2 px);

/* Adjust a pixel position by scale */
void ui_px_move_scale(ui_ctx* ctx, vec2 dst, vec2 px, vec2 scale);

/* Function to calculate new scale from pixel size within 'screen' size */
void ui_px_from_scale(vec2 dst, vec2 px, vec2 screen);

/* Set the UI's screensize (px) */
void ui_ctx_scale_set(ui_ctx* ctx, vec2 size_px);

/* Get the UI's screensize (px) */
void ui_ctx_scale_get(ui_ctx* ctx, vec2 dst_px);

/* Start the NanoVG Frame */
void ui_frame_start(ui_ctx* ctx);
/* End the NanoVG Frame */
void ui_frame_end(ui_ctx* ctx);

/* Set a vec2 attribute by type & size
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_set2f(ui_ctx* ctx, ui_attrib attrib, vec2 value);

/* Set a vec3 attribute by type & size
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_set3f(ui_ctx* ctx, ui_attrib attrib, vec3 value);

/* Set a vec4 attribute by type & size
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_set4f(ui_ctx* ctx, ui_attrib attrib, vec4 value);

/* Set a color attribute by type
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_setc(ui_ctx* ctx, ui_attrib attrib, ui_color color);

/* Set a float attribute by type & size
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_setf(ui_ctx* ctx, ui_attrib attrib, float value);

/* Set an integer attribute by type & size
 * ctx - the context to hold the attribute
 * attrib - the attribute
 * value - the value of the attribute */
void ui_attrib_seti(ui_ctx* ctx, ui_attrib attrib, int32_t value);

/* Get an integer attribute's value
 * ctx - the context containing the attribute map
 * attrib - the attribute type
 * returns: attribute value or -1 for fail */
int ui_attrib_geti(ui_ctx* ctx, ui_attrib attrib);

/* Get a float attribute's value
 * ctx - the context containing the attribute map
 * attrib - the attribute type
 * returns: attribute value or -1 for fail */
float ui_attrib_getf(ui_ctx* ctx, ui_attrib attrib);

/* Get vec4 attribute's value
 * ctx - the context containing the attribute map
 * attrib - the attribute type
 * dst - the destination to store the value
 * NOTE: will set all values in dst to -1 on fail */
void ui_attrib_get2f(ui_ctx* ctx, ui_attrib attrib, vec2 dst);

/* Get vec3 attribute's value
 * ctx - the context containing the attribute map
 * attrib - the attribute type
 * dst - the destination to store the value
 * NOTE: will set all values in dst to -1 on fail */
void ui_attrib_get3f(ui_ctx* ctx, ui_attrib attrib, vec3 dst);

/* Get vec4 attribute's value
 * ctx - the context containing the attribute map
 * attrib - the attribute type
 * dst - the destination to store the value
 * NOTE: will set all values in dst to -1 on fail */
void ui_attrib_get4f(ui_ctx* ctx, ui_attrib attrib, vec4 dst);

/* Check if an attribute has been set */
uint8_t ui_attrib_exists(ui_ctx* ctx, ui_attrib attrib);

/* Get a font based on its assigned name
 * ctx - the context to check
 * font_name - the name of the font given on creation
 * returns: the font ID */
ui_font ui_font_get(ui_ctx* ctx, const char* font_name);

/* Create a font for the UI system to use
 * ctx - the context to manage the font
 * data - the raw data of the font (file)
 * data_length - the length of the data
 * name - a name to assign the font
 * returns: the font ID */
ui_font ui_font_create(ui_ctx* ctx, unsigned char* data, int data_length,
                       const char* name);

/* Create a UI Text struct
 * ctx - the context for scale
 * pos - the position of the text on screen
 * string - the text to draw
 * font_size - the size to draw the text
 * font_id - the ID of the font (returned on font creation)
 * alignment - the alignment of the text (UI_ALIGN_XXX) */
ui_text ui_text_create(ui_ctx* ctx, vec2 pos, char* string, float font_size,
                       ui_font font_id, int alignment);

/* Create a UI Button struct
 * ctx - the context for screen scale
 * pos - the position of the button on screen (scale)
 * size - the size of the button on screen (scale)
 * text - the text of the button
 * text_alignment - the alignment of the text within the button
 * font_size - the size to draw the text inside the button */
ui_button ui_button_create(ui_ctx* ctx, vec2 pos, vec2 size, char* text,
                           int32_t text_alignment, float font_size);

/* Create a UI Progress struct
 * ctx - the context for screen scale
 * pos - the position of the progress on screen (scale)
 * size - the size of the progress bar on screen (scale)
 * progress - the percentage of progress (0 to 1)
 * vertical - if to draw the progress vertically */
ui_progress ui_progress_create(ui_ctx* ctx, vec2 pos, vec2 size,
                               float progress);
/* Create a UI Slider struct
 * ctx - the context for screen scale
 * pos - the position of the slider (scale)
 * size - the size of the slider (scale)
 * button_size - the size of the slider's button (scale)
 * round_button - if to draw the button as a circle
 * value - the progress / default value of the slider (anything between min&max)
 * min - the minimum value of the slider
 * max - the maximum value of the slider
 * steps - the number of steps to allow the slider to go thru (0 = none/free)*/
ui_slider ui_slider_create(ui_ctx* ctx, vec2 pos, vec2 size, vec2 button_size,
                           int round_button, float value, float min, float max,
                           int steps);

/* Create a UI Line struct
 * ctx - the context for screen scale
 * start - the start of the line (scale)
 * end - the end of the line (scale)
 * color - the color of the line
 * thickness - the thickness of the line (px) */
ui_line ui_line_create(ui_ctx* ctx, vec2 start, vec2 end, ui_color color,
                       float thickness);

/* Create a UI Dropdown struct
 * ctx - the context for screen scale
 * pos - the position of the dropdown on screen (scale)
 * size - the size of the dropdown on screen (scale)
 * options - an array of strings listing out the options for the dropdown
 * option_count - the number of strings in the options array */
ui_dropdown ui_dropdown_create(ui_ctx* ctx, vec2 pos, vec2 size, char** options,
                               uint16_t option_count);

/* Create a UI Option Struct
 * ctx - the context for screen scale
 * text - the text of the option (optional)
 * font_size - the size of the text
 * text_alignment - the alignment of the text
 * pos - the position of the option (scale)
 * size - the size of the option (scale) */
ui_option ui_option_create(ui_ctx* ctx, vec2 pos, vec2 size, const char* text,
                           float font_size, int32_t text_alignment);

/* Create a UI Box struct
 * ctx - the context for screen scale
 * pos - the position of the box on screen (scale)
 * size - the size of the box on screen */
ui_box ui_box_create(ui_ctx* ctx, vec2 pos, vec2 size);

/* Create a UI Image struct
 * ctx - the context for screen scale
 * data - the data of the image (file)
 * data_length - the length of the image
 * flags - the flags for the image
 * pos - the position of the image (scale)
 * size - the size of the image (scale) */
ui_img ui_img_create(ui_ctx* ctx, unsigned char* data, int data_len,
                     ui_img_flags flags, vec2 pos, vec2 size);

/* Set a dropdown's colors, if 0 is passed the argument is ignored */
void ui_dropdown_set_colors(ui_dropdown* dropdown, ui_color bg,
                            ui_color hover_bg, ui_color fg, ui_color hover_fg,
                            ui_color border_color, ui_color hover_border_color,
                            ui_color select_bg, ui_color select_color,
                            ui_color hover_select_bg,
                            ui_color hover_select_color);

/* Set a box's colors, if 0 is passed the argument is ignored */
void ui_box_set_colors(ui_box* box, ui_color bg, ui_color hover_bg,
                       ui_color border_color, ui_color hover_border_color);

/* Set a text's colors, if 0 is passed the argument is ignored */
void ui_text_set_colors(ui_text* text, ui_color color, ui_color shadow);

/* Set a buttons's colors, if 0 is passed the argument is ignored */
void ui_button_set_colors(ui_button* button, ui_color bg, ui_color hover_bg,
                          ui_color fg, ui_color hover_fg, ui_color border_color,
                          ui_color hover_border_color);

/* Set a progress's colors, if 0 is passed the argument is ignored */
void ui_progress_set_colors(ui_progress* progress, ui_color bg,
                            ui_color active_bg, ui_color fg, ui_color active_fg,
                            ui_color border_color,
                            ui_color active_border_color);

/* Set a slider's colors, if 0 is passed the argument is ignored */
void ui_slider_set_colors(ui_slider* slider, ui_color bg, ui_color active_bg,
                          ui_color fg, ui_color active_fg,
                          ui_color border_color, ui_color active_border_color,
                          ui_color button_color, ui_color active_button_color,
                          ui_color button_border_color,
                          ui_color active_button_border_color);

/* Set a line's colors, if 0 is passed the argument is ignored */
void ui_line_set_colors(ui_line* line, ui_color color);

/* Set a img's colors, if 0 is passed the argument is ignored */
void ui_img_set_colors(ui_img* img, ui_color border_color,
                       ui_color hover_border_color);

/* Set a option's colors, if 0 is passed the argument is ignored */
void ui_option_set_colors(ui_option* option, ui_color bg, ui_color hover_bg,
                          ui_color fg, ui_color hover_fg);

/* Set the border radius of a ui img (px) */
void ui_img_set_border_radius(ui_img* img, float radius);

/* Set the border radius of a ui button (px) */
void ui_button_set_border_radius(ui_button* button, float radius);

/* Set the border radius of a ui box (px) */
void ui_box_set_border_radius(ui_box* box, float radius);

/* Set the border radius of a ui dropdown (px) */
void ui_dropdown_set_border_radius(ui_dropdown* dropdown, float radius);

/* Advance to the next selectable element in a tree */
void ui_text_next(ui_text* text);

/* Advance to the previous selectable element in a tree */
void ui_text_prev(ui_text* text);

/* Add an option to the dropdown's option list
 * dropdown - the dropdown to add to
 * option - the string to add
 * returns: the index of the option */
uint16_t ui_dropdown_add_option(ui_dropdown* dropdown, const char* option);

/* Set the dropdown's index to the cursor
 * dropdown - the dropdown to affect */
void ui_dropdown_set_to_cursor(ui_dropdown* dropdown);

/* Set the dropdown's scroll padding preferences
 * dropdown - the dropdown to affect
 * top_scroll_pad - the number of options to leave at the top before scrolling
 * bottom_scroll_pad - the number of options to leave at the bottom before
 *                     scrolling */
void ui_dropdown_set_scroll(ui_dropdown* dropdown, uint8_t top_scroll_pad,
                            uint8_t bottom_scroll_pad);

/* Set the dropdown's index
 * dropdown - the dropdown to affect
 * select - the index to set to */
void ui_dropdown_set(ui_dropdown* dropdown, uint16_t select);

/* Scroll a dropdown down
 * dropdown - the dropdown to affect
 * amount - the number of indexes to move
 * returns: number of indexes moved */
uint16_t ui_dropdown_scroll_down(ui_dropdown* dropdown, uint16_t amount);

/* Scroll a dropdown up
 * dropdown - the dropdown to affect
 * amount - the number of indexes to move
 * returns: number of indexes moved */
uint16_t ui_dropdown_scroll_up(ui_dropdown* dropdown, uint16_t amount);

/* Move the cursor to the next index in a dropdown
 * dropdown - the dropdown to affect */
void ui_dropdown_next(ui_dropdown* dropdown);

/* Move the cursor to the previous index in a dropdown
 * dropdown - the dropdown to affect */
void ui_dropdown_prev(ui_dropdown* dropdown);

/* Check if the dropdown has a change & clear the flag
 * dropdown - the dropdown to check
 * returns: if there has been a change */
int8_t ui_dropdown_has_change(ui_dropdown* dropdown);

/* Move a slider to the next step value (doesn't work without steps) */
float ui_slider_next_step(ui_slider* slider);

/* Move a slider to the previous step value (doesn't work without steps) */
float ui_slider_prev_step(ui_slider* slider);

/* Destroy an image & its contents */
void ui_img_destroy(ui_ctx* ctx, ui_img* img);

/* Destroy a dropdown & its contents */
void ui_dropdown_destroy(ui_dropdown* dropdown);

void ui_button_destroy(ui_button* button);
void ui_option_destroy(ui_ctx* ctx, ui_option* option);
void ui_text_destroy(ui_text* text);

void ui_text_bounds(ui_ctx* ctx, ui_text* text, vec4 bounds);

void ui_text_draw(ui_ctx* ctx, ui_text* text);
void ui_box_draw(ui_ctx* ctx, ui_box* box, int8_t focused);
void ui_button_draw(ui_ctx* ctx, ui_button* button, int8_t focused);
void ui_progress_draw(ui_ctx* ctx, ui_progress* progress, int8_t focused);
void ui_slider_draw(ui_ctx* ctx, ui_slider* progress, int8_t focused);
void ui_dropdown_draw(ui_ctx* ctx, ui_dropdown* dropdown, int8_t focused);
void ui_line_draw(ui_ctx* ctx, ui_line* line);
void ui_option_draw(ui_ctx* ctx, ui_option* option, int8_t focused);
void ui_img_draw(ui_ctx* ctx, ui_img* img, int8_t focused);

/* -- Immediate mode drawing --  */
/* Draw aligned text in immediate mode (non struct based)
 * ctx - ui context to draw with
 * pos - the position (in screen scale 0-1.f)
 * font_size - the size of the text to draw (px)
 * font - the font to use
 * alignment - the alignment attributes to use
 * text - the string to draw */
void ui_im_text_draw_aligned(ui_ctx* ctx, vec2 pos, float font_size,
                             ui_font font, int alignment, char* text);

/* Draw text in immediate mode (non struct based)
 * ctx - ui context to draw with
 * pos - the position of the text (in screen scale 0-1.f)
 * font - the font to use
 * text - the string to draw */
void ui_im_text_draw(ui_ctx* ctx, vec2 pos, float font_size, ui_font font,
                     char* text);

/* Draw a box in immediate mode (non struct based)
 * ctx - ui context to draw with
 * pos - the position of the box (in screen scale (0-1.f))
 * size - the size of the box (in screen scale 0-1.f)
 * color - the color of the box */
void ui_im_box_draw(ui_ctx* ctx, vec2 pos, vec2 size, ui_color color);

/* Draw a circle in immediate mode (non struct based)
 * ctx - ui context to draw with
 * pos - position of the circle (in screen scale (0-1.f))
 * radius - the radius of the cirlce in px
 * thickness - thickness of the circle in px
 * color - color of the circle to draw */
void ui_im_circle_draw(ui_ctx* ctx, vec2 pos, float radius, float thickness,
                       ui_color color);

/* Draw a line in immediate mode (non struct based)
 * ctx - ui context to draw with
 * start - start position to draw the line (in screen scale (0-1.f))
 * end - end position to draw the line (in screen scale (0-1.f))
 * thickness - thickness of the line to draw in px
 * color - color of the line to draw*/
void ui_im_line_draw(ui_ctx* ctx, vec2 start, vec2 end, float thickness,
                     ui_color color);

/* Draw an image in immediate mode
 * ctx - ui context to draw with
 * img - image to draw (has to be loaded beforehand)
 * pos - position to draw the image (in screen scale (0-1.f))
 * size - the size of the image (in screen scale (0-1.f))*/
void ui_im_img_draw(ui_ctx* ctx, ui_img img, vec2 pos, vec2 size);

/* Get the max font size for a specific bounding box (px)
 * ctx - context to check against
 * text - the text object to use
 * bounds - the bounds of the text box (px)
 * allow_reveal - calculate based on revealed test */
float ui_text_max_size(ui_ctx* ctx, ui_text text, vec2 bounds,
                       int allow_reveal);
float ui_dropdown_max_font_size(ui_ctx* ctx, ui_dropdown dropdown);

int16_t ui_element_contains(ui_ctx* ctx, ui_element element, vec2 point);

/* Check a UI Tree for any events
 * tree - the tree to check
 * uid - the uid/element to check for
 * returns: the event type / value */
int32_t ui_tree_check_event(ui_tree* tree, uint32_t uid);

/* Reset state/cursors of a tree
 * tree - the tree to reset*/
void ui_tree_reset(ui_tree* tree);

/* Create an element structure from a UI type */
ui_element ui_element_get(void* data, int type);

/* Center an element to a position (scale) */
void ui_element_center_to(ui_element element, vec2 point);

/* Create a ui tree array
 * capacity - the amount of elements to hold
 * returns: formatted tree struct  */
ui_tree ui_tree_create(uint16_t capacity);

/* Check a tree for an event (using mouse)
 * NOTE: This function is not needed if you're not supporting mouse
 * ctx - the context to use for mouse position
 * tree - the tree to check
 * returns: the ID of the element under the mouse */
uint32_t ui_tree_check(ui_ctx* ctx, ui_tree* tree);

/* Destroy all the buffers & elements in a tree */
void ui_tree_destroy(ui_ctx* ctx, ui_tree* tree);

/* Add an element to a UI Tree
 * ctx - the context being used
 * tree - the tree to add to
 * void - the pointer to the element
 * type - the type of the element
 * priority - the priority of the element for input interaction (mouse)
 * selectable - if the element is selectable by mouse / cursor
 * layer - the layer to draw the element on (draw order) */
uint32_t ui_tree_add(ui_ctx* ctx, ui_tree* tree, void* data,
                     ui_element_type type, int8_t priority, int8_t selectable,
                     int16_t layer);

/* Set the cursor of a tree to an element ID */
void ui_tree_set_cursor_to(ui_tree* tree, uint32_t id);

/* Print the contents of a tree using ASTERA_DBG */
void ui_tree_print(ui_tree* tree);

/* Get the element ID of the tree's cursor */
uint32_t ui_tree_get_cursor_id(ui_tree* tree);

/* Check if a tree is hovering an element */
int8_t ui_tree_is_active(ui_ctx* ctx, ui_tree* tree, uint32_t id);

/* Send a select command to the tree
 * ctx - the context to use for scale
 * tree - the tree to check / use
 * event_type - the arbitrary type of event (up to user)
 * is_mouse - if it's a mouse event / should go to mouse hover vs input hover */
uint32_t ui_tree_select(ui_ctx* ctx, ui_tree* tree, int32_t event_type,
                        int is_mouse);

/* Select an element within a tree by ID */
uint32_t ui_tree_select_id(ui_tree* tree, uint32_t id, int32_t event_type);

/* Advance to the next selectable element in a tree */
uint32_t ui_tree_next(ui_tree* tree);

/* Advance to the previous selectable element in a tree */
uint32_t ui_tree_prev(ui_tree* tree);

/* Scroll whatever hovered element down
 * tree - the tree to affect
 * amount - the amount to scroll
 * is_mouse - if to use the mouse cursor
 * returns: affected element ID, 0 if none */
uint32_t ui_tree_scroll_down(ui_tree* tree, uint16_t amount, uint8_t is_mouse);

/* Scroll whatever hovered element up
 * tree - the tree to affect
 * amount - the amount to scroll
 * is_mouse - if to use the mouse cursor
 * returns: affected element ID, 0 if none */
uint32_t ui_tree_scroll_up(ui_tree* tree, uint16_t amount, uint8_t is_mouse);

/* Draw all the elements within a tree */
void ui_tree_draw(ui_ctx* ctx, ui_tree* tree);

#ifdef __cplusplus
}
#endif

#endif

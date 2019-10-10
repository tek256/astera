#ifndef UI_H
#define UI_H

#include <linmath.h>
#include "config.h"
#include "platform.h"
#include "render.h"

#define UI_BUFFER_SIZE 4096

typedef struct {
	u16 x, y;
	u16 width, height;
	u16 layer;
} u_bounds_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	const char* text;
} u_button_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	const char* text;
} u_text_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	int state;
} u_check_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	int max, value, step;
} u_slider_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	r_sheet sheet;
	u16 sub_img;
	vec2 min, max;
} u_img_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	vec3 color;
} u_box_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	u16 option_count, selected;
	u16 active, rows;
	const char** options;
} u_dropdown_t;

typedef struct {
	u_bounds_t bounds;
	u16 uid;
	u16 count;

	u_img_t* imgs;		
	u16 img_count, img_cap;

	u_box_t* boxes;
	u16 box_count, box_cap;

	u_dropdown_t* dropdowns;
	u16 drop_count, drop_cap;

	u_text_t* texts;
	u16 text_count, text_cap;

	u_button_t* buttons;
	u16 button_count, button_cap;
	
	u_check_t* checks;
	u16 check_count, check_cap;	

	u_slider_t* sliders;
	u16 slider_count, slider_cap;
} u_window_t;

typedef struct {
	u_window_t* windows;
	u16 count, capacity;
	u32 element_count;
	u16 selected_window;

	u16 selected_element;

	u32 texture;

	u32 quad;
	u32 quad_count;

	vec2 tex_size;

	vec3 colors[UI_BUFFER_SIZE];
	vec4 texcs[UI_BUFFER_SIZE]; //0,1 = sub_size, 2 = tex_id
	vec4 bounds[UI_BUFFER_SIZE]; //0,1 = size, 1,2 = offset 

	r_font* font;
	vec2 text_tex_size;
	u32 text_count;
	vec3 text_colors[UI_BUFFER_SIZE];
	vec4 text_texcoords[UI_BUFFER_SIZE];
	vec4 text_bounds[UI_BUFFER_SIZE];
} u_context_t;

typedef struct {
	vec3 window_bg;
	vec3 button_focus_bg;
	vec3 button_focus_fg;
	vec3 button_bg;
	vec3 button_fg;
	vec3 text_fg;
	vec3 slider_bg;
	vec3 slider_fg;
	vec3 slider_focus_bg;
	vec3 slider_focus_fg;
	vec3 dropdown_bg;
	vec3 dropdown_fg;
	vec3 dropdown_focus_bg;
	vec3 dropdown_focus_fg;	
	vec3 check_bg;
	vec3 check_fg;
	vec3 check_focus_bg;
	r_font default_font;
} u_colors_t;

static u_context_t u_context;
static u_colors_t u_colors;

void u_render();

u16 u_init();

void u_set_texture(unsigned int texture);

void u_add_quad(u_bounds_t bounds, vec2 sub_size, unsigned int tex_id, vec3 color);

void u_add_text(u_bounds_t bounds, const char* text, vec3 color);
s16 u_check_buffer(void** buffer, u16 element_size, u16* count, u16* capacity, u16 grow);

void u_set_color(u_colors_t* colors, const char* str, const char* color);

u16 u_select_window(u_window_t* window);
u_window_t* u_get_selected_window();

u_window_t* u_window(u_context_t* context, u_bounds_t bounds);
u_button_t* u_button(u_bounds_t bounds, const char* text);
u_text_t* u_text(u_bounds_t bounds, const char* text);
u_check_t* u_check(u_bounds_t bounds, u16 state);
u_slider_t* u_slider(u_bounds_t bounds, int step, int max, int value);
u_dropdown_t* u_dropdown(u_bounds_t bounds, const char** options, u16 option_count, u16 selected_option);

u_img_t* u_img(u_bounds_t bounds, r_sheet sheet, unsigned int sub_tex);
u_box_t* u_box(u_bounds_t bounds, vec3 color);

#endif

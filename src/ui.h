#ifndef UI_H
#define UI_H

#include <linmath.h>
#include <nuklear.h>
#include "platform.h"

#define U_TEXT 1
#define U_TEXT_COLOR 2
#define U_OPTION 3
#define U_RADIO 4
#define U_CHECK 5
#define U_BUTTON 6
#define U_SLIDER 7
#define U_PROGRESS 8
#define U_EDIT_STR 9

#define U_ROW_CUSTOM 0
#define U_ROW_STANDARD 1
#define U_ROW_PADDING 2

#define MAX_VERT_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

enum text_align {
    TEXT_ALIGN_LEFT        = 0x01,
    TEXT_ALIGN_CENTERED    = 0x02,
    TEXT_ALIGN_RIGHT       = 0x04,
    TEXT_ALIGN_TOP         = 0x08,
    TEXT_ALIGN_MIDDLE      = 0x10,
    TEXT_ALIGN_BOTTOM      = 0x20
};

enum text_alignment {
    TEXT_LEFT        = TEXT_ALIGN_MIDDLE|TEXT_ALIGN_LEFT,
    TEXT_CENTERED    = TEXT_ALIGN_MIDDLE|TEXT_ALIGN_CENTERED,
    TEXT_RIGHT       = TEXT_ALIGN_MIDDLE|TEXT_ALIGN_RIGHT
};

typedef struct {
	struct nk_context* nk;	
} u_ctx;

typedef struct {
	u16 uid;
	u16 prev;
	u16 next;
} u_focus_link;

typedef struct {
	u_focus_link* start;
	u_focus_link* selected;
	u16 length;
	u16 loop;
} u_focus_group;

typedef struct {
	const char* str;
	u16 str_len;
	u16 flags;
} u_text_t;

typedef struct {
	const char* str;
	u16 str_len;
	vec3 color;
	u16 flags;
} u_text_color_t;

typedef struct {
	const char* text;
	u16 value;
} u_option_t;

typedef struct {
	const char* text;
	u32* value;
} u_radio_t;

typedef struct {
	const char* text;
	u16 value;
} u_check_t;

typedef struct {
	const char* text;
} u_button_t;

typedef struct {
	float min;
	float* value;
	float max;
	float step;
} u_slider_t;

typedef struct {
	u16* value;
	u16 end;
	u16 modifiable;
} u_progress_t;

typedef struct {
	char* str_buff;
	u16* length;
	u16 max_length;
	u16 flags;
} u_edit_str_t;

typedef struct {
	u16 uid;
	u16 column;
	u16 type;
	const char* name;
	union {
		u_text_t text;
		u_text_color_t text_color;
		u_option_t option;
		u_radio_t radio;
		u_check_t check;
		u_button_t button;
		u_slider_t slider;
		u_progress_t progress;
		//NOTE: implement if needed
		//u_edit_str_t edit_str;
	} data;
} u_element;

typedef struct {
	u16 columns;
	u16 standard;
	float height;
	u_element* elements;
	u16 element_count;
	float* ratios;
	u16 ratio_count;
} u_row_t;

typedef struct {
	u16 uid;
	char name[8];
	u_row_t* rows;
	u16 count;
	u_focus_group focus;	
} u_section;

typedef struct {
	u16 columns;
	float height;
	
	u_element* elements;
	u16 element_count;
	u16 element_capacity;

	float* ratios;
	u16 ratio_count;
	u16 ratio_capacity;
} u_row_con;

typedef struct {
	u16 uid;
	char name[8];
	u_row_t* rows;
	u16 row_count;
	u16 row_capacity;

	u_focus_link* focus_linkage;
	u16 focus_count;
	u16 focus_capacity;
} u_section_con;

//create the arrays
void u_init(struct nk_context* handle);

void u_con_start();
void u_con_end();

u16 u_check_add();

void u_set_style();

u16  u_start();
u16  u_end();

u16  u_section_start(const char* name);
u16  u_section_name(const char* name);
u16  u_section_end();

void u_draw_section(const char* name);

void u_focus_add(u16 uid, u16 prev, u16 next);
u16  u_focused();

u16  u_button(const char* name, const char* msg);
u16  u_window(const char* name, float x, float y, float w, float h);
u16  u_window_t(const char* name, const char* title, float x, float y, float w, float h);
u16  u_checkbox(const char* name, const char* label, int value);
u16  u_option(const char* name, const char* label, int value);
u16  u_radio(const char* name, const char* label, int* value);
void u_text(const char* text, u16 text_length, u16 flags);
void u_color_text(const char* text, u16 text_length, u16 flags, vec3 color);
u16  u_slider(const char* name, float min, float* value, float max, float step);
u16  u_progress(const char* name, u16* value, u16 end, u16 modifiable);
/* TODO implement if needed
 * void u_edit_str(char* str_buff, u16* length, u16 max_length, u16 flags);*/

void u_row(float height, int columns);
void u_row_padding(float height);

u16  u_row_add(u_element element);

void u_row_start(float height, int columns);
void u_row_push(float ratio);
void u_row_end();

void u_space(int columns);

u16  u_combo_start(const char* text, float width, float height);
u16  u_combo_label(const char* text, u16 flags);
void u_combo_end();

u16  u_width();
void u_push_event(u16 uid);
u16  u_has_event(u16 uid);
void u_update();

#endif

#ifndef r_H
#define r_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>

#include "platform.h"
#include "conf.h"

//max number of quads to draw at once
#define RENDER_BATCH_SIZE 128

//max length of a uniform string
#define SHADER_STR_SIZE 64

//number of animations to cache at max
#define RENDER_ANIM_CACHE 64

//number of shaders to store in map at max
#define RENDER_SHADER_CACHE 2

//number of uniform locations to store
#define RENDER_SHADER_VALUE_CACHE 32

//max length of an animation in frames
#define RENDER_ANIM_MAX_FRAMES 32

typedef struct {
	int allowed : 1;
	int scaled  : 1;
	unsigned char video_mode_count : 6; //0 - 63
} r_flags;

typedef struct {
	int width, height;
	int fullscreen, vsync, borderless;
	int refreshRate;
	char* title;
} r_window_info;

typedef struct {
	int width, height;
	int x, y;
	int fullscreen, vsync;
	int close_requested;
	int refreshRate;
	GLFWwindow* glfw;
} r_window;

typedef struct {
	vec3 pos;
	vec3 rot;
	mat4x4 view;
	mat4x4 proj;
	vec2 size;
	f32 fov;
	f32 near;
	f32 far;
} r_camera;

typedef u32 r_shader;

typedef struct {
	r_shader shader;

	mat4x4 models[RENDER_BATCH_SIZE];
	u32 tex_ids[RENDER_BATCH_SIZE];
	u32 flip_x[RENDER_BATCH_SIZE];
	u32 flip_y[RENDER_BATCH_SIZE];

	u16 count;
	u16 capacity;
} r_shader_batch;

typedef struct {
	r_shader shader;
	u32 values[RENDER_SHADER_VALUE_CACHE];
	const char* names[RENDER_SHADER_VALUE_CACHE];
	u16 count;
	u16 capacity;
} r_shader_cache;

typedef struct {
	r_shader shaders[RENDER_SHADER_CACHE];
	const char* names[RENDER_SHADER_CACHE];
	r_shader_cache caches[RENDER_SHADER_CACHE];
	r_shader_batch batches[RENDER_SHADER_CACHE];
	u32 count;
	u32 capacity;	
} r_shader_map;

/*
 *
static r_drawable drawables[RENDER_BATCH_SIZE];
static int drawable_count = 0;

static u32 drawable_uid = 1;


 */
typedef struct {
	u32 id;
	u32 width, height;
} r_tex;

typedef struct {
	u32 id;
	u32 width, height;
	u32 subwidth, subheight;
} r_sheet;

//animation states
#define R_ANIM_STOP  0x00
#define R_ANIM_PLAY  0x01
#define R_ANIM_PAUSE 0x10

typedef struct {
	u32* frames;
	u32  frame_count;
	u32  frame_rate;
	u32  uid;	
	u32  sheet_id;
} r_anim;

typedef struct {
	r_anim anims[RENDER_ANIM_CACHE];
	char*  names[RENDER_ANIM_CACHE];

	u16 count;
	u16 capacity;
	u16 uid;
} r_anim_cache;

typedef struct {
	u32* frames;
	u32 sheet_id;
	u32 frame_count;
	u32 frame_rate;
	u32 anim_id;

	f32 time;

	u16 frame;	
	unsigned char state, pstate;
	int loop : 1; 
} r_animv;

#ifndef EXCLUDE_CREATE
#define RENDER_SHEET_CACHE 32
typedef struct {
	const char* vertex_shaders[RENDER_SHADER_CACHE];
	const char* fragment_shaders[RENDER_SHADER_CACHE];
	r_shader shader_ids[RENDER_SHADER_CACHE];
	u32 shader_count;
	
	u32 anim_ids[RENDER_ANIM_CACHE];
	r_anim anims[RENDER_ANIM_CACHE];
	u32 anim_count;

	const char* sheet_paths[RENDER_SHEET_CACHE];
	u32 sheet_ids[RENDER_SHEET_CACHE];
	u32 sheet_subwidths[RENDER_SHEET_CACHE];
	u32 sheet_subheights[RENDER_SHEET_CACHE];
	u32 sheet_count;

	const char* tex_paths[RENDER_SHEET_CACHE];
	u32 tex_ids[RENDER_SHEET_CACHE];
	u32 tex_count;
} r_resource_map;
#endif

//#endif

typedef struct {
	r_animv anim;
	r_shader shader;
	vec2 size;
	vec2 position;
	mat4x4 model;

	u16 c_tex;
	unsigned char layer;
	int visible : 1;
	int change  : 1;
	u32 uid;
	u32 flip_x, flip_y;
} r_drawable;

typedef struct {
	r_drawable drawables[RENDER_BATCH_SIZE];
	u32 count;
	u32 capacity;
	u32 uid;
} r_drawable_cache; 

// === UI ===
#define UI_NONE  0
#define UI_HOVER 1
#define UI_CLICK 2
#define UI_RELEASE 3

typedef struct {
	vec2 pos, size;
	vec3 color, hover_color, click_color;
	int state;
} r_box;

typedef struct {
	vec2 pos, size;
	unsigned int sheet;
	unsigned int tex, hover_tex, click_tex;
	vec2 tex_size, sub_size;
	int state;
} r_tex_box;

typedef struct {
	vec2 pos;
	float radius;
	vec3 color, hover_color, click_color;
	int state;
} r_circle;

typedef struct {
	vec2 pos, size;
	vec3 color, hover_color, click_color;

	unsigned int sheet;
	vec2 tex_size, sub_size;
	unsigned int button, button_hover, button_click;
	float radius;
	float value;
	int button_state;

	int state;
} r_slider;

typedef struct {
	//size is calculated in text
	vec2 pos, size;
	float font_size;
	const char* msg;

	//allow for typing effect
	int msg_start, msg_end;

	int state;
} r_text;

static r_window g_window;
static r_camera g_camera;

static r_anim_cache g_anim_cache;
static r_drawable_cache g_drawable_cache;
static r_shader_map g_shader_map;

#ifndef EXCLUDE_CREATE
r_resource_map* r_get_map();
#endif

int r_init_quad();

int  r_init(c_conf conf);
void r_exit();
void r_update(long delta);

r_tex       r_get_tex(const char* fp);
void        r_bind_tex(u32 tex);

r_sheet r_get_sheet(const char* fp, u32 subwidth, u32 subheight);

void r_update_batch(r_shader shader, r_sheet* sheet);
//NOTE: count required since we're using instanced rendering
void r_draw_call(r_shader shader, r_sheet* sheet);

void r_destroy_anims();
void r_destroy_quad(u32 vao);

static GLuint r_get_sub_shader(const char* filePath, int type);
r_shader      r_get_shader(const char* vert, const char* frag);
r_shader      r_get_shadern(const char* name);
void          r_bind_shader(r_shader shader);
void          r_destroy_shader(r_shader shader);
int           r_get_uniform_loc(r_shader shader, const char* name);

void          r_map_shader(r_shader shader, const char* name);
void 		  r_cache_uniform(r_shader shader, const char* uniform, u32 location);
void          r_clear_cache(r_shader shader);
void          r_remove_from_cache(r_shader shader);

int  r_hex_number(char v);
int  r_hex_multi(char* v, int len);
void  r_get_color(vec3 val, char* v);

int r_is_anim_cache();
int r_is_shader_cache();

r_tex_box r_get_tex_box(vec2 pos, vec2 size, r_sheet sheet, u32 tex, u32 hover_tex, u32 click_tex);
r_box r_get_box(vec2 pos, vec2 size, vec3 color, vec3 hover_color, vec3 click_color);

/*	vec2 pos, size;
	vec3 color, hover_color, click_color;

	unsigned int sheet;
	vec2 tex_size, sub_size;
	unsigned int button, button_hover, button_click;
	float radius;
	float value;
	int button_state;

	int state;*/
r_slider r_get_slider(vec2 pos, vec2 size, float radius, float value, r_sheet sheet, vec3 color, vec3 hover_color, vec3 click_color, u32 tex, u32 hover_tex, u32 click_tex);

int r_draw_box(r_box* box);
int r_draw_tex(r_tex_box* box);
int r_draw_slider(r_slider* slider);
void r_draw_text(r_text* text);

int  r_draw_button(vec2 pos, vec2 size, vec3 color, vec3 clicked_color);

r_anim  r_get_anim(r_sheet sheet, u32* frames, int frame_count, int frame_rate);
r_animv r_v_anim(r_anim* anim); 

r_anim* r_get_anim_n(const char* name);
r_anim* r_get_anim_i(u32 uid);
void    r_cache_anim(r_anim anim, const char* name);

void r_anim_p(r_animv* anim); //anim play
void r_anim_s(r_animv* anim); //anim stop
void r_anim_h(r_animv* anim); //anim halt

r_drawable* r_get_drawable(r_anim* anim, r_shader shader, vec2 size, vec2 pos);
r_drawable* r_get_drawablei(u32 uid);
void r_drawable_set_anim(r_drawable* drawable, r_anim* anim);
void	   r_update_drawable(r_drawable* drawable, long delta);

void r_create_camera(r_camera* camera, vec2 size, vec2 position);
void r_update_camera();
void r_move_cam(f32 x, f32 y);

void r_set_uniformf(r_shader shader, const char* name, f32 value);
void r_set_uniformi(r_shader shader, const char* name, int value);
void r_set_v4(r_shader shader, const char* name, vec4 value);
void r_set_v3(r_shader shader, const char* name, vec3 value);
void r_set_v2(r_shader shader, const char* name, vec2 value);
void r_set_m4(r_shader shader, const char* name, mat4x4 value);

void r_set_m4x(r_shader shader, u32 count, const char* name, mat4x4* values);
void r_set_ix(r_shader shader, u32 count, const char* name, int* values);
void r_set_fx(r_shader shader, u32 count, const char* name, f32* values);
void r_set_v2x(r_shader shader, u32 count, const char* name, vec2* values);
void r_set_v3x(r_shader shader, u32 count, const char* name, vec3* values);
void r_set_v4x(r_shader shader, u32 count, const char* name, vec4* values);

void r_set_uniformfi(int loc, f32 val);
void r_set_uniformii(int loc, int val);
void r_set_v4i(int loc, vec4 val);
void r_set_v3i(int loc, vec3 val);
void r_set_v2i(int loc, vec2 val);
void r_set_m4i(int loc, mat4x4 val);

static void r_create_modes();
static int  r_window_info_valid(r_window_info info);
static const GLFWvidmode* r_find_closest_mode(r_window_info info);
static const GLFWvidmode* r_find_best_mode();

static void glfw_err_cb(int error, const char* msg);
static void glfw_window_pos_cb(GLFWwindow* window, int x, int y);
static void glfw_window_size_cb(GLFWwindow* window, int w, int h);
static void glfw_window_close_cb(GLFWwindow* window);
static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y);
static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action, int mods);
static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy);
static void glfw_joy_cb(int joystick, int action);
static void glfw_char_cb(GLFWwindow* window, u32 c);

int  r_create_window(r_window_info info);
void r_destroy_window();
void r_request_close();
void r_save_prefs();

void r_center_window();
void r_set_window_pos(int x, int y);

int r_should_close();

void r_swap_buffers();
void r_clear_window();
#endif

#ifndef r_H
#define r_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "geom.h"

#define R_ANIM_STOP 0
#define R_ANIM_PLAY 1
#define R_ANIM_PAUSE 2

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

#define RENDER_ENABLE_SHADER_CACHE
#define RENDER_ENABLE_ANIM_CACHE

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
	v3 pos, rot;
	m4 view, proj;
	v2 size;
	float fov;
	float near;
	float far;
} r_camera;

typedef unsigned int r_shader;

#if defined(RENDER_ENABLE_SHADER_CACHE)
typedef struct {
	r_shader shader;
	unsigned int values[RENDER_SHADER_VALUE_CACHE];
	const char* names[RENDER_SHADER_VALUE_CACHE];
	unsigned short count;
	unsigned short capacity;
} r_shader_cache;

typedef struct {
	r_shader shaders[RENDER_SHADER_CACHE];
	const char* names[RENDER_SHADER_CACHE];
	r_shader_cache caches[RENDER_SHADER_CACHE];
	unsigned int count;
	unsigned int capacity;	
} r_shader_map;
#endif

typedef struct {
	unsigned int id;
	unsigned int width, height;
} r_tex;

typedef struct {
	unsigned int id;
	unsigned int width, height;
	unsigned int subwidth, subheight;
} r_sheet;

typedef struct {
	r_sheet sheet;
	unsigned short frame_count;
	unsigned short* frames;
	unsigned char frame_rate;	
} r_anim;

#if defined(RENDER_ENABLE_ANIM_CACHE)
typedef struct {
	r_anim anims[RENDER_ANIM_CACHE];
	char*  names[RENDER_ANIM_CACHE];
	unsigned short count;
	unsigned short capacity;
} r_anim_cache;
#endif

typedef struct {
	r_anim anim;
	
	unsigned long time;
	unsigned short frame;	
	unsigned char state, pstate;
	int loop : 1; 
} r_animv;

typedef struct {
	r_animv anim;
	v2 size;
	v2 position;
	m4 model;

	unsigned char layer;
	int visible : 1;
	int change  : 1; //change in position / size
} r_drawable;

typedef struct {
	m4 models[RENDER_BATCH_SIZE];
	unsigned short model_count;

	unsigned short tex_ids[RENDER_BATCH_SIZE];
	unsigned short tex_id_count;

	unsigned char layers[RENDER_BATCH_SIZE];
	unsigned short layer_count;
} r_cache;

static r_window g_window;
static r_cache  g_r_cache;

#if defined(RENDER_ENABLE_ANIM_CACHE)
static r_anim_cache g_anim_cache;
#endif

#if defined(RENDER_ENABLE_SHADER_CACHE)
static r_shader_map g_shader_map;
#endif

static int r_init_quad();

int  r_init();
void r_exit();
void r_update(long delta);

r_tex       r_get_tex(const char* fp);
void        r_bind_tex(r_tex* tex);

r_sheet r_get_sheet(const char* fp, unsigned int subwidth, unsigned int subheight);

//NOTE: count required since we're using instanced rendering
void r_draw_call(r_shader shader, unsigned int count);

static GLuint r_get_sub_shader(const char* filePath, int type);
r_shader      r_get_shader(const char* vert, const char* frag);
void          r_bind_shader(r_shader shader);
void          r_destroy_shader(r_shader shader);
int           r_get_uniform_loc(r_shader shader, const char* name);

#if defined(RENDER_ENABLE_SHADER_CACHE)
void          r_map_shader(r_shader shader, const char* name);
void 		  r_cache_uniform(r_shader shader, const char* uniform, unsigned int location);
void          r_clear_cache(r_shader shader);
void          r_remove_from_cache(r_shader shader);
#endif

int  r_hex_number(char v);
int  r_hex_multi(char* v, int len);
v3   r_get_color(char* v);

int r_is_anim_cache();
int r_is_shader_cache();

r_anim  r_get_anim(r_sheet* sheet, int* frames, unsigned short frame_count, unsigned char frame_rate);
r_animv r_v_anim(r_anim* anim); 

#if defined(RENDER_ENABLE_ANIM_CACHE)
r_anim  r_get_anim_n(const char* name);
void    r_cache_anim(r_anim anim, const char* name);
#endif

void r_anim_p(r_animv* anim); //anim play
void r_anim_s(r_animv* anim); //anim stop
void r_anim_h(r_animv* anim); //anim halt

r_drawable r_get_drawable(r_anim* anim, v2 size, v2 pos);
void	   r_update_drawable(r_drawable* drawable);

r_camera r_create_camera(v2 size, v2 position);
void r_update_camera(r_camera* camera);

void r_set_uniformf(r_shader shader, const char* name, float value);
void r_set_uniformi(r_shader shader, const char* name, int value);
void r_set_v4(r_shader shader, const char* name, v4 value);
void r_set_v3(r_shader shader, const char* name, v3 value);
void r_set_v2(r_shader shader, const char* name, v2 value);
void r_set_quat(r_shader shader, const char* name, quat value);
void r_set_m4(r_shader shader, const char* name, m4 value);

void r_set_m4x(r_shader shader, unsigned int count, const char* name, m4* values);
void r_set_ix(r_shader shader, unsigned int count, const char* name, int* values);
void r_set_fx(r_shader shader, unsigned int count, const char* name, float* values);
void r_set_v2x(r_shader shader, unsigned int count, const char* name, v2* values);
void r_set_v3x(r_shader shader, unsigned int count, const char* name, v3* values);
void r_set_v4x(r_shader shader, unsigned int count, const char* name, v4* values);

void r_set_uniformfi(int loc, float val);
void r_set_uniformii(int loc, int val);
void r_set_v4i(int loc, v4 val);
void r_set_v3i(int loc, v3 val);
void r_set_v2i(int loc, v2 val);
void r_set_quati(int loc, quat val);
void r_set_m4i(int loc, m4 val);

static void r_create_modes();
static int  r_window_info_valid(r_window_info info);
static const GLFWvidmode* r_find_closest_mode(r_window_info info);
static const GLFWvidmode* r_find_best_mode();

static void r_window_resized();

static void glfw_err_cb(int error, const char* msg);
static void glfw_window_pos_cb(GLFWwindow* window, int x, int y);
static void glfw_window_size_cb(GLFWwindow* window, int w, int h);
static void glfw_window_close_cb(GLFWwindow* window);
static void glfw_key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfw_mouse_pos_cb(GLFWwindow* window, double x, double y);
static void glfw_mouse_button_cb(GLFWwindow* window, int button, int action, int mods);
static void glfw_scroll_cb(GLFWwindow* window, double dx, double dy);
static void glfw_joy_cb(int joystick, int action);
static void glfw_char_cb(GLFWwindow* window, unsigned int c);

int  r_create_window(r_window_info info);
void r_destroy_window();
void r_request_close();

void r_center_window();
void r_set_window_pos(int x, int y);

int r_should_close();

void r_swap_buffers();
void r_clear_window();
#endif

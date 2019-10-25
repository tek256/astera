#ifndef r_H
#define r_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>

#include "platform.h"
#include "config.h"
#include "asset.h"

#include <stb/stb_truetype.h>

typedef struct {
	int allowed;
	int scaled;
	int video_mode_count;
} r_flags;

typedef struct {
	int width, height;
	int fullscreen, vsync, borderless;
	int refreshRate;
	char* title;
	char* icon;
} r_window_info;

typedef struct {
	int width, height;
	int x, y;
	int fullscreen, vsync, borderless;
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
	u32 id;
	u32 width, height;
} r_tex;

typedef struct {
	u32 id;
	u32 width, height;
	u32 subwidth, subheight;
} r_sheet;

typedef struct {
	r_sheet sheet;
	u32 sub_id;
} r_subtex;

typedef struct {
	u32* frames;
	u32  frame_count;
	u32  frame_rate;
	u32  uid;	
	u32  sheet_id;
} r_anim;

typedef struct {
	r_anim anims[RENDER_ANIM_CACHE];
	const char* names[RENDER_ANIM_CACHE];

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

typedef struct {
	const char* name;
	int type;
	r_shader shader;

	void* data;

	unsigned int count;
	unsigned int capacity;
	unsigned int stride;
} r_uniform_array;

typedef struct {
	r_shader shader;

	r_uniform_array* uniform_arrays;
	u32 uniform_array_count;	

	r_sheet* sheet;	

	u16 count;
	u16 capacity;
} r_shader_batch;

typedef struct {
	const char* names[RENDER_SHADER_CACHE];
	r_shader_batch batches[RENDER_SHADER_CACHE];
	u32 count;
	u32 capacity;	
} r_shader_map;


#ifndef EXCLUDE_CREATE
#define RENDER_SHEET_CACHE 32
typedef struct {
	const char* vertex_shaders[RENDER_SHADER_CACHE];
	const char* fragment_shaders[RENDER_SHADER_CACHE];
	const char* shader_names[RENDER_SHADER_CACHE];
	r_shader shader_ids[RENDER_SHADER_CACHE];
	u32 shader_count;

	const char* anim_names[RENDER_ANIM_CACHE];	
	u32 anim_ids[RENDER_ANIM_CACHE];
	r_anim anims[RENDER_ANIM_CACHE];
	u32 anim_count;

	const char* sheet_paths[RENDER_SHEET_CACHE];
	const char* sheet_names[RENDER_SHEET_CACHE];
	u32 sheet_ids[RENDER_SHEET_CACHE];
	u32 sheet_subwidths[RENDER_SHEET_CACHE];
	u32 sheet_subheights[RENDER_SHEET_CACHE];
	u32 sheet_count;

	const char* tex_names[RENDER_SHEET_CACHE];
	const char* tex_paths[RENDER_SHEET_CACHE];
	u32 tex_ids[RENDER_SHEET_CACHE];
	u32 tex_count;
} r_resource_map;
#endif

typedef struct {
	vec2 position, size;

	int visible : 1;
	int change  : 1;
	int animated : 1;

	u32 uid;

	r_shader shader;
	union {
		r_animv anim;
		r_subtex tex;
	};

	mat4x4 model;
	u8 layer;
	u8 flip_x, flip_y;
} r_drawable;

typedef struct {
	vec2 position, velocity;
	float life;
} r_particle;

typedef struct {
	r_drawable drawables[RENDER_BATCH_SIZE];
	u32 count;
	u32 capacity;
	u32 uid;
} r_drawable_cache; 

static r_window g_window;
static r_camera g_camera;

static r_anim_cache g_anim_cache;
static r_drawable_cache g_drawable_cache;
static r_shader_map g_shader_map;

#ifndef EXCLUDE_CREATE
r_resource_map* r_get_map();
#endif

int  r_init(r_window_info info);
void r_exit();
void r_update(long delta);

r_tex r_tex_create(asset_t* asset);
void  r_tex_bind(u32 tex);

r_sheet r_sheet_create(asset_t* asset, u32 subwidth, u32 subheight);

void r_update_batch(r_shader shader, r_sheet* sheet);
void r_draw_call(r_shader shader);

r_shader      r_shader_create(asset_t* vert, asset_t* frag);
r_shader      r_get_shadern(const char* name);

void          r_shader_bind(r_shader shader);
void          r_shader_destroy(r_shader shader);
void 		  r_shader_setup_array(r_shader shader, const char* name, int stride, int capacity, int array);

r_uniform_array* r_shader_get_array(r_shader shader, const char* name);
r_shader_batch*  r_shader_get_batch(r_shader shader);

void		  r_shader_add_to_array(r_shader shader, const char* name, void* data, int count);
int 	  	  r_shader_clear_array(r_shader shader, const char* name);

void          r_shader_setup(r_shader shader, const char* name);
void          r_shader_clear_arrays(r_shader shader);
void          r_shader_destroy_batch(r_shader shader);

void       r_get_color(vec3 val, char* v);

r_anim  r_anim_create(r_sheet sheet, u32* frames, int frame_count, int frame_rate);
r_animv r_anim_v(r_anim* anim); 

r_anim* r_get_animn(const char* name);
r_anim* r_get_animi(u32 uid);
void    r_cache_anim(r_anim anim, const char* name);

void r_anim_p(r_animv* anim); //anim play
void r_anim_s(r_animv* anim); //anim stop
void r_anim_h(r_animv* anim); //anim halt

r_drawable* r_create_drawable(r_anim* anim, r_shader shader, vec2 size, vec2 pos);
r_drawable* r_get_drawable(u32 uid);
void        r_drawable_destroy(r_drawable* drawable);
void        r_drawable_set_anim(r_drawable* drawable, r_anim* anim);
void	    r_drawable_update(r_drawable* drawable, long delta);

void r_shader_set_array(r_shader shader, const char* name);
void r_shader_set_arrays(r_shader shader);

void r_cam_create(r_camera* camera, vec2 size, vec2 position);
void r_cam_update(void);
void r_cam_move(f32 x, f32 y);

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

void r_window_get_size(int* w, int* h);

//return length of the string
int r_get_videomode_str(const char* dst, int index);
void r_set_videomode(int index);
void r_select_mode(int index, int fullscreen, int vsync, int borderless);
int r_get_vidmode_count(void);

int r_allow_render(void);
int r_is_vsync(void);
int r_is_fullscreen(void);
int r_is_borderless(void);

int  r_window_create(r_window_info info);
void r_window_destroy(void);
void r_window_request_close(void);
int  r_window_set_icon(asset_t* asset);

void r_window_center(void);
void r_window_set_pos(int x, int y);

int r_window_should_close(void);

void r_window_swap_buffers(void);
void r_window_clear(void);
#endif

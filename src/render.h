#ifndef r_H
#define r_H

#include <GLFW/glfw3.h>

#include <misc/linmath.h>

#include <stdio.h>
#include <stdlib.h>

#include "asset.h"
#include "config.h"
#include "platform.h"

#include <misc/stb_truetype.h>

typedef struct {
  int allowed;
  int scaled;
  int video_mode_count;
} r_flags;

typedef struct {
  int width, height;
  int fullscreen, vsync, borderless;
  int refreshRate;
  char *title;
  char *icon;
} r_window_info;

typedef struct {
  int width, height;
  int x, y;
  int fullscreen, vsync, borderless;
  int close_requested;
  int refreshRate;
  GLFWwindow *glfw;
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
  u32 *frames;
  u32 frame;
  u32 frame_count;
  u32 frame_rate;
  u32 uid;
  r_sheet sheet;

  u8 pstate;
  u8 state;

  float time;

  int loop : 1;
  int use : 1;
} r_anim;

typedef struct {
  r_anim *anims;
  const char **names;
  u16 high;
  u16 count;
  u16 capacity;
} r_anim_map;

typedef struct {
  vec2 position, size;

  r_shader shader;

  union {
    r_anim anim;
    r_subtex tex;
  } render;

  u8 layer;
  int flip_x;
  int flip_y;
  int change : 1;
  int animated : 1;
  int visible : 1;

  int used : 1;

  vec4 color;

  mat4x4 model;
} r_sprite;

typedef struct {
  const char *name;
  int type;
  r_shader shader;

  u32 uid;

  union {
    mat4x4 *mat_ptr;
    float *float_ptr;
    int *int_ptr;
    vec2 *vec2_ptr;
    vec3 *vec3_ptr;
    vec4 *vec4_ptr;
  } data;

  unsigned int count;
  unsigned int capacity;
} r_uniform_array;

typedef struct {
  r_shader shader;
  int count;

  const char **names;
  u32 *uids;
  int *types;
  unsigned int *locations;
  unsigned int *capacities;

  int used : 1;
} r_uniform_map;

typedef struct {
  r_shader shader;
  r_sheet sheet;

  r_uniform_array *uniform_arrays;
  u32 uniform_array_count;

  int sprite_count, sprite_capacity;
  int used : 1;
} r_shader_batch;

typedef struct {
  r_shader *shaders;
  const char **names;
  r_uniform_map *uniform_maps;
  u32 count;
  u32 capacity;

  r_shader_batch *batches;
  u32 batch_capacity;
} r_shader_map;

// ------ KEYFRAMES ------

typedef enum { LINEAR = 0, EASE_IN, EASE_EASE, EASE_OUT } r_keyframe_curve;

typedef struct {
  float point;
  float value;
  r_keyframe_curve curve;
} r_keyframe;

typedef struct {
  r_keyframe *list;
  unsigned int count;
} r_keyframes;

typedef enum { POINT = 0, CIRCLE, BOX } r_particle_spawn;

typedef struct {
  union {
    r_anim anim;
    r_subtex tex;
    vec4 color;
  } render;

  float life;
  vec2 position, size;

  mat4x4 model;

  int animated : 1;
  int texture : 1;
  int colored : 1;
  int alive : 1;
} r_particle;

typedef struct {
  r_particle *list;
  unsigned int capacity, high;
  unsigned int max_emission;
  unsigned int emission_count;

  float particle_life, system_life;
  float spawn_rate;

  float time, spawn_time;

  vec2 position, size, velocity;
  vec2 particle_size;

  r_particle_spawn spawn_type;

  r_keyframes fade_frames;
  r_keyframes size_frames;

  union {
    r_subtex tex;
    r_anim anim;
    vec4 color;
  } render;

  int animated : 1;
  int texture : 1;
  int colored : 1;
} r_particles;

static r_window g_window;
static r_camera g_camera;

static r_anim_map g_anim_map;
static r_shader_map g_shader_map;

// I'll even out these dashes one day..
// ----------- CACHES --------------------

void r_init_anim_map(int size);
void r_init_shader_map(int size);
void r_init_batches(int size);

// ------------- UTILITY --------------------

void r_get_color(vec3 val, char *v);

// ------------- RENDER STATE -----------------

int r_init(r_window_info info);
void r_exit();
void r_update(long delta);
void r_end(); // End of frame to force any batches with sprites left in them
              // to draw

// ------------- TEXTURE FUNCTIONS --------------

r_tex r_tex_create(asset_t *asset);
void r_tex_bind(u32 tex);

r_sheet r_sheet_create(asset_t *asset, u32 subwidth, u32 subheight);

// -------------- PARTICLES FUNCTIONS ---------------

float r_keyframe_get_value(r_keyframes frames, float point);

void r_particles_init(r_particles *system, unsigned int particle_capacity);
void r_particles_update(r_particles *system, double delta);

// ----------------- BATCH FUNCTIONS -----------------

void r_sprite_draw(r_sprite draw);
void r_batch_draw(r_shader_batch *batch);

int r_batch_count(void);
int r_sprite_draw_count(void);
void r_batch_info(void);
void r_batch_clear(r_shader_batch *batch);

void r_shader_clear_array(r_uniform_array *array);
r_shader_batch *r_batch_create(r_shader shader, r_sheet sheet);

r_shader_batch *r_shader_get_batch(r_shader shader, r_sheet sheet);
void r_batch_set_arrays(r_shader_batch *batch);
void r_batch_destroy_all(r_shader shader);
void r_batch_destroy(r_shader shader, r_sheet sheet);

// ---------- ANIMATION FUNCTIONS ------------

r_anim r_anim_create(r_sheet sheet, u32 *frames, int frame_count,
                     int frame_rate);
void r_anim_destroy(int uid);
int r_anim_get_index(const char *name);
r_anim *r_anim_get(int uid);
int r_anim_cache(r_anim anim, const char *name);

// ---------- SPRITE FUNCTIONS -------------

r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size);

r_sheet r_sprite_get_sheet(r_sprite sprite);
int r_sprite_get_sheet_id(r_sprite sprite);

int r_sprite_get_tex_id(r_sprite sprite);
void r_sprite_set_anim(r_sprite *drawable, r_anim anim);
void r_sprite_set_tex(r_sprite *drawable, r_subtex tex);
void r_sprite_update(r_sprite *drawable, long delta);
void r_sprite_req_draw(r_sprite *drawable);

// ----------- CAMERA FUNCTIONS ------------

void r_cam_create(r_camera *camera, vec2 size, vec2 position);
void r_cam_update(void);
void r_cam_move(f32 x, f32 y);

// ---------- SHADER FUNCTIONS -------------
r_shader r_shader_create(asset_t *vert, asset_t *frag);
r_shader r_shader_get(const char *name);

void r_shader_bind(r_shader shader);
void r_shader_destroy(r_shader shader);

void r_shader_cache(r_shader shader, const char *name);

// --- SHADER UNIFORMS ---
void r_shader_uniform(r_shader shader, r_sheet sheet, const char *name,
                      void *data, int count);
void r_shader_uniformi(r_shader shader, r_sheet sheet, int uid, void *data,
                       int count);
void r_shader_sprite_uniform(r_sprite sprite, int uid, void *data);
void r_shader_clear_arrays(r_shader shader);

// This is the entry point for r_uniform_map
int r_shader_setup_array(r_shader shader, const char *name, int capacity,
                         int type);

r_uniform_map *r_shader_get_uniform_map(r_shader shader);

int r_shader_get_array_index(r_shader shader, const char *name);
r_uniform_array *r_shader_get_arrayi(r_shader shader, r_sheet sheet, int uid);

void r_set_uniformf(r_shader shader, const char *name, f32 value);
void r_set_uniformi(r_shader shader, const char *name, int value);
void r_set_v4(r_shader shader, const char *name, vec4 value);
void r_set_v3(r_shader shader, const char *name, vec3 value);
void r_set_v2(r_shader shader, const char *name, vec2 value);
void r_set_m4(r_shader shader, const char *name, mat4x4 value);

void r_set_m4x(r_shader shader, u32 count, const char *name, mat4x4 *values);
void r_set_ix(r_shader shader, u32 count, const char *name, int *values);
void r_set_fx(r_shader shader, u32 count, const char *name, f32 *values);
void r_set_v2x(r_shader shader, u32 count, const char *name, vec2 *values);
void r_set_v3x(r_shader shader, u32 count, const char *name, vec3 *values);
void r_set_v4x(r_shader shader, u32 count, const char *name, vec4 *values);

// ---------- WINDOW FUNCTIONS -------------

void r_window_get_size(int *w, int *h);

// return length of the string
int r_get_videomode_str(char *dst, int index);
void r_set_videomode(int index);
void r_select_mode(int index, int fullscreen, int vsync, int borderless);
int r_get_vidmode_count(void);

int r_allow_render(void);
int r_is_vsync(void);
int r_is_fullscreen(void);
int r_is_borderless(void);

int r_window_create(r_window_info info);
void r_window_destroy(void);
void r_window_request_close(void);
int r_window_set_icon(asset_t *asset);

void r_window_center(void);
void r_window_set_pos(int x, int y);

int r_window_should_close(void);

void r_window_swap_buffers(void);
void r_window_clear(void);
#endif

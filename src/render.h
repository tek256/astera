#ifndef r_H
#define r_H

#include <GLFW/glfw3.h>
#include <misc/linmath.h>

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
  float gamma;
  char *title;
  char *icon;
} r_window_info;

typedef struct {
  int width, height;
  int x, y;
  int fullscreen, vsync, borderless;
  int close_requested;
  int refreshRate;
  float gamma;
  GLFWwindow *glfw;
} r_window;

typedef uint32_t r_shader;

typedef struct {
  uint32_t fbo, tex, rbo;
  uint32_t width, height;
  // fuck it, we'll have the quad in here too for now
  uint32_t vao, vbo, vto;
  r_shader shader;
  mat4x4 model, view;
} r_framebuffer;

typedef struct {
  vec3 pos;
  vec3 rot;
  mat4x4 view;
  mat4x4 proj;
  vec2 size;
  float fov;
  float near;
  float far;
} r_camera;

typedef struct {
  uint32_t id;
  uint32_t width, height;
} r_tex;

typedef struct {
  uint32_t id;
  uint32_t width, height;
  uint32_t subwidth, subheight;
} r_sheet;

typedef struct {
  r_sheet sheet;
  uint32_t sub_id;
} r_subtex;

typedef struct {
  uint32_t *frames;
  uint32_t frame;
  uint32_t frame_count;
  uint32_t frame_rate;
  uint32_t uid;
  r_sheet sheet;

  uint8_t pstate;
  uint8_t state;

  float time;

  int loop : 1;
  int use : 1;
} r_anim;

typedef struct {
  r_anim *anims;
  const char **names;
  uint16_t high;
  uint16_t count;
  uint16_t capacity;
} r_anim_map;

typedef struct {
  vec2 position, size;

  r_shader shader;

  union {
    r_anim anim;
    r_subtex tex;
  } render;

  uint8_t layer;
  int flip_x;
  int flip_y;
  int change : 1;
  int animated : 1;
  int visible : 1;

  int used : 1;

  vec4 color;

  mat4x4 model;
} r_sprite;

#define R_ANIM_STOP 0x00
#define R_ANIM_PLAY 0x01
#define R_ANIM_PAUSE 0x10

typedef enum { r_vec2 = 0, r_vec3, r_vec4, r_float, r_int, r_mat } uniform_type;

typedef struct {
  const char *name;
  int type;
  r_shader shader;

  uint32_t uid;

  // Let me look up the framebuffer API real quick so I don't botch this
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
  uint32_t *uids;
  int *types;
  unsigned int *locations;
  unsigned int *capacities;

  int used : 1;
} r_uniform_map;

typedef struct {
  r_shader shader;
  r_sheet sheet;

  r_uniform_array *uniform_arrays;
  uint32_t uniform_array_count;

  int sprite_count, sprite_capacity;
  int used : 1;
} r_shader_batch;

typedef struct {
  r_shader *shaders;
  const char **names;
  r_uniform_map *uniform_maps;
  uint32_t count;
  uint32_t capacity;

  r_shader_batch *batches;
  uint32_t batch_capacity;
} r_shader_map;

typedef enum {
  CURVE_LINEAR = 0,
  CURVE_EASE_IN,
  CURVE_EASE_OUT,
  CURVE_EASE_EASE
} r_keyframe_curve;

typedef struct {
  float point;
  float value;
  r_keyframe_curve curve;
} r_keyframe;

typedef struct {
  r_keyframe *list;
  unsigned int count;
} r_keyframes;

typedef enum { SPAWN_POINT = 0, SPAWN_CIRCLE, SPAWN_BOX } r_particle_spawn;
typedef enum {
  DIR_NONE = 0,
  DIR_CIRCLE,
  DIR_CIRCLE_UNIFORM,
  DIR_PREDEF
} r_particle_dir;

typedef struct {
  union {
    r_anim anim;
    r_subtex tex;
  } render;

  vec4 color;

  float life;
  vec2 position, size;

  uint8_t layer;
  mat4x4 model;

  int alive : 1;
  int reused : 1;
} r_particle;

typedef struct {
  r_particle *list;
  unsigned int capacity, count;
  unsigned int max_emission;
  unsigned int emission_count;

  float particle_life, system_life;
  float spawn_rate;

  float time, spawn_time;

  uint8_t layer;

  vec2 position, size, velocity;
  vec2 particle_size;

  r_particle_spawn spawn_type;
  r_particle_dir dir_type;

  r_keyframes fade_frames;
  r_keyframes size_frames;

  union {
    r_subtex tex;
    r_anim anim;
  } render;

  vec4 color;

  // One of the downsides is that we can't _really_ initialize this right away,
  // the nature of it means that the lifetime of these arrays start at first
  // draw call
  r_uniform_array *arrays;
  int array_count;

  int animated : 1;
  int texture : 1;
  int colored : 1;
  int valid_uniforms : 1;
} r_particles;

static r_window g_window;
static r_camera g_camera;

void r_init_anim_map(int size);
void r_init_shader_map(int size);
void r_init_batches(int size);

void r_get_color(vec3 val, char *v);

int r_init(r_window_info info);
void r_exit(void);
void r_update(void);
void r_end(void);

r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                   r_shader shader);
void r_framebuffer_destroy(r_framebuffer fbo);
void r_framebuffer_bind(r_framebuffer fbo);
void r_framebuffer_draw(r_framebuffer fbo);

r_tex r_tex_create(asset_t *asset);
void r_tex_bind(uint32_t tex);

r_sheet r_sheet_create(asset_t *asset, uint32_t subwidth, uint32_t subheight);
void r_sheet_bind(uint32_t sheet);

float r_keyframe_get_value(r_keyframes frames, float point);
r_keyframes r_keyframes_create(int keyframe_count);
void r_keyframes_set(r_keyframes *frames, int frame, float point, float value,
                     r_keyframe_curve curve);

void r_particles_init(r_particles *system, unsigned int particle_capacity);
void r_particles_update(r_particles *system, double delta);
void r_particles_destroy(r_particles *particles);
void r_particles_draw(r_particles *particles, r_shader shader);

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

r_anim r_anim_create(r_sheet sheet, uint32_t *frames, int frame_count,
                     int frame_rate);
void r_anim_destroy(int uid);
int r_anim_get_index(const char *name);
r_anim *r_anim_get(int uid);
int r_anim_cache(r_anim anim, const char *name);

void r_anim_play(r_anim *anim);
void r_anim_stop(r_anim *anim);
void r_anim_pause(r_anim *anim);

r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size);

r_sheet r_sprite_get_sheet(r_sprite sprite);
int r_sprite_get_sheet_id(r_sprite sprite);

void r_sprite_play(r_sprite *sprite);
void r_sprite_pause(r_sprite *sprite);
void r_sprite_stop(r_sprite *sprite);

int r_sprite_get_tex_id(r_sprite sprite);
void r_sprite_set_anim(r_sprite *drawable, r_anim anim);
void r_sprite_set_tex(r_sprite *drawable, r_subtex tex);
void r_sprite_update(r_sprite *drawable, long delta);
void r_sprite_req_draw(r_sprite *drawable);

void r_cam_create(r_camera *camera, vec2 size, vec2 position);
void r_cam_update(void);
void r_cam_move(float x, float y);
void r_cam_get_size(float *width, float *height);
void r_cam_set_size(float width, float height);

r_shader r_shader_create(asset_t *vert, asset_t *frag);
r_shader r_shader_get(const char *name);

void r_shader_bind(r_shader shader);
void r_shader_destroy(r_shader shader);

void r_shader_cache(r_shader shader, const char *name);

void r_shader_uniform(r_shader shader, r_sheet sheet, const char *name,
                      void *data, int count);
void r_shader_uniformi(r_shader shader, r_sheet sheet, int uid, void *data,
                       int count);
void r_shader_sprite_uniform(r_sprite sprite, int uid, void *data);
void r_shader_clear_arrays(r_shader shader);

int r_shader_setup_array(r_shader shader, const char *name, int capacity,
                         int type);

r_uniform_map *r_shader_get_uniform_map(r_shader shader);

int r_shader_get_array_index(r_shader shader, const char *name);
r_uniform_array *r_shader_get_arrayi(r_shader shader, r_sheet sheet, int uid);

void r_set_uniformf(r_shader shader, const char *name, float value);
void r_set_uniformi(r_shader shader, const char *name, int value);
void r_set_v4(r_shader shader, const char *name, vec4 value);
void r_set_v3(r_shader shader, const char *name, vec3 value);
void r_set_v2(r_shader shader, const char *name, vec2 value);
void r_set_m4(r_shader shader, const char *name, mat4x4 value);

void r_set_m4x(r_shader shader, uint32_t count, const char *name,
               mat4x4 *values);
void r_set_ix(r_shader shader, uint32_t count, const char *name, int *values);
void r_set_fx(r_shader shader, uint32_t count, const char *name, float *values);
void r_set_v2x(r_shader shader, uint32_t count, const char *name, vec2 *values);
void r_set_v3x(r_shader shader, uint32_t count, const char *name, vec3 *values);
void r_set_v4x(r_shader shader, uint32_t count, const char *name, vec4 *values);

void r_window_get_size(int *w, int *h);

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
void r_window_clear_color(const char *str);

int r_get_refresh_rate(void);
#endif

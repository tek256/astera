#ifndef ASTERA_RENDER_HEADER
#define ASTERA_RENDER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <GLFW/glfw3.h>

#include <astera/export.h>
#include <astera/linmath.h>

#include <stdint.h>

#if !defined(ASTERA_RENDER_LAYER_MOD)
#define ASTERA_RENDER_LAYER_MOD 0.001
#endif

#if !defined(ASTERA_MORE_IMG_SUPPORT)
#define STBI_NO_BMP
#define STBI_NO_TGA
#define STBI_NO_JPEG
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_HDR
#endif

typedef struct {
  int      allowed : 1;
  int      scaled : 1;
  uint16_t video_mode_count;
} r_flags;

typedef struct {
  int         width, height;
  int         max_width, max_height;
  int         min_width, min_height;
  int         fullscreen, vsync, borderless;
  int         refresh_rate;
  int         resizable;
  float       gamma;
  const char* title;
  char*       icon;
} r_window_info;

typedef struct {
  int         width, height;
  int         max_width, max_height;
  int         min_width, min_height;
  int         x, y;
  int         resizable;
  int         fullscreen, vsync, borderless;
  int         close_requested;
  int         refresh_rate;
  float       gamma;
  GLFWwindow* glfw;
} r_window;

typedef uint32_t r_shader;

typedef struct {
  uint32_t fbo, tex, rbo;
  uint32_t width, height;
  uint32_t vao, vbo, vto;
  r_shader shader;
  mat4x4   model, view;
} r_framebuffer;

typedef struct {
  vec3   pos;
  vec3   rot;
  mat4x4 view;
  mat4x4 proj;
  vec2   size;
  float  fov;
  float  near;
  float  far;
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
  r_sheet  sheet;
  uint32_t sub_id;
} r_subtex;

typedef struct {
  uint32_t x, y;
  int32_t  sub_id;
  int32_t  layer;
  int      flip_x : 1;
  int      flip_y : 1;
} r_baked_quad;

typedef struct {
  uint32_t vao, vbo, vboi;
  r_sheet  sheet;

  r_baked_quad* quads;
  uint32_t      quad_count;
  uint32_t      width, height;
  int32_t       layer;

  vec2   position, size;
  mat4x4 model;
} r_baked_sheet;

typedef struct {
  uint32_t* frames;
  uint32_t  frame;
  uint32_t  frame_count;
  uint32_t  frame_rate;
  uint32_t  uid;
  r_sheet   sheet;

  uint8_t pstate;
  uint8_t state;

  float time;

  int loop : 1;
  int use : 1;
} r_anim;

typedef struct {
  r_anim*      anims;
  const char** names;
  uint16_t     high;
  uint16_t     count;
  uint16_t     capacity;
} r_anim_map;

typedef struct {
  vec2 position, size;

  r_shader shader;

  union {
    r_anim   anim;
    r_subtex tex;
  } render;

  uint8_t layer;
  int     flip_x;
  int     flip_y;
  int     change : 1;
  int     animated : 1;
  int     visible : 1;

  int used : 1;

  vec4 color;

  mat4x4 model;
} r_sprite;

#define R_ANIM_STOP  0x00
#define R_ANIM_PLAY  0x01
#define R_ANIM_PAUSE 0x10

#define r_vec2  0
#define r_vec3  1
#define r_vec4  2
#define r_float 3
#define r_int   4
#define r_mat   5

typedef struct {
  const char* name;
  int         type;
  r_shader    shader;

  uint32_t uid;

  union {
    mat4x4* mat_ptr;
    float*  float_ptr;
    int*    int_ptr;
    vec2*   vec2_ptr;
    vec3*   vec3_ptr;
    vec4*   vec4_ptr;
  } data;

  unsigned int count;
  unsigned int capacity;
} r_uniform_array;

typedef struct {
  r_shader shader;
  int      count;

  const char**  names;
  uint32_t*     uids;
  int*          types;
  unsigned int* locations;
  unsigned int* capacities;

  int used : 1;
} r_uniform_map;

typedef struct {
  r_shader shader;
  r_sheet  sheet;

  r_uniform_array* uniform_arrays;
  uint32_t         uniform_array_count;

  int sprite_count, sprite_capacity;
  int used : 1;
} r_shader_batch;

typedef struct {
  r_shader*      shaders;
  const char**   names;
  r_uniform_map* uniform_maps;
  uint32_t       count;
  uint32_t       capacity;

  r_shader_batch* batches;
  uint32_t        batch_capacity;
} r_shader_map;

typedef enum {
  CURVE_LINEAR = 0,
  CURVE_EASE_IN,
  CURVE_EASE_OUT,
  CURVE_EASE_EASE
} r_keyframe_curve;

typedef struct {
  float            point;
  float            value;
  r_keyframe_curve curve;
} r_keyframe;

typedef struct {
  r_keyframe*  list;
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
    r_anim   anim;
    r_subtex tex;
  } render;

  vec4 color;

  float life;
  vec2  position, size;

  uint8_t layer;
  mat4x4  model;

  int alive : 1;
  int reused : 1;
} r_particle;

typedef struct {
  r_particle*  list;
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
  r_particle_dir   dir_type;

  r_keyframes fade_frames;
  r_keyframes size_frames;

  union {
    r_subtex tex;
    r_anim   anim;
  } render;

  vec4 color;

  r_uniform_array* arrays;
  int              array_count;

  int animated : 1;
  int texture : 1;
  int colored : 1;
  int valid_uniforms : 1;
} r_particles;

static r_window g_window;
static r_camera g_camera;

ASTERA_API int8_t r_check_error(void);
ASTERA_API int8_t r_check_error_loc(const char* loc);

ASTERA_API void r_init_anim_map(int size);
ASTERA_API void r_init_shader_map(int size);
ASTERA_API void r_init_batches(int size);

ASTERA_API void r_get_color(vec3 val, const char* v);

ASTERA_API int  r_init(r_window_info info);
ASTERA_API void r_exit(void);
ASTERA_API void r_update(void);
ASTERA_API void r_end(void);

ASTERA_API r_framebuffer r_framebuffer_create(uint32_t width, uint32_t height,
                                              r_shader shader);
ASTERA_API void          r_framebuffer_destroy(r_framebuffer fbo);
ASTERA_API void          r_framebuffer_bind(r_framebuffer fbo);
ASTERA_API void          r_framebuffer_draw(r_framebuffer fbo);

ASTERA_API r_tex r_tex_create(unsigned char* data, int length);
ASTERA_API void  r_tex_bind(uint32_t tex);

ASTERA_API r_sheet r_sheet_create(unsigned char* data, int length,
                                  uint32_t subwidth, uint32_t subheight);
ASTERA_API void    r_sheet_bind(uint32_t sheet);

ASTERA_API r_baked_sheet r_baked_sheet_create(r_sheet       sheet,
                                              r_baked_quad* quads,
                                              uint32_t quad_count, vec2 pos,
                                              uint32_t layer);
ASTERA_API void r_baked_sheet_draw(r_shader shader, r_baked_sheet* sheet);
ASTERA_API void r_baked_sheet_destroy(r_baked_sheet* sheet);

ASTERA_API float       r_keyframe_get_value(r_keyframes frames, float point);
ASTERA_API r_keyframes r_keyframes_create(int keyframe_count);
ASTERA_API void r_keyframes_set(r_keyframes* frames, int frame, float point,
                                float value, r_keyframe_curve curve);

ASTERA_API void r_particles_init(r_particles* system,
                                 unsigned int particle_capacity);
ASTERA_API void r_particles_update(r_particles* system, double delta);
ASTERA_API void r_particles_destroy(r_particles* particles);
ASTERA_API void r_particles_draw(r_particles* particles, r_shader shader);

ASTERA_API void r_sprite_draw(r_sprite draw);
ASTERA_API void r_batch_draw(r_shader_batch* batch);

ASTERA_API int  r_batch_count(void);
ASTERA_API int  r_sprite_draw_count(void);
ASTERA_API void r_batch_info(void);
ASTERA_API void r_batch_clear(r_shader_batch* batch);

ASTERA_API void r_shader_clear_array(r_uniform_array* array);
ASTERA_API r_shader_batch* r_batch_create(r_shader shader, r_sheet sheet);

ASTERA_API r_shader_batch* r_shader_get_batch(r_shader shader, r_sheet sheet);
ASTERA_API void            r_batch_set_arrays(r_shader_batch* batch);
ASTERA_API void            r_batch_destroy_all(r_shader shader);
ASTERA_API void            r_batch_destroy(r_shader shader, r_sheet sheet);

ASTERA_API r_anim r_anim_create(r_sheet sheet, uint32_t* frames,
                                int frame_count, int frame_rate);
ASTERA_API void   r_anim_destroy(int uid);
ASTERA_API int    r_anim_get_index(const char* name);
ASTERA_API r_anim* r_anim_get(int uid);
ASTERA_API int     r_anim_cache(r_anim anim, const char* name);

ASTERA_API void r_anim_play(r_anim* anim);
ASTERA_API void r_anim_stop(r_anim* anim);
ASTERA_API void r_anim_pause(r_anim* anim);

ASTERA_API r_sprite r_sprite_create(r_shader shader, vec2 pos, vec2 size);

ASTERA_API r_sheet r_sprite_get_sheet(r_sprite sprite);
ASTERA_API int     r_sprite_get_sheet_id(r_sprite sprite);

ASTERA_API void r_sprite_play(r_sprite* sprite);
ASTERA_API void r_sprite_pause(r_sprite* sprite);
ASTERA_API void r_sprite_stop(r_sprite* sprite);

ASTERA_API r_subtex r_subtex_create(r_sheet sheet, uint32_t id);

ASTERA_API int  r_sprite_get_tex_id(r_sprite sprite);
ASTERA_API void r_sprite_set_anim(r_sprite* drawable, r_anim anim);
ASTERA_API void r_sprite_set_tex(r_sprite* drawable, r_subtex tex);
ASTERA_API void r_sprite_update(r_sprite* drawable, long delta);

ASTERA_API void r_cam_create(r_camera* camera, vec2 size, vec2 position);
ASTERA_API void r_cam_update(void);
ASTERA_API void r_cam_move(float x, float y);
ASTERA_API void r_cam_get_size(float* width, float* height);
ASTERA_API void r_cam_set_size(float width, float height);

ASTERA_API r_shader r_shader_create(unsigned char* vert, int vert_length,
                                    unsigned char* frag, int frag_length);
ASTERA_API r_shader r_shader_get(const char* name);

ASTERA_API void r_shader_bind(r_shader shader);
ASTERA_API void r_shader_destroy(r_shader shader);

ASTERA_API void r_shader_cache(r_shader shader, const char* name);

ASTERA_API void r_shader_uniform(r_shader shader, r_sheet sheet,
                                 const char* name, void* data, int count);
ASTERA_API void r_shader_uniformi(r_shader shader, r_sheet sheet, int uid,
                                  void* data, int count);
ASTERA_API void r_shader_sprite_uniform(r_sprite sprite, int uid, void* data);
ASTERA_API void r_shader_clear_arrays(r_shader shader);

ASTERA_API int r_shader_setup_array(r_shader shader, const char* name,
                                    int capacity, int type);

ASTERA_API r_uniform_map* r_shader_get_uniform_map(r_shader shader);

ASTERA_API int r_shader_get_array_index(r_shader shader, const char* name);
ASTERA_API r_uniform_array* r_shader_get_arrayi(r_shader shader, r_sheet sheet,
                                                int uid);

ASTERA_API void r_set_uniformf(r_shader shader, const char* name, float value);
ASTERA_API void r_set_uniformi(r_shader shader, const char* name, int value);
ASTERA_API void r_set_v4(r_shader shader, const char* name, vec4 value);
ASTERA_API void r_set_v3(r_shader shader, const char* name, vec3 value);
ASTERA_API void r_set_v2(r_shader shader, const char* name, vec2 value);
ASTERA_API void r_set_m4(r_shader shader, const char* name, mat4x4 value);

ASTERA_API void r_set_m4x(r_shader shader, uint32_t count, const char* name,
                          mat4x4* values);
ASTERA_API void r_set_ix(r_shader shader, uint32_t count, const char* name,
                         int* values);
ASTERA_API void r_set_fx(r_shader shader, uint32_t count, const char* name,
                         float* values);
ASTERA_API void r_set_v2x(r_shader shader, uint32_t count, const char* name,
                          vec2* values);
ASTERA_API void r_set_v3x(r_shader shader, uint32_t count, const char* name,
                          vec3* values);
ASTERA_API void r_set_v4x(r_shader shader, uint32_t count, const char* name,
                          vec4* values);

ASTERA_API void r_window_get_size(int* w, int* h);

ASTERA_API int  r_get_videomode_str(char* dst, int index);
ASTERA_API void r_set_videomode(int index);
ASTERA_API void r_select_mode(int index, int fullscreen, int vsync,
                              int borderless);
ASTERA_API int  r_get_vidmode_count(void);

ASTERA_API int r_allow_render(void);
ASTERA_API int r_is_vsync(void);
ASTERA_API int r_is_fullscreen(void);
ASTERA_API int r_is_borderless(void);

ASTERA_API void r_poll_events(void);

ASTERA_API int  r_window_create(r_window_info info);
ASTERA_API void r_window_destroy(void);
ASTERA_API void r_window_request_close(void);
ASTERA_API int  r_window_set_icon(unsigned char* data, int length);

ASTERA_API void r_window_center(void);
ASTERA_API void r_window_set_pos(int x, int y);

ASTERA_API int r_window_should_close(void);

ASTERA_API void r_window_swap_buffers(void);
ASTERA_API void r_window_clear(void);
ASTERA_API void r_window_clear_color(const char* str);

ASTERA_API int r_window_is_resizable(void);

ASTERA_API int  r_window_get_max_width(void);
ASTERA_API int  r_window_get_max_height(void);
ASTERA_API void r_window_get_max_bounds(int* width, int* height);
ASTERA_API void r_window_set_max_bounds(int width, int height);

ASTERA_API int  r_window_get_min_width(void);
ASTERA_API int  r_window_get_min_height(void);
ASTERA_API void r_window_get_min_bounds(int* width, int* height);
ASTERA_API void r_window_set_min_bounds(int width, int height);

ASTERA_API void r_window_set_size_bounds(int min_width, int min_height,
                                         int max_width, int max_height);

ASTERA_API void r_window_request_attention(void);
ASTERA_API int  r_window_is_focused(void);

ASTERA_API r_window_info r_window_info_create(int width, int height,
                                              int max_width, int max_height,
                                              int min_width, int min_height,
                                              int resizable, const char* name,
                                              int refresh_rate, int vsync,
                                              int fullscreen, int borderless);

ASTERA_API int r_get_refresh_rate(void);

#ifdef __cplusplus
}
#endif

#endif


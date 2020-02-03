#ifndef ASTERA_AUDIO_HEADER
#define ASTERA_AUDIO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <AL/al.h>
#include <AL/alc.h>

#include <misc/linmath.h>

#include <stdint.h>

#define STB_VORBIS_HEADER_ONLY
#include <misc/stb_vorbis.c>

#include "asset.h"
#include "sys.h"

#if !defined(AUDIO_FRAME_SIZE)
#define AUDIO_FRAME_SIZE 4096
#endif

#if !defined(AUDIO_DEFAULT_FRAMES_PER_BUFFER)
#define AUDIO_DEFAULT_FRAMES_PER_BUFFER 64
#endif

#if !defined(AUDIO_BUFFERS_PER_MUSIC)
#define AUDIO_BUFFERS_PER_MUSIC 2
#endif

#if !defined(AUDIO_MUSIC_MAX_FAILS)
#define AUDIO_MUSIC_MAX_FAILS 3
#endif

#if !defined(AUDIO_CUSTOM_LAYERS)
#define AUDIO_DEFAULT_LAYERS

#define AUDIO_SFX_LAYER 0
#define AUDIO_MUSIC_LAYER 1
#define AUDIO_MISC_LAYER 2
#define AUDIO_UI_LAYER 3
#define MAX_AUDIO_LAYERS 4

#if !defined(MAX_AUDIO_LAYERS)
#define MAX_AUDIO_LAYERS 4
#endif

#if !defined(AUDIO_SFX_LAYER)
#define AUDIO_SFX_LAYER 0
#endif
#if !defined(AUDIO_MUSIC_LAYER)
#define AUDIO_MUSIC_LAYER 1
#endif

#if !defined(MAX_AUDIO_LAYERS)
#define MAX_AUDIO_LAYERS 2
#endif
#endif

#if defined(AUDIO_DEFAULT_LAYERS)
#endif

#if defined(AUDIO_SFX_LAYER) && !defined(AUDIO_SFX_GAIN)
#define AUDIO_SFX_GAIN 0.8f
#endif
#if defined(AUDIO_MUSIC_LAYER) && !defined(AUDIO_MUSIC_GAIN)
#define AUDIO_MUSIC_GAIN 0.5f
#endif
#if defined(AUDIO_MISC_LAYER) && !defined(AUDIO_MISC_GAIN)
#define AUDIO_MISC_GAIN 0.8f
#endif
#if defined(AUDIO_UI_LAYER) && !defined(AUDIO_UI_GAIN)
#define AUDIO_UI_GAIN 0.8f
#endif

#if !defined(MAX_LAYER_SFX)
#define MAX_LAYER_SFX 32
#endif
#if !defined(MAX_LAYER_SONGS)
#define MAX_LAYER_SONGS 4
#endif
#if !defined(MAX_BUFFERS)
#define MAX_BUFFERS 64
#endif

#define LAYER_STOPPED 1
#define LAYER_PLAYING 2
#define LAYER_PAUSED 3

#if !defined(MAX_MUSIC_RUNTIME)
#define MAX_MUSIC_RUNTIME 4096
#endif

#if !defined(MAX_SFX)
#define MAX_SFX MAX_LAYER_SFX *MAX_AUDIO_LAYERS / 2
#endif
#if !defined(MAX_SONGS)
#define MAX_SONGS MAX_LAYER_SONGS *MAX_AUDIO_LAYERS
#endif

#if !defined(DEFAULT_SFX_RANGE)
#define DEFAULT_SFX_RANGE 20
#endif

typedef struct {
  ALCcontext *context;
  ALCdevice *device;
  int allow : 1;
} a_ctx;

typedef struct {
  float gain;
  vec3 pos;
} a_listener;

typedef struct {
  uint32_t id;
  uint16_t channels;
  float length;
  uint32_t sample_rate;
} a_buf;

typedef struct {
  // non-realtime
  uint16_t layer;

  // realtime
  vec3 pos;
  vec3 vel;
  float gain, range, max_range;
  uint16_t max_loop;
  int loop : 1;
  int stop : 1;

  // callback
  uint16_t loop_count;
  float time;
} a_req;

typedef struct {
  char *file_path;
  char *name;
  char **names;
  int *offsets;
  int offset_count;
  int total_samples;
} a_meta;

typedef struct {
  int format;
  uint32_t source;
  uint32_t buffers[AUDIO_BUFFERS_PER_MUSIC];
  float gain;

  uint32_t current_sample;
  uint32_t samples_left;
  uint32_t total_samples;
  stb_vorbis *vorbis;

  a_meta *meta;

  int32_t sample_rate, sample_count;

  int16_t packets_per_buffer;

  uint8_t *data;
  int32_t data_length, data_offset;
  int32_t header_end;

  uint16_t *pcm;
  uint32_t pcm_length;

  float delta;

  a_req *req;
  int has_req;

  int loop : 1;
} a_music;

typedef struct {
  uint32_t id;
  vec3 position;
  float range, gain;
  int loop;
  a_req *req;
  int has_req;
} a_sfx;

typedef struct {
  uint32_t id;
  float gain;

  uint32_t sfx_count;
  uint32_t music_count;

  a_sfx *sources[MAX_LAYER_SFX];
  a_music *musics[MAX_LAYER_SONGS];

  uint32_t gain_change : 1;
} a_layer;

typedef struct {
  uint16_t song_count;
  uint16_t song_capacity;
  a_music songs[MAX_SONGS];

  uint16_t sfx_count;
  uint16_t sfx_capacity;
  a_sfx sfx[MAX_SFX];

  a_layer layers[MAX_AUDIO_LAYERS];
  uint16_t layer_count;

  uint16_t buf_count;
  uint16_t buf_capacity;
  a_buf bufs[MAX_BUFFERS];
  const char *buf_names[MAX_BUFFERS];
} a_resource_map;

static a_resource_map g_a_map;
static a_ctx g_a_ctx;
static a_listener g_listener;

int a_init(const char *device, uint32_t master, uint32_t sfx, uint32_t music);
void a_exit();

int a_can_play(void);

void a_set_pos(vec3 p);

void a_set_vol(uint32_t master, uint32_t sfx, uint32_t music);
void a_set_vol_master(uint32_t master);
void a_set_vol_sfx(uint32_t sfx);
void a_set_vol_music(uint32_t sfx);

float a_get_vol_master(void);
float a_get_vol_sfx(void);
float a_get_vol_music(void);

void a_update(time_s delta);
void a_update_sfx(void);

uint32_t a_get_device_name(char *dst, int capacity);

int8_t a_layer_add_music(uint32_t id, a_music *music);
int8_t a_layer_add_sfx(uint32_t id, a_sfx *sfx);

a_buf a_buf_create(asset_t *asset);
a_buf *a_buf_get(const char *name);
void a_buf_destroy(a_buf buffer);

a_sfx *a_play_sfxn(const char *name, a_req *req);
a_sfx *a_play_sfx(a_buf *buff, a_req *req);

int a_ctx_create(const char *device_name);
void a_destroy_context(void);

a_music *a_music_create(asset_t *asset, a_meta *meta, a_req *req);
void a_music_reset(a_music *music);
void a_music_destroy(a_music *music);

time_s a_music_len(a_music *music);
time_s a_music_time(a_music *music);

void a_music_play(a_music *music);
void a_music_stop(a_music *music);
void a_music_resume(a_music *music);
void a_music_pause(a_music *music);

#ifdef __cplusplus
}
#endif
#endif

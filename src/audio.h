#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <misc/linmath.h>

#include <stdio.h>

#define STB_VORBIS_HEADER_ONLY
#include <misc/stb_vorbis.c>

#include "debug.h"
#include "platform.h"

#define AUDIO_FRAME_SIZE 4096
#define AUDIO_DEFAULT_FRAMES_PER_BUFFER 8
#define AUDIO_BUFFERS_PER_MUSIC 2

#define AUDIO_DEFAULT_LAYERS

#if defined(AUDIO_DEFAULT_LAYERS)
#define AUDIO_SFX_LAYER 0
#define AUDIO_MUSIC_LAYER 1
#define AUDIO_MISC_LAYER 2
#define AUDIO_UI_LAYER 3
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
#define MAX_SFX MAX_LAYER_SFX *MAX_AUDIO_LAYERS
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
  f32 gain;
  vec3 pos;
} a_listener;

typedef struct {
  u32 id;
  u16 channels;
  u16 length;
  u32 sample_rate;
} a_buf;

typedef struct {
  // non-realtime
  u16 layer;

  // realtime
  vec3 pos;
  vec3 vel;
  f32 gain, range, max_range;
  u16 max_loop;
  int loop : 1;
  int stop : 1;

  // callback
  u16 loop_count;
  f32 time;
} a_req;

typedef struct {
  char **names;
  int *offsets;
  int count;
} a_keyframes;

typedef struct {
  int format;
  u32 source;
  u32 buffers[AUDIO_BUFFERS_PER_MUSIC];
  f32 gain;

  u32 current_sample;
  u32 samples_left;
  u32 total_samples;
  stb_vorbis *vorbis;

  u32 *keyframes;
  u32 *keyframe_offsets;
  u32 keyframe_count;

  s32 sample_rate, sample_count;

  s16 packets_per_buffer;

  u8 *data;
  s32 data_length, data_offset;
  s32 header_end;

  u16 *pcm;
  u16 pcm_length;

  f32 delta;

  a_req *req;
  int has_req;

  int loop : 1;
} a_music;

typedef struct {
  u32 id;
  vec3 position;
  f32 range, gain;
  int loop;
  a_req *req;
  int has_req;
} a_sfx;

typedef struct {
  u32 id;
  f32 gain;

  u32 sfx_count;
  u32 song_count;

  a_sfx *sources[MAX_LAYER_SFX];
  a_music *musics[MAX_LAYER_SONGS];

  u32 gain_change : 1;
} a_layer;

typedef struct {
  u16 song_count;
  u16 song_capacity;
  a_music songs[MAX_SONGS];

  u16 sfx_count;
  u16 sfx_capacity;
  a_sfx sfx[MAX_SFX];

  a_layer layers[MAX_AUDIO_LAYERS];
  u16 layer_count;

  u16 buf_count;
  u16 buf_capacity;
  a_buf bufs[MAX_BUFFERS];
  const char *buf_names[MAX_BUFFERS];
} a_resource_map;

static a_resource_map _map;
static a_ctx _ctx;
static a_listener _listener;

int a_init(u32 master, u32 sfx, u32 music);
void a_exit();

int a_allow_play(void);

void a_set_pos(vec3 p);

void a_set_vol(u32 master, u32 sfx, u32 music);
void a_set_vol_master(u32 master);
void a_set_vol_sfx(u32 sfx);
void a_set_vol_music(u32 sfx);

f32 a_get_vol_master(void);
f32 a_get_vol_sfx(void);
f32 a_get_vol_music(void);

void a_update(long delta);
void a_update_sfx(void);

u32 a_get_device_name(char *dst, int capacity);

a_buf a_get_buf(unsigned char *data, u32 length);
a_buf *a_get_bufn(const char *name);
void a_destroy_buf(a_buf buffer);

a_sfx *a_play_sfxn(const char *name, a_req *req);
a_sfx *a_play_sfx(a_buf *buff, a_req *req);

static int a_get_open_sfx(void);

int a_create_context(const char *device_name);
void a_destroy_context(void);

static void a_compute_stereo(short *output, int num_c, float **data,
                             int d_offset, int len);
static void a_interleave_output(int buffer_c, short *buffer, int data_c,
                                float **data, int data_offset, int length);

a_keyframes a_get_keyframes(const char *name);
a_music *a_create_music(unsigned char *data, u32 length, s32 sample_count,
                        s32 *keyframes, s32 *keyframe_offsets,
                        s32 keyframe_size, a_req *req);
int a_update_music(a_music *music);
void a_destroy_music(a_music *music);

f32 a_get_music_len_time(a_music *music);
f32 a_get_music_time(a_music *music);

void a_play_music(a_music *music);
void a_stop_music(a_music *music);
void a_resume_music(a_music *music);
void a_pause_music(a_music *music);

#endif

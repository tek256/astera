#ifndef AUDIO_H
#define AUDIO_H

#include <AL/alc.h>
#include <AL/al.h>

#include <linmath.h>

#include <stdio.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include "platform.h"
#include "debug.h"

#define AUDIO_DEFAULT_LAYERS

#ifdef AUDIO_DEFAULT_LAYERS
#define AUDIO_SFX_LAYER 0
#define AUDIO_MUSIC_LAYER 1
#define AUDIO_MISC_LAYER 2
#define AUDIO_UI_LAYER 3

#define AUDIO_SFX_GAIN 0.8f
#define AUDIO_MUSIC_GAIN 0.5f
#define AUDIO_MISC_GAIN 0.8f
#define AUDIO_UI_GAIN 0.8f

#endif

#define MAX_LAYER_SFX 32
#define MAX_LAYER_SONGS 4
#define MAX_AUDIO_LAYERS 2
#define MAX_BUFFERS 64

#define LAYER_STOPPED 1
#define LAYER_PLAYING 2
#define LAYER_PAUSED  3

#define MAX_MUSIC_RUNTIME 4096

#define MAX_SFX MAX_LAYER_SFX * MAX_AUDIO_LAYERS
#define MAX_SONGS MAX_LAYER_SONGS * MAX_AUDIO_LAYERS

#define DEFAULT_SFX_RANGE 20

typedef struct {
    ALCcontext* context;
    ALCdevice*  device;
	f32 gain;
	int allow : 1;
} a_ctx;

typedef struct {
    u32 id;
    u16 channels;
    u16 length;
    u32   sample_rate;
} a_buf;

typedef struct {
	//non-realtime
	u16 layer;

	//realtime
	vec3 pos;
	vec3 vel;
	f32 gain, range;
	u16 max_loop;
	int loop : 1;
	int stop : 1; 
	
	//callback
	u16 loop_count;
	f32 time;
} a_req;

typedef struct {
	u32 sample_rate;
    u32 sample_size;
    u32 channels;

    int format;
    u32 source;
    u32 buffers[4];
	f32 gain;
	
	u32 samples_left;
    u32 total_samples;
    stb_vorbis* vorbis;

	u8* data;
	s32 used;
	u32 len, pos;

	u16* pcm;
	u16 pcm_len;

	a_req* req;
	int has_req;

    int loop : 1;
} a_music;

typedef struct {
    u32 id;
    vec3  position;
    f32 range, gain;
    int   loop;
	a_req* req;
	int has_req;
} a_sfx;

typedef struct {
	u32 id;
	f32 gain;

	u32 sfx_count;
	u32 song_count;

	a_sfx* sources[MAX_LAYER_SFX];
	a_music* musics[MAX_LAYER_SONGS];
} a_layer;

typedef struct {
	u16 song_count;
	u16 song_capacity;
	a_music songs[MAX_SONGS];

	u16 sfx_count;
	u16 sfx_capacity;
	a_sfx sfx[MAX_SFX];

	a_layer layers[MAX_AUDIO_LAYERS];


	u16 buf_count;
	u16 buf_capacity;
	a_buf bufs[MAX_BUFFERS];
	const char* buf_names[MAX_BUFFERS];
} a_resource_map;

static a_resource_map _map;
static a_ctx _ctx;

static char** a_device_list;
static int a_device_list_count = 0;


int          a_init(u32 master, u32 sfx, u32 music);
void         a_exit();

void		 a_update(long delta);
void         a_update_sfx();

a_buf        a_get_buf(unsigned char* data, u32 length);
a_buf*       a_get_bufn(const char* name);
void         a_destroy_buf(a_buf buffer);

a_sfx*       a_play_sfxn(const char* name, a_req* req);
a_sfx*		 a_play_sfx(a_buf* buff, a_req* req);

static int   a_get_open_sfx();

int          a_load_devices();
int          a_create_context(const char* device_name);
void         a_destroy_context();
const char** a_get_devices(int* count);
void         a_swap_device(const char* device_name);


static void a_compute_stereo(short* output, int num_c, float** data, int d_offset, int len);
static void  a_interleave_output(int buffer_c, short* buffer, int data_c, float** data, int data_offset, int length);

a_music*     a_create_music(unsigned char* data, u32 length, a_req* req);
int          a_update_music(a_music* music);
void         a_destroy_music(a_music* music);

f32        a_get_music_len_time(a_music* music);
f32        a_get_music_time(a_music* music);

void         a_play_music(a_music* music);
void         a_stop_music(a_music* music);
void         a_resume_music(a_music* music);
void         a_pause_music(a_music* music);

#endif

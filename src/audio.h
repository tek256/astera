#ifndef AUDIO_H
#define AUDIO_H

#ifndef EXCLUDE_AUDIO
#include <AL/alc.h>
#include <AL/al.h>

#include <linmath.h>

#include <stdio.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

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
#define MAX_AUDIO_LAYERS 4
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
	float gain;
	int allow : 1;
} a_ctx;

typedef struct {
    unsigned int id;
    unsigned short channels;
    unsigned short length;
    unsigned int   sample_rate;
} a_buf;

typedef struct {
	//non-realtime
	unsigned short layer;

	//realtime
	vec3 pos;
	vec3 vel;
	float gain, range;
	unsigned short max_loop;
	int loop : 1;
	int stop : 1; 
	
	//callback
	unsigned short loop_count;
	float time;
} a_req;

typedef struct {
	unsigned int sample_rate;
    unsigned int sample_size;
    unsigned int channels;

    int format;
    unsigned int source;
    unsigned int buffers[2];
	float gain;
	
	unsigned int samples_left;
    unsigned int total_samples;
    stb_vorbis* vorbis;
    int loop;

	a_req* req;
	int has_req;
} a_music;

typedef struct {
    unsigned int id;
    vec3  position;
    float range, gain;
    int   loop;
	a_req* req;
	int has_req;
} a_sfx;

typedef struct {
	unsigned int id;
	float gain;

	unsigned int sfx_count;
	unsigned int song_count;

	a_sfx* sources[MAX_LAYER_SFX];
	a_music* musics[MAX_LAYER_SONGS];
} a_layer;

typedef struct {
	unsigned short song_count;
	unsigned short song_capacity;
	a_music songs[MAX_SONGS];

	unsigned short sfx_count;
	unsigned short sfx_capacity;
	a_sfx sfx[MAX_SFX];

	a_layer layers[MAX_AUDIO_LAYERS];


	unsigned short buf_count;
	unsigned short buf_capacity;
	a_buf bufs[MAX_BUFFERS];
	const char* buf_names[MAX_BUFFERS];
} a_map;

static a_map _map;
static a_ctx _ctx;

static char** a_device_list;
static int a_device_list_count = 0;


int          a_init(unsigned int master, unsigned int sfx, unsigned int music);
void         a_exit();

void		 a_update(long delta);
void         a_update_sfx();

a_buf        a_get_buf(unsigned char* data, unsigned int length);
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

a_music*     a_create_music(unsigned char* data, unsigned int length, a_req* req);
int          a_update_music(a_music* music);
void         a_destroy_music(a_music* music);

float        a_get_music_len_time(a_music* music);
float        a_get_music_time(a_music* music);

void         a_play_music(a_music* music);
void         a_stop_music(a_music* music);
void         a_resume_music(a_music* music);
void         a_pause_music(a_music* music);

#endif
#endif

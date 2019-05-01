#ifndef AUDIO_H
#define AUDIO_H

#include <AL/alc.h>
#include <AL/al.h>
#include "math.h"

#include <stdlib.h>
#include <stdio.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include <string.h>

#define MAX_AUDIO_SOURCES_PER_LAYER 32
#define MAX_AUDIO_LAYERS 4

#ifndef STD_BOOL_H
#define STD_BOOL_H
typedef enum{false=0,true=1} bool;
#endif

typedef unsigned int uint;

#define LAYER_STOPPED 1
#define LAYER_PLAYING 2
#define LAYER_PAUSED  3

#define MAX_MUSIC_RUNTIME 4096

#define MAX_QUICK_SFX 8

#define CACHE_AUDIO_FILES

typedef struct {
    ALCcontext* context;
    ALCdevice*  device;
} a_ctx;

typedef struct {
    uint id;
    bool buffered;
    int  channels;
    int  length;
    int  sample_rate;
} a_buf;

typedef struct {
    uint sample_rate;
    uint sample_size;
    uint channels;

    int format;
    uint source;
    uint buffers[2];
} a_stream;

//just to make things clearer when loading
typedef char* a_music_ptr;
typedef char* a_sfx_ptr;

typedef struct {
    a_stream stream;
    uint samples_left;
    uint total_samples;
    stb_vorbis* vorbis;
    bool loop;
} a_music;

typedef struct {
    uint  id;
    vec3  position;
    float range, gain;
    bool  has_sound;
    bool  loop;
} a_src;

typedef struct {
    uint id;
    a_src source;
} a_sfx;

typedef struct {
    float gain;
    vec3 position;
    vec3 velocity;
    float range;
} a_sound;

static a_sfx a_qsfx[MAX_QUICK_SFX];
static int a_sfx_playing = 0;

static a_ctx _a_ctx;

static char** a_device_list;
static int a_device_list_count = 0;

static bool a_allow = false;

#ifdef CACHE_AUDIO_FILES
//psuedo-audio buffer pointer
typedef struct a_file {
    const char* path;
    a_buf* buffer;
} a_file;

#ifndef MAX_AUDIO_FILES
#define MAX_AUDIO_FILES 256
#endif

static a_file audio_files[MAX_AUDIO_FILES];
static int    audio_file_count = 0;

int          a_init();
void		 a_update(long delta);

a_file*      a_gen_file(const char* fp, a_buf* buffer);
int          a_get_file_index(const char* fp);
a_file*      a_get_file(const char* fp);
void         a_destroy_file(const char* fp);
a_buf*       a_get_buffer(const char* fp);
a_file**     a_load_sfx(const char** fp, int count);
void         a_destroy_file_cache();
#endif

bool         a_load_devices();
bool         a_create_context(const char* device_name);
void         a_destroy_context();
const char** a_get_devices(int* count);
void         a_swap_device(const char* device_name);

a_buf     a_create_buffer(const char* path);
a_buf*    a_create_buffers(const char** paths, int p_count, int* b_count);
void         a_destroy_buffer(a_buf buffer);
void         a_clean_buffers(a_buf* buffers, int count);

void         a_attach_buffer(a_src* src, a_buf* buf);
void         a_detach_buffer(a_src* src);

a_stream     a_create_stream(int sample_rate, int sample_size, int channels);
void         a_destroy_stream(a_stream stream);
void         a_update_stream(a_stream stream, const void* data, int sample_count);
a_music*     a_create_music(const char* path);
bool         a_update_music(a_music* music);
void         a_destroy_music(a_music* music);
float        a_get_music_len_time(a_music* music);
float        a_get_music_time(a_music* music);
void         a_play_music(a_music* music);
void         a_stop_music(a_music* music);
void         a_resume_music(a_music* music);
void         a_pause_music(a_music* music);
ALenum       a_get_music_state(a_music* music);
a_src        a_create_source(vec3 position, float range, uint buffer);

void         a_destroy_source(a_src source);
void         a_play_source(a_src* source);
void         a_pause_source(a_src* source);
void         a_stop_source(a_src* source);
int          a_src_state(a_src* source);
void         a_clean_sources(a_src* sources, int count);

int          a_play_sfx(a_buf* buffer, a_sound* sound);
void         a_pause_sfx(int index);
void         a_stop_sfx(int index);
bool         a_is_sfx_buffer(int index, a_buf* buffer);
int          a_get_sfx_state(int index);
void         a_update_sfx();
static int   a_get_open_sfx();

#endif

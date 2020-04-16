// POSSIBLE CHANGES
// TODO maybe patch to automatically track/delete all effects created
//       (ref: a_exit)
//
// TODO Filters

#ifndef ASTERA_AUDIO_HEADER
#define ASTERA_AUDIO_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/export.h>
#include <astera/linmath.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <stdint.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include <astera/asset.h>
#include <astera/sys.h>

#if !defined(AUDIO_FRAME_SIZE)
#define AUDIO_FRAME_SIZE 4096
#endif

#if !defined(AUDIO_DEFAULT_FRAMES_PER_BUFFER)
#define AUDIO_DEFAULT_FRAMES_PER_BUFFER 64
#endif

#if !defined(AUDIO_BUFFERS_PER_MUSIC)
#define AUDIO_BUFFERS_PER_MUSIC 2
#endif
// getting dog
#if !defined(AUDIO_MUSIC_MAX_FAILS)
#define AUDIO_MUSIC_MAX_FAILS 3
#endif

#if !defined(AUDIO_CUSTOM_LAYERS)
#define AUDIO_DEFAULT_LAYERS

#define AUDIO_SFX_LAYER   0
#define AUDIO_MUSIC_LAYER 1
#define AUDIO_MISC_LAYER  2
#define AUDIO_UI_LAYER    3
#define MAX_AUDIO_LAYERS  4

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
#define LAYER_PAUSED  3

#if !defined(MAX_MUSIC_RUNTIME)
#define MAX_MUSIC_RUNTIME 4096
#endif

#if !defined(MAX_FX)
#define MAX_FX 32
#endif

#if !defined(MAX_SFX)
#define MAX_SFX MAX_LAYER_SFX* MAX_AUDIO_LAYERS / 2
#endif
#if !defined(MAX_SONGS)
#define MAX_SONGS MAX_LAYER_SONGS* MAX_AUDIO_LAYERS
#endif

#if !defined(DEFAULT_SFX_RANGE)
#define DEFAULT_SFX_RANGE 20
#endif

typedef struct {
  ALCcontext* context;
  ALCdevice*  device;
  int         fx_per_source;
  uint32_t    max_fx;
  int         efx : 1;
  int         allow : 1;
} a_ctx;

typedef struct {
  float gain;
  vec3  pos;
  vec3  ori;
  vec3  vel;
  float _ori[6];
} a_listener;

typedef struct {
  uint32_t id;
  uint16_t channels;
  float    length;
  uint32_t sample_rate;
} a_buf;

typedef struct {
  char*  file_path;
  char*  name;
  char** names;
  int*   offsets;
  int    offset_count;
  int    total_samples;
} a_meta;

typedef enum {
  REVERB = 0x0001,
  EQ     = 0x000C,
} a_fx_type;

// TODO Filter / Delay effects

typedef struct {
  float  density;               // [0.0, 1.0],   default 1.0
  float  diffusion;             // [0.0, 1.0],   default 1.0
  float  gain;                  // [0.0, 1.0],   default 0.32
  float  gainhf;                // [0.0, 1.0],   default 0.89
  float  decay;                 // [0.1, 20.0],  default 1.49
  float  decay_hfratio;         // [0.1, 2.0],   default 0.83
  float  refl_gain;             // [0.0, 3.16],  default 0.05
  float  refl_delay;            // [0.0, 0.3],   default 0.007
  float  late_gain;             // [0.0, 10.0],  default 1.26
  float  late_delay;            // [0.0, 0.1],   default 0.011
  float  air_absorption_gainhf; // [0.892, 1.0], default 0.994
  float  room_rolloff_factor;   // [0.0, 10.0],  default: 0.0
  int8_t decay_hflimit;         // [0, 1],       default 1
} a_fx_reverb;

typedef struct {
  float low_gain;    // default: 1        [0.126, 7.943]
  float low_cutoff;  // hz, default: 200  [50.0, 800.0]
  float mid1_gain;   // default: 1        [0.126, 7.943]
  float mid1_center; // hz, default: 500  [200.0, 3000.0]
  float mid1_width;  // default: 1        [0.01, 1.0]
  float mid2_gain;   // default: 1        [0.126, 7.943]
  float mid2_center; // hz, default: 3000 [1000.0, 8000.0]
  float mid2_width;  // default: 1        [0.01, 1.0]
  float high_gain;   // default 1.0       [0.126, 7.943]
  float high_cutoff; // hz, default: 6000 [4000.0 - 16000.0]
} a_fx_eq;

typedef struct a_fx_slot a_fx_slot;
typedef struct a_fx      a_fx;

struct a_fx {
  uint32_t   id;
  a_fx_type  type;
  a_fx_slot* slot;
  void*      data;
};

struct a_fx_slot {
  uint32_t id;
  a_fx*    fx;
  float    gain;
};

typedef enum {
  LOW  = 0x0001,
  HIGH = 0x0002,
  BAND = 0x0003,
} a_filter_type;

typedef struct {
  uint32_t      id;
  a_filter_type type;
  union {
    struct {
      float gain;
      float gainhf;
    } low;
    struct {
      float gain;
      float gainlf;
    } high;
    struct {
      float gain;
      float gainlf;
      float gainhf;
    } band;
  } data;
} a_filter;

typedef struct {
  // non-realtime
  uint16_t  layer;
  a_fx**    fx;
  uint16_t  fx_count;
  a_filter* filter;

  // realtime
  vec3     pos;
  vec3     vel;
  float    gain, range;
  uint16_t max_loop;
  int      loop : 1;
  int      stop : 1;

  // callback
  uint16_t loop_count;
  time_s   time;
  int      playing : 1;
} a_req;

typedef struct {
  int      format;
  uint32_t source;
  uint32_t buffers[AUDIO_BUFFERS_PER_MUSIC];
  float    gain;

  uint32_t    current_sample;
  uint32_t    samples_left;
  uint32_t    total_samples;
  stb_vorbis* vorbis;

  a_meta* meta;

  int32_t sample_rate, sample_count;

  int16_t packets_per_buffer;

  uint8_t* data;
  int32_t  data_length, data_offset;
  int32_t  header_end;

  uint16_t* pcm;
  uint32_t  pcm_length;

  float delta;

  a_req* req;
  int    has_req;

  int loop : 1;
} a_music;

typedef struct {
  uint32_t  id;
  vec3      position;
  float     range, gain;
  int       loop;
  a_req*    req;
  int       has_req;
  uint32_t* fx_slots;
} a_sfx;

typedef struct {
  uint32_t id;
  float    gain;

  uint32_t sfx_count;
  uint32_t music_count;

  a_sfx*   sources[MAX_LAYER_SFX];
  a_music* musics[MAX_LAYER_SONGS];

  uint32_t gain_change : 1;
} a_layer;

typedef struct {
  uint16_t song_count;
  uint16_t song_capacity;
  a_music  songs[MAX_SONGS];

  uint16_t sfx_count;
  uint16_t sfx_capacity;
  a_sfx    sfx[MAX_SFX];

  a_layer  layers[MAX_AUDIO_LAYERS];
  uint16_t layer_count;

  uint16_t    buf_count;
  uint16_t    buf_capacity;
  a_buf       bufs[MAX_BUFFERS];
  const char* buf_names[MAX_BUFFERS];

  uint16_t   fx_count;
  uint16_t   fx_capacity;
  a_fx_slot* fx_slots;
} a_resource_map;

static a_resource_map g_a_map;
static a_ctx          g_a_ctx;
static a_listener     g_listener;

ASTERA_API void a_efx_info(void);

ASTERA_API int  a_init(const char* device, uint32_t master, uint32_t sfx,
                       uint32_t music);
ASTERA_API void a_exit();

ASTERA_API int a_can_play(void);

ASTERA_API void a_set_pos(vec3 p);
ASTERA_API void a_set_ori(vec3 d);
ASTERA_API void a_set_orif(float v[6]);
ASTERA_API void a_set_vel(vec3 v);

ASTERA_API void a_get_pos(vec3* p);
ASTERA_API void a_get_ori(vec3* o);
ASTERA_API void a_get_orif(float* f);
ASTERA_API void a_get_vel(vec3* v);

ASTERA_API void a_set_vol(uint32_t master, uint32_t sfx, uint32_t music);
ASTERA_API void a_set_vol_master(uint32_t master);
ASTERA_API void a_set_vol_sfx(uint32_t sfx);
ASTERA_API void a_set_vol_music(uint32_t sfx);

ASTERA_API float a_get_vol_master(void);
ASTERA_API float a_get_vol_sfx(void);
ASTERA_API float a_get_vol_music(void);

ASTERA_API void a_update(time_s delta);
ASTERA_API void a_update_sfx(void);

ASTERA_API a_fx_reverb a_fx_reverb_default(void);

ASTERA_API a_fx_reverb a_fx_reverb_create(
    float density, float diffusion, float gain, float gainhf, float decay,
    float decay_hfratio, float refl_gain, float refl_delay, float late_gain,
    float late_delay, float air_absorption_gainhf, float room_rolloff_factor,
    int8_t decay_hflimit);

ASTERA_API a_fx_eq a_fx_eq_create(float low, float low_cutoff, float mid1_gain,
                                  float mid1_center, float mid1_width,
                                  float mid2_gain, float mid2_center,
                                  float mid2_width, float high_gain,
                                  float high_cutoff);

ASTERA_API int16_t a_fx_slot_get();
ASTERA_API int16_t a_fx_slot_attach(a_fx* fx);
ASTERA_API void    a_fx_slot_detach(a_fx* fx);
ASTERA_API void    a_fx_slot_update(a_fx_slot* slot);
ASTERA_API void    a_fx_slot_destroy(a_fx_slot* slot);

ASTERA_API a_fx a_fx_create(a_fx_type type, void* data);
ASTERA_API void a_fx_update(a_fx fx);
ASTERA_API void a_fx_destroy(a_fx fx);

ASTERA_API a_filter a_filter_create(a_filter_type type, float gain, float hf,
                                    float lf);
ASTERA_API void     a_filter_update(a_filter filter);
ASTERA_API void     a_filter_destroy(a_filter filter);

ASTERA_API uint32_t a_get_device_name(char* dst, int capacity);

ASTERA_API int8_t a_layer_add_music(uint32_t id, a_music* music);
ASTERA_API int8_t a_layer_add_sfx(uint32_t id, a_sfx* sfx);

// well, I need to convert this bit too lol, refactoring can be a bitch
ASTERA_API a_buf a_buf_create(asset_t* asset);
ASTERA_API a_buf* a_buf_get(const char* name);
ASTERA_API void   a_buf_destroy(a_buf buffer);

ASTERA_API a_sfx* a_play_sfxn(const char* name, a_req* req);
ASTERA_API a_sfx* a_play_sfx(a_buf* buff, a_req* req);

ASTERA_API a_req a_req_create(uint16_t layer, vec2 pos, float gain,
                              int8_t loop);

ASTERA_API int  a_ctx_create(const char* device_name);
ASTERA_API void a_destroy_context(void);

ASTERA_API a_music* a_music_create(asset_t* asset, a_meta* meta, a_req* req);
ASTERA_API void     a_music_reset(a_music* music);
ASTERA_API void     a_music_destroy(a_music* music);

ASTERA_API time_s a_music_len(a_music* music);
ASTERA_API time_s a_music_time(a_music* music);

ASTERA_API void a_music_play(a_music* music);
ASTERA_API void a_music_stop(a_music* music);
ASTERA_API void a_music_resume(a_music* music);
ASTERA_API void a_music_pause(a_music* music);

#ifdef __cplusplus
}
#endif
#endif

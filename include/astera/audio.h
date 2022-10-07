#ifndef ASTERA_ASTERA_HEADER
#define ASTERA_ASTERA_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__APPLE__)
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <alc.h>
#include <al.h>
#endif

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#include <stdint.h>

#if defined(ASTERA_LOWP_TIME)
typedef float time_s;
#else
typedef double time_s;
#endif

typedef float a_vec3[3];

typedef struct {
  /* gain - the gain of the listener */
  float gain;
  /* position - the position in 3D space of the listener
   * orientation - the euler rotation of the listener in 3D space
   * velocity - the velocity the listener is moving at
   * _ori - private representation of orientation for OpenAL (vec3: at, up) */
  a_vec3 position, orientation, velocity;
  float  _ori[6];
} a_listener;

typedef struct {
  /* id - the context specific ID */
  uint16_t id;
  /* buf - the OpenAL buffer ID */
  uint32_t buf;
  /* channels - the # of channels the buffer uses */
  uint16_t channels;
  /* length - the length in samples the buffer */
  uint32_t length;
  /* sample_rate - the samples per second that the buffer uses */
  uint32_t sample_rate;
} a_buf;

typedef enum {
  FX_NONE   = 0x0000,
  FX_REVERB = 0x0001,
  FX_EQ     = 0x000C,
} a_fx_type;

typedef struct {
  float  density;               /* [0.0, 1.0],   default 1.0 */
  float  diffusion;             /* [0.0, 1.0],   default 1.0 */
  float  gain;                  /* [0.0, 1.0],   default 0.32 */
  float  gainhf;                /* [0.0, 1.0],   default 0.89 */
  float  decay;                 /* [0.1, 20.0],  default 1.49 */
  float  decay_hfratio;         /* [0.1, 2.0],   default 0.83 */
  float  refl_gain;             /* [0.0, 3.16],  default 0.05 */
  float  refl_delay;            /* [0.0, 0.3],   default 0.007 */
  float  late_gain;             /* [0.0, 10.0],  default 1.26 */
  float  late_delay;            /* [0.0, 0.1],   default 0.011 */
  float  air_absorption_gainhf; /* [0.892, 1.0], default 0.994 */
  float  room_rolloff_factor;   /* [0.0, 10.0],  default: 0.0 */
  int8_t decay_hflimit;         /* [0, 1],       default 1 */
} a_fx_reverb;

typedef struct {
  float low_gain;    /* default: 1        [0.126, 7.943] */
  float low_cutoff;  /* hz, default: 200  [50.0, 800.0] */
  float mid1_gain;   /* default: 1        [0.126, 7.943] */
  float mid1_center; /* hz, default: 500  [200.0, 3000.0] */
  float mid1_width;  /* default: 1        [0.01, 1.0] */
  float mid2_gain;   /* default: 1        [0.126, 7.943] */
  float mid2_center; /* hz, default: 3000 [1000.0, 8000.0] */
  float mid2_width;  /* default: 1        [0.01, 1.0] */
  float high_gain;   /* default 1.0       [0.126, 7.943] */
  float high_cutoff; /* hz, default: 6000 [4000.0 - 16000.0] */
} a_fx_eq;

typedef struct {
  uint16_t id;

  /* AL effect ID */
  uint32_t effect_id;
  /* AL auxiliary effect slot ID */
  uint32_t slot_id;

  float gain;

  a_fx_type type;
  void*     data;
} a_fx;

typedef enum {
  FILTER_LOW  = 0x0001,
  FILTER_HIGH = 0x0002,
  FILTER_BAND = 0x0003,
} a_filter_type;

typedef struct {
  uint16_t      id;
  uint32_t      al_id;
  a_filter_type type;
  float         gain;
  union {
    struct {
      float gainhf;
    } low;
    struct {
      float gainlf;
    } high;
    struct {
      float gainlf;
      float gainhf;
    } band;
  } data;
} a_filter;

typedef struct {
  /* FIXED:
   * these variables are only used at initialization
   * fx - the list of effects to use with the song/sound
   * filters - a list of filters to use with the song/sound
   * fx_count - the number of fx to use */
  uint16_t *fx, *filters;
  uint16_t  fx_count, filter_count;

  /* DYNAMIC:
   * these variables affect the song / sound in real time

   * position - the position to set the sound/song
   * velocity - the velocity to set the sound/song (doesn't move it)
   * gain - the gain of the sound/song
   * range - the range of the sound/song */
  a_vec3 position, velocity;
  float  gain, range;

  /* max_loop - the max amount of times the sound/song can loop */
  uint16_t max_loop;

  /* loop - if the sound/song should loop (1 = true, 0 = false)
   * stop - if the sound/song should stop (1 = true, 0 = false)
   * destroy - if the sound/song should be destroyed (1 = true, 0 = false) */
  uint8_t loop;
  uint8_t stop, destroy;

  /* OBSERVATIONAL:
   * these variables are updated in real time for observation
   * loop_count - the amount of times the sound/song has looped
   * time - the current time in the sound/song
   * state - the current state of the sound/song */
  uint16_t loop_count;
  time_s   time;
  int8_t   valid;
  int32_t  state;
} a_req;

typedef struct {
  /* id - the ID for this song in an a_ctx */
  uint16_t id;

  /* format - the OpenAL format of the song
   * channels - the number of channels in the song */
  int32_t format, channels;

  /* source - the OpenAL source for the song
   * buffers - the OpenAL buffers for the song
   * buffer_count - the number of OpenAL Buffers
   * gain - the gain of the song */
  uint32_t  source;
  uint32_t* buffers;
  uint32_t* buffer_sizes;
  uint8_t   buffer_count;

  /* delta - the realtime position of the song
   * length - the length of the song
   * curr - the current offset before playing buffer */
  time_s delta, length, curr;

  /* vorbis - the STB_Vorbis handle for OGG Decoding
   * info - the stb vorbis info handle */
  stb_vorbis*     vorbis;
  stb_vorbis_info info;

  /* sample_count - the total number of samples in the vorbis stream
   * sample_offset - the overall offset of the buffer */
  uint32_t sample_count, sample_offset;

  /* packets_per_buffer - the amount of packets to decode per buffer */
  uint16_t packets_per_buffer;

  /* data - the raw data of the OGG Vorbis track */
  uint8_t* data;

  /* req - where & how to play the song */
  a_req* req;

  /* loop - if the song should loop or not */
  uint8_t loop;
} a_song;

typedef struct {
  /* id - the id / index of this sfx in an a_ctx */
  uint16_t id;

  /* buffer - the OpenAL Buffer attached to this slot
   * source - the OpenAL source attached to this slot */
  uint32_t buffer, source;

  /* length - the length in samples */
  uint32_t length;

  /* req - the request attached */
  a_req* req;
} a_sfx;

typedef enum {
  EASE_NONE = 0,
  EASE_IN,
  EASE_OUT,
  EASE_EASE,
} a_keyframe_ease;

/* A key pair value with attributes */
typedef struct {
  /* time - the time in ms of this keyframe */
  float time;
  /* value - the value that should be at this keyframe */
  float value;
  /* ease - how to interpolate towards/away from this keyframe */
  a_keyframe_ease ease;
} a_keyframe;

/* A struct for holding a series of keyframes */
typedef struct {
  /* keyframes - all the data of the keyframe timeline */
  a_keyframe* keyframes;
  /* keyframe_count - the # of keyframes in this timeline */
  uint32_t keyframe_count;
} a_timeline;

/* A struct for viewing/moving around a timeline without copying all of the
 * timeline data */
typedef struct {
  /* time - the current time in ms
   * value - the current value calculated from the keyframes */
  float time;
  float value;

  /* current_index - the current index within keyframes the timeline is at */
  uint32_t current_index;
  /* timeline - the source data of keyframes we reference */
  a_timeline* timeline;

  /* output - where to apply the current value */
  float* output;
} a_timeline_view;

typedef struct {
  /* id - the context specific ID of this layer (used to reference it) */
  uint16_t id;

  /* name - the name of this layer */
  const char* name;

  /* sfx - the sfx IDs for this layer
   * songs - the song IDs for this layer */
  uint32_t *sfx, *songs;

  /* sfx_count - the # of sfx currently in the layer
   * sfx_capacity - the max # of sfx allowed in this layer */
  uint32_t sfx_count, sfx_capacity;

  /* song_count - the # of songs currently in the layer
   * song_capacity - the max # of songs allowed in this layer */
  uint32_t song_count, song_capacity;

  /* gain - gain that affects all sfx/songs in this layer */
  float gain;
} a_layer;

/* Structure for creating an audio context with parameters */
typedef struct {
  /* device - device to target (0/null = OS default) */
  const char* device;
  /* max_layers - max number of layers sfx/songs are assignable to */
  uint8_t max_layers;
  /* max_buffers - max # of buffers (sounds) you can have at one time */
  uint16_t max_buffers;
  /* max_buffers - max # of sfx that can be playing at one time */
  uint16_t max_sfx;
  /* max_songs - max # of songs that can be playing at one time */
  uint16_t max_songs;
  /* max_filters - max # of audio filters that can be used at once */
  uint16_t max_filters;
  /* max_fx - max # of audio effects that can be used at once */
  uint16_t max_fx;
  /* max_mono_sources - max # of mono sources that can exist at once */
  uint16_t max_mono_sources;
  /* max_stereo_sources - max # of stereo sources that can exist at once */
  uint16_t max_stereo_sources;
  /* pcm_size - size of the PCM buffer for updating songs (bigger the better) */
  uint32_t pcm_size;
} a_ctx_info;

/* See audio.c for a_ctx definition */
typedef struct a_ctx a_ctx;

/* Quick set a_vec3 variable values */
void a_vec3_set(a_vec3 dst, float x, float y, float z);

/* Print various info about the OpenAL EFX Extension capabilities on this
 * machine */
void a_efx_info(a_ctx* ctx);

/* Get the name of the current audio device
 * ctx - the context to check
 * string_length - a pointer to set to the length of the returned string
 * returns: a pointer to the string */
const char* a_ctx_get_device(a_ctx* ctx, uint8_t* string_length);

/* Check if the context is allowing playback
 * returns 1 = yes, 0 = no */
uint8_t a_can_play(a_ctx* ctx);

/* Get a  default formatted context info for creating a context
 * returns: Formatted a_ctx_info struct with defaults */
a_ctx_info a_ctx_info_default(void);

/* Create an audio context for playback
 * ctx_info - the parameters/info needed to create the context requested
 * Returns: allocated audio context structure used for playback */
a_ctx* a_ctx_create(a_ctx_info ctx_info);

/* Destroy the Audio Context & all of its contents */
uint8_t a_ctx_destroy(a_ctx* ctx);

/* Update the Audio Context
 * ctx - the context to update */
void a_ctx_update(a_ctx* ctx);

/* Queue up a SFX to play
 * ctx - the context to play the SFX within
 * layer - a layer to use to manage this sfx (optional, 0 for none)
 * buf_id - the audio buffer ID of the sound data
 * req - the request callback for specifics of where / how to play the sfx
 * returns: the ID of the sfx (non-zero, 0 = error) */
uint16_t a_sfx_play(a_ctx* ctx, uint16_t layer, uint16_t buf_id, a_req* req);

/* Stops and removes the SFX from its slot
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_stop(a_ctx* ctx, uint16_t sfx_id);

/* Pause a SFX in its slot
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_pause(a_ctx* ctx, uint16_t sfx_id);

/* Resume a paused SFX
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_resume(a_ctx* ctx, uint16_t sfx_id);

/* Create a song
 * ctx - the context to create the song within
 * data - the raw (OGG Vorbis) file data
 * length - the length of the raw data
 * packets_per_buffer - the amount of packets to put into a buffer
 * buffers - the number of OpenAL buffers to use
 * returns: the song ID (non-zero = success, 0 = fail) */
uint16_t a_song_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                       const char* name, uint16_t packets_per_buffer,
                       uint8_t buffers, uint32_t max_buffer_size);

/* Create a song from filesystem
 * ctx - the context to create the song within
 * file_path - the filepath to get the song data from
 * packets_per_buffer - the amount of packets to put into a buffer
 * buffers - the number of OpenAL buffers to use
 * data_ptr - a pointer to hold buffer created from getting file from disk
 * returns: the song ID (non-zero = success, 0 = fail) */
uint16_t a_song_create_fs(a_ctx* ctx, const char* file_path, const char* name,
                          uint16_t packets_per_buffer, uint8_t buffers,
                          uint32_t max_buffer_size, unsigned char** data_ptr);

/* Destroy a song & its contents
 * ctx - the context the song is contained within
 * id - the ID of the song from creation */
uint8_t a_song_destroy(a_ctx* ctx, uint16_t id);

/* Get the pointer of a song
 * ctx - the context containing the song
 * song_id - the song ID
 * returns: a pointer to where the song is kept in the context */
a_song* a_song_get_ptr(a_ctx* ctx, uint16_t song_id);

/* Play a song
 * ctx - the context the song is within
 * layer_id - the layer to play the song on (optional, 0 for none)
 * song_id - the song ID from its creation
 * req - the request on where / how to play the song */
uint8_t a_song_play(a_ctx* ctx, uint16_t layer_id, uint16_t song_id,
                    a_req* req);

/* Stop a song & reset it
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_song_stop(a_ctx* ctx, uint16_t song_id);

/* Pause a song
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_song_pause(a_ctx* ctx, uint16_t song_id);

/* Reume a song from a paused state
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_song_resume(a_ctx* ctx, uint16_t song_id);

/* Get the time of a song
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: time (milliseconds), -1 for fail */
time_s a_song_get_time(a_ctx* ctx, uint16_t song_id);

/* Get the number of samples in a song (total)
 * ctx - the context that contains the song
 * song_id - the song ID
 * returns: the total number of samples in the song */
uint32_t a_song_get_sample_count(a_ctx* ctx, uint16_t song_id);

/* Get the duration / length of a song in milliseconds
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: time (milliseconds), -1 for fail */
time_s a_song_get_length(a_ctx* ctx, uint16_t song_id);

/* Set a song to play from a given time
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * from_start - the time (in milliseconds) from the start of the song
 * returns: success = 1, fail = 0 */
uint8_t a_song_set_time(a_ctx* ctx, uint16_t song_id, time_s from_start);

/* Reset a song (time & state)
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_song_reset(a_ctx* ctx, uint16_t song_id);

/* Get the state of a song (stop, play, pause)
 * ctx - the context containing the song
 * song_id - the ID of the song returned on creation
 * returns: song's state (OpenAL Enum) */
uint32_t a_song_get_state(a_ctx* ctx, uint16_t song_id);

/* Find the ID of a song based on its name
 * returns: song's ID */
uint16_t a_song_get_id(a_ctx* ctx, const char* name);

/* Create an audio buffer (generally small sounds)
 * ctx - the context to manage the buffer with
 * data - the raw data of the buffer
 * data_length - the length of the raw data
 * name - a string name for the buffer (optional)
 * is_ogg - if you want to decode using OGG or WAV format (1 = ogg, 0 = wav)
 * returns: ID of the buffer in the context (non-zero, 0 = fail) */
uint16_t a_buf_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                      const char* name, uint8_t is_ogg);

/* Create an audio buffer (generally small sounds) from file
 * ctx - the context to manage the buffer with
 * file_path - the filepath of the audio file to load in
 * name - a string name for the buffer (optional)
 * is_ogg - if you want to decode using OGG or WAV format (1 = ogg, 0 = wav)
 * returns: ID of the buffer in the context (non-zero, 0 = fail) */
uint16_t a_buf_create_fs(a_ctx* ctx, const char* file_path, const char* name,
                         uint8_t is_ogg);

/* Destroy an audio buffer
 * ctx - the context that contains the audio buffer
 * buf_id - the ID of the buffer returned on the creation
 * returns: success = 1, fail = 0 */
uint8_t a_buf_destroy(a_ctx* ctx, uint16_t buf_id);

/* Find an audio buffer by name
 * ctx - the context that contains the audio buffer
 * name - the name of the audio buffer */
uint16_t a_buf_get(a_ctx* ctx, const char* name);

/* Get a buffer slot by ID
 * ctx - the context that contains the audio buffer
 * id  - the buffer's ID
 * returns: pointer to the buffer */
a_buf* a_buf_get_id(a_ctx* ctx, uint16_t id);

/* Get any open buffer slot
 * ctx - the context
 * returns: pointer to an open slot */
a_buf* a_buf_get_open(a_ctx* ctx);

/* Get the listener's gain
 * ctx - the context of the listener
 * returns: the listener's gain */
float a_listener_get_gain(a_ctx* ctx);

/* Get the listener position
 * ctx - the context to check
 * dst - the vector to store the data in */
void a_listener_get_pos(a_ctx* ctx, a_vec3 dst);

/* Get the listener orientation
 * ctx - the context to check
 * dst - the array to store the data in
 *       (minimum size of 6 floats, at[3],up[3]) */
void a_listener_get_ori(a_ctx* ctx, float dst[6]);

/* Get the listener velocity
 * ctx - the context to check
 * dst - the vector to store the data in */
void a_listener_get_vel(a_ctx* ctx, a_vec3 dst);

/* Set the listener's gain
 * ctx - the context of the listener
 * gain - the gain to set it to */
void a_listener_set_gain(a_ctx* ctx, float gain);

/* Set the current listener position
 * ctx - the context to affect
 * position - the position to change to */
void a_listener_set_pos(a_ctx* ctx, a_vec3 position);

/* Set the listener's orientation
 * ctx - the context to affect
 * position - the orientation to change to */
void a_listener_set_ori(a_ctx* ctx, float ori[6]);

/* Set the listener velocity
 * ctx - the context to affect
 * velocity - the velocity to change to
 * NOTE: This doesn't move the listener just sets the velocity effects */
void a_listener_set_vel(a_ctx* ctx, a_vec3 velocity);

/* Create a request structure
 * position - the position in 3D space to play the song/sound
 * gain - the gain to apply to this song/sound
 * range - the range of the song/sound, pass 0 to set to default
 * loop - whether or not the song/sound should loop
 * fx - a list of effect IDs to use
 * fx_count - the number of effects to use
 * filters - a list of filter IDs to use
 * filter_count - the number of filters to use
 * returns: formatted request structure */
a_req a_req_create(a_vec3 position, float gain, float range, uint8_t loop,
                   uint16_t* fx, uint16_t fx_count, uint16_t* filters,
                   uint16_t filter_count);

/* Use an effect within a context
 * ctx - the context to use
 * type - the type of effect to create
 * data - a pointer to the effect data
 * returns: the ID of the effect */
uint16_t a_fx_create(a_ctx* ctx, a_fx_type type, void* data);

/* Remove an effect & free up the slot
 * ctx - the context to affect
 * fx_id - the ID of the effect
 * returns: success = 1, fail = 0 */
uint8_t a_fx_destroy(a_ctx* ctx, uint16_t fx_id);

/* Update an effect
 * ctx - the context containing the effect
 * fx_id - the ID of the effect
 * returns: success = 1, fail = 0 */
uint8_t a_fx_update(a_ctx* ctx, uint16_t fx_id);

/* Get the type of effect type given an ID
 * ctx - the context containing the effect
 * fx_id - the ID of the fx
 * returns: the FX type `a_fx_type`, 0 = NONE */
a_fx_type a_fx_get_type(a_ctx* ctx, uint16_t fx_id);

/* Get an effect's slot based on ID
 * ctx - the context containing the effect
 * fx_id - the ID of the effect
 * returns: a_fx* pointer to the slot */
a_fx* a_fx_get_slot(a_ctx* ctx, uint16_t fx_id);

/* Create a the default preset of the reverb effect
 *               (see struct definition for values) */
a_fx_reverb a_fx_reverb_default(void);

/* Create an instance of the reverb effect
 * returns: the formatted reverb effect structure */
a_fx_reverb a_fx_reverb_create(float density, float diffusion, float gain,
                               float gainhf, float decay, float decay_hfratio,
                               float refl_gain, float refl_delay,
                               float late_gain, float late_delay,
                               float air_absorption_gainhf,
                               float room_rolloff_factor, int8_t decay_hflimit);

/* Create a default preset of the EQ effect
 *       (see struct definition for values) */
a_fx_eq a_fx_eq_default(void);

/* Create an instance of the EQ effect
 * returns: the formatted EQ effect structure */
a_fx_eq a_fx_eq_create(float low_gain, float low_cutoff, float mid1_gain,
                       float mid1_center, float mid1_width, float mid2_gain,
                       float mid2_center, float mid2_width, float high_gain,
                       float high_cutoff);

/* Create a filter for use in OpenAL
 * ctx - the context to create the filter within
 * type - the type of filter to use
 * gain - the gain of the filter
 * hf - the high frequency band to use
 * lf - the low frequency band to use
 * returns: the formatted filter structure
 *
 * NOTE: you only have to use the HF / LF if the type of filter you're using
 * requires them otherwise the values are ignored */
uint16_t a_filter_create(a_ctx* ctx, a_filter_type type, float gain, float hf,
                         float lf);

/* Update a filter's values
 * ctx - the context the filter is within
 * filter_id - the ID of the filter returned on creation */
uint8_t a_filter_update(a_ctx* ctx, uint16_t filter_id);

/* Remove a filter from usage & free up its slot
 * ctx - the context containing the filter
 * filter_id - the ID of the filter returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_filter_destroy(a_ctx* ctx, uint16_t filter_id);

/* Get a direct pointer to the slot of a filter
 * ctx - the context containing the filter
 * filter_id - the ID of the filter returned on creation
 * returns: the filter slot */
a_filter* a_filter_get_slot(a_ctx* ctx, uint16_t filter_id);

/* Create a layer to hold resources and modify them in groups
 * ctx - the audio context to use
 * name - the name of the layer
 * max_sfx - the max amount of sfx to hold in the layer
 * max_songs - the max amount of songs to hold in the layer
 * returns: layer_id (non-zero, 0 = fail) */
uint16_t a_layer_create(a_ctx* ctx, const char* name, uint32_t max_sfx,
                        uint32_t max_songs);

/* Get the layer ID of a layer by name
 * ctx - the context to check
 * name - the name of the layer
 * returns: layer_id (non-zero, 0 = fail) */
uint16_t a_layer_get_id(a_ctx* ctx, const char* name);

/* Destroy a layer (NOTE: will not free resources in the layer)
 * ctx - the context to check
 * layer_id - the id of the layer
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_destroy(a_ctx* ctx, uint16_t layer_id);

/* Add an sfx to a layer
 * ctx - the context to use
 * layer_id - the ID of the layer to modify
 * sfx_id - the ID of the sfx to add
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_add_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id);

/* Add a song to a layer
 * ctx - the context to use
 * layer_id - the ID of the layer to modify
 * song_id - the ID of the song to add
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_add_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id);

/* Remove an sfx from a layer
 * ctx - the context to use
 * layer_id - the ID of the layer to modify
 * song_id - the ID of the song to add
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_remove_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id);

/* Remove a song from a layer
 * ctx - the context to use
 * layer_id - the ID of the layer to modify
 * song_id - the ID of the song to add
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_remove_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id);

/* Set the gain of the layer & all its resources
 * ctx - the context containing the layer
 * layer_id - the ID of the layer to affect
 * gain - the gain of the layer
 * return: 1 = success, 0 = fail */
uint8_t a_layer_set_gain(a_ctx* ctx, uint16_t layer_id, float gain);

/* Get the gain of a layer
 * ctx - the context containing the layer
 * layer_id - the ID of the layer
 * returns: the gain of the layer, -1.f if not found */
float a_layer_get_gain(a_ctx* ctx, uint16_t layer_id);

/* Create a keyframe timeline
 * times - the timestamp of each keyframe
 * values - the value of each keyframe
 * eases - how each keyframe should ease
 * count - the number of keyframes
 * returns: a keyframe timeline */
a_timeline a_timeline_create(float* times, float* values,
                             a_keyframe_ease* eases, uint32_t count);

/* Free all the data from the timeline */
void a_timeline_destroy(a_timeline* timeline);

/* Create a view of a timeline
 * timeline - timeline data to follow
 * returns: view of a timeline */
a_timeline_view a_timeline_create_view(a_timeline* timeline);

/* Get the current value from a timeline view
 * view - the current view of a timeline
 * returns: value based on current time within the timeline */
float a_timeline_get_value(a_timeline_view* view);

/* Update a timeline view
 * view - the current view of a timeline to update
 * dt - the delta time to change */
void a_timeline_update(a_timeline_view* view, float dt);

/* Set a timeline view's time
 * view - the view of a timeline to update
 * time - the time in ms to set the view to */
void a_timeline_set_time(a_timeline_view* view, float time);

/* Set where to output the value of a timeline view when updated
 * view - timeline view to output current values from
 * output - pointer to update with values */
void a_timeline_set_output(a_timeline_view* view, float* output);

/* Calculate a given value from a specific time
 * timeline - timeline to follow/calculate from
 * time - the time in ms to calculate the value at
 * returns: calculated value from the timeline */
float a_timeline_calc_value_at(a_timeline* timeline, float time);

/* Reset the time of a timeline and values along with it */
void a_timeline_reset(a_timeline_view* view);

#ifdef __cplusplus
}
#endif

#endif

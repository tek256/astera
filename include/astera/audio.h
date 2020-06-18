// POSSIBLE CHANGES
// TODO maybe patch to automatically track/delete all effects created
//       (ref: a_exit)
// Linux: You probably want pulseaudio-devel, alsa-devel, and alsa-oss, maybe
// SDL2 for that audio backend
// TODO Filters
// TODO Harden decoding of Vorbis/OGG (maybe ask Sean)?

#ifndef ASTERA_ASTERA_HEADER
#define ASTERA_ASTERA_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/sys.h>
#include <astera/linmath.h>
#include <stdint.h>

#define AL_LIBTYPE_STATIC
#include <AL/alc.h>
#include <AL/al.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#if !defined(ASTERA_SONG_MAX_FAILS)
#define ASTERA_SONG_MAX_FAILS 3
#endif

#if !defined(ASTERA_DEFAUT_SFX_RANGE)
#define ASTERA_DEFAUT_SFX_RANGE 20
#endif

typedef struct {
  float gain;
  vec3  position, orientation, velocity;
  float _ori[6];
} a_listener;

typedef struct {
  uint16_t id;
  uint32_t buf;
  uint16_t channels;
  uint32_t length;
  uint32_t sample_rate;
} a_buf;

typedef enum {
  NONE   = 0x0000,
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

typedef struct {
  uint16_t id;

  uint32_t effect_id;
  uint32_t slot_id;

  float gain;

  a_fx_type type;
  void*     data;
} a_fx;

typedef enum {
  LOW  = 0x0001,
  HIGH = 0x0002,
  BAND = 0x0003,
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
  // FIXED:
  // these variables are only used at initialization

  // fx - the list of effects to use with the song/sound
  // filters - a list of filters to use with the song/sound
  // fx_count - the number of fx to use
  uint16_t *fx, *filters;
  uint16_t  fx_count, filter_count;

  // REALTIME:
  // these variables are updated in realtime
  // DYNAMIC:
  // these variables affect the song / sound in real time

  // position - the position to set the sound/song
  // velocity - the velocity to set the sound/song (doesn't move it)
  // gain - the gain of the sound/song
  // range - the range of the sound/song
  vec3  position, velocity;
  float gain, range;

  // max_loop - the max amount of times the sound/song can loop
  uint16_t max_loop;

  // loop - if the sound/song should loop (1 = true, 0 = false)
  // stop - if the sound/song should stop (1 = true, 0 = false)
  uint8_t loop;
  uint8_t stop;

  // OBSERVATIONAL:
  // these variables are updated in real time for observation

  // loop_count - the amount of times the sound/song has looped
  // time - the current time in the sound/song
  // state - the current state of the sound/song
  uint16_t loop_count;
  time_s   time;
  uint8_t  state, valid;
} a_req;

typedef struct {
  // id - the ID for this song in an a_ctx
  uint16_t id;

  // format - the OpenAL format of the song
  // channels - the number of channels in the song
  int32_t format, channels;

  // source - the OpenAL source for the song
  // buffers - the OpenAL buffers for the song
  // buffer_count - the number of OpenAL Buffers
  // gain - the gain of the song
  uint32_t  source;
  uint32_t* buffers;
  uint32_t* buffer_sizes;
  uint8_t   buffer_count;

  // delta - the realtime position of the song
  // length - the length of the song
  // curr - the current offset before playing buffer
  time_s delta, length, curr;

  // vorbis - the STB_Vorbis handle for OGG Decoding
  // info - the stb vorbis info handle
  stb_vorbis*     vorbis;
  stb_vorbis_info info;

  // sample_count - the total number of samples in the vorbis stream
  // sample_offset - the overall offset of the buffer
  uint32_t sample_count, sample_offset;

  // packets_per_buffer - the amount of packets to decode per buffer
  uint16_t packets_per_buffer;

  // data - the raw data of the OGG Vorbis track
  uint8_t* data;

  // req - where & how to play the song
  a_req* req;

  // loop - if the song should loop or not
  uint8_t loop;
} a_song;

typedef struct {
  // id - the id / index of this sfx in an a_ctx
  uint16_t id;

  // buffer - the OpenAL Buffer attached to this slot
  // source - the OpenAL source attached to this slot
  uint32_t buffer, source;

  // length - the length in samples
  uint32_t length;

  // req - the request attached
  a_req* req;
} a_sfx;

typedef struct {
  // ID is also considered it's index
  uint16_t id;

  float gain;

  a_sfx**  sfx;
  a_song** songs;

  uint16_t sfx_count, sfx_capacity;
  uint16_t song_count, song_capacity;

  uint8_t gain_change;
} a_layer;

// TODO define more errors for accurate feedback / debugging
typedef enum {
  ASTERA_NO_ERROR = 0,
} a_error;

typedef struct {
  // context - the OpenAL-Soft Context
  // device - the device OpenAL-Soft is using
  ALCcontext* context;
  ALCdevice*  device;

  // listener - the listener values for OpenAL
  a_listener listener;

  // songs - the list of songs
  // song_names - a list of song names
  // song_count - the current amount of songs
  // song_capacity - the max amount of songs
  // song_high - the high water mark for songs in the song list
  a_song*      songs;
  const char** song_names;
  uint16_t     song_count, song_capacity, song_high;

  // sfx - the list of sound effects (sounds)
  // sfx_count - the current amount of sfx
  // sfx_capacity - the max amount of sfx
  // sfx_high - the high water mark for sfx in the sfx list
  a_sfx*   sfx;
  uint16_t sfx_count, sfx_capacity, sfx_high;

  // layers - the list of audio layers
  // layer_names - a list of names for the layers
  // layer_count - the current amount of layers in the list
  // layer_capacity - the max amount of layers
  // layer_high - the high water mark for layers in the layer list
  a_layer*     layers;
  const char** layer_names;
  uint16_t     layer_count, layer_capacity, layer_high;

  // buffers - the list of audio buffers (sounds / raw data)
  // buffer_names - a list of names for audio buffers in the list
  // buffer_count - the current amount of buffers in the list
  // buffer_capacity - the max amount of buffers
  // buffer_high - the high water mark for buffers in the the list
  a_buf*       buffers;
  const char** buffer_names;
  uint16_t     buffer_count, buffer_capacity, buffer_high;

  // fx_slots - a list of slots for OpenAL Effects
  // fx_count - the amount of effects currently in the list
  // fx_capacity - the max amount of effects
  // fx_high - the high water mark for the effects in the list
  a_fx*    fx_slots;
  uint16_t fx_count, fx_capacity, fx_high;

  a_filter* filter_slots;
  uint16_t  filter_count, filter_capacity, filter_high;

  // max_fx - the OpenAL defined max FX
  // fx_per_source - the OpenAL Defined max FX per source
  uint16_t fx_per_source, max_fx;

  uint8_t resizable; // allow for dynamic resizing of arrays
  uint8_t allow;     // allow playback
  uint8_t use_fx;    // allow effect usage

  // pcm - a buffer for decoding
  // pcm_length - the number of shorts the pcm can hold
  // pcm_index - the index of the last element in the pcm
  uint16_t* pcm;
  uint32_t  pcm_length, pcm_index;

  // error - the last error value set
  int32_t error;
} a_ctx;

/* Print various info about the OpenAL EFX Extension capabilities on this
 * machine */
void a_efx_info(a_ctx* ctx);

/* Get the name of the current audio device
 * ctx - the context to check
 * string_length - a pointer to set to the length of the returned string
 * returns: a pointer to the string */
const char* a_ctx_get_device(a_ctx* ctx, uint8_t* string_length);

/* Change the device OpenAL is targetting TODO Implement this maybe
 * NOTE: This is hard to implement since OpenAL isn't device independent, so
 * it'd require rebuffering of everything before destruction of the initial
 * context aka double memory / streamed memory, OR reaccessing all files, which
 * is a pain in the ass.
uint8_t a_ctx_set_device(a_ctx* ctx, const char* device_name); */

/* Check if the context is allowing playback
 * returns 1 = yes, 0 = no */
uint8_t a_ctx_play_allowed(a_ctx* ctx);

/* Create an audio context for playback
 * device - the device's name to use (NULL for default)
 * layers - the number of layers to create for managing sounds
 * max_sfx - the max amount of sfx for the context to handle
 * max_buffers - the max amount of audio buffers for the context to handle
 * max_fx - the max amount of audio fx for the context to handle
 * max_songs - the max amount of songs for the context to handle
 * pcm_size - the size of the PCM buffer for OGG vorbis decoding */
a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                    uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                    uint16_t max_filters, uint32_t pcm_size);

/* Destroy the Audio Context & all of it's contents */
uint8_t a_ctx_destroy(a_ctx* ctx);

/* Update the Audio Context
 * ctx - the context to update */
void a_ctx_update(a_ctx* ctx);

/* Create a layer to manage various audio resources
 * name - the name of the layer
 * max_sfx - the max amount of sfx for the layer to manage
 * max_songs - the max amount of songs for the layer to manage
 * returns: the ID of the layer (non-zero, 0 = error) */
uint16_t a_ctx_layer_create(a_ctx* ctx, const char* name, uint16_t max_sfx,
                            uint16_t max_songs);

/* Queue up a SFX to play
 * ctx - the context to play the SFX within
 * layer - a layer to use to manage this sfx (optional, 0 for none)
 * buf_id - the audio buffer ID of the sound data
 * req - the request callback for specifics of where / how to play the sfx
 * returns: the ID of the sfx (non-zero, 0 = error) */
uint16_t a_sfx_play(a_ctx* ctx, uint16_t layer, uint16_t buf_id, a_req* req);

/* Removes an SFX from it's slot
 * ctx - the context containing the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_remove(a_ctx* ctx, uint16_t sfx_id);

/* Stops and removes the SFX from it's slot
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_stop(a_ctx* ctx, uint16_t sfx_id);

/* Pause a SFX in it's slot
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_pause(a_ctx* ctx, uint16_t sfx_id);

/* Resume a paused SFX
 * ctx - the context to use to find the sfx
 * sfx_id - the ID of the SFX returned when played
 * returns: success = 1, fail = 0 */
uint8_t a_sfx_resume(a_ctx* ctx, uint16_t sfx_id);

/* Get the ID of a layer by name
 * ctx - the context to search for the layer
 * name - the name of the layer
 * returns: the ID of the layer (non-zero, 0 = error) */
uint16_t a_layer_get_id(a_ctx* ctx, const char* name);

/* Set the gain of a layer & update it's contents to reflect that
 * ctx - the context containing the layer
 * layer_id - the ID of the layer
 * gain - the gain of the layer
 * returns: 1 = success, 0 = fail */
uint8_t a_layer_set_gain(a_ctx* ctx, uint16_t layer_id, float gain);

/* Get the gain of a layer
 * ctx - the context containing the layer
 * layer_id - the ID of the layer
 * returns: the gain of the layer */
float a_layer_get_gain(a_ctx* ctx, uint16_t layer_id);

/* Add a song to be managed by a layer
 * ctx - the context containing both the song and layer
 * layer_id - the ID of the layer returned on creation
 * song_id - the ID of the song returned on creation */
uint8_t a_layer_add_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id);

/* Add a sound effect to be managed by a layer
 * layer_id - the ID of the layer returned on creation
 * sfx_id - the ID of the sound effect returned on creation */
uint8_t a_layer_add_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id);

/* Remove a song from a layer
 * layer_id - the ID of the layer returned on creation
 * song_id - the ID of the song returned on creation */
uint8_t a_layer_remove_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id);

/* Remove a sound effect from a layer
 * layer_id - the ID of the layer returned on creation
 * sfx_id - the ID of the sfx returned on creation */
uint8_t a_layer_remove_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id);

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

/* Destroy a song & it's contents
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
 * song_id - the song ID from it's creation
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
 * from_start - the time (in Milliseconds) from the start of the song
 * returns: success = 1, fail = 0 */
uint8_t a_song_set_time(a_ctx* ctx, uint16_t song_id, time_s from_start);

/* Reset a song (time & state)
 * ctx - the context that contains the song
 * song_id - the ID of the song returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_song_reset(a_ctx* ctx, uint16_t song_id);

/* Get the state of a song (stop, play, pause)
 * ctx - the context containing the song
 * song_id - the ID of the song returned on creation */
ALenum a_song_get_state(a_ctx* ctx, uint16_t song_id);

/* Find the ID of a song based on it's name */
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
void a_listener_get_pos(a_ctx* ctx, vec3 dst);

/* Get the listener orientation
 * TODO document orientation vectors (up, at)
 * ctx - the context to check
 * dst - the array to store the data in (minimum size of 6 floats) */
void a_listener_get_ori(a_ctx* ctx, float dst[6]);

/* Get the listener velocity
 * ctx - the context to check
 * dst - the vector to store the data in */
void a_listener_get_vel(a_ctx* ctx, vec3 dst);

/* Set the listener's gain
 * ctx - the context of the listener
 * gain - the gain to set it to */
void a_listener_set_gain(a_ctx* ctx, float gain);

/* Set the current listener position
 * ctx - the context to affect
 * position - the position to change to */
void a_listener_set_pos(a_ctx* ctx, vec3 position);

/* Set the listener's orientation
 * ctx - the context to affect
 * position - the orientation to change to */
void a_listener_set_ori(a_ctx* ctx, float ori[6]);

/* Set the listener velocity
 * ctx - the context to affect
 * velocity - the velocity to change to
 * NOTE: This doesn't move the listener just sets the velocity effects */
void a_listener_set_vel(a_ctx* ctx, vec3 velocity);

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
a_req a_req_create(vec3 position, float gain, float range, uint8_t loop,
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
 *              (see struct definition for values) */
a_fx_reverb a_fx_reverb_default(void);

/* Create an instance of the reverb effect
 * TODO: Document variable names
 * returns: the formatted reverb effect structure */
a_fx_reverb a_fx_reverb_create(float density, float diffusion, float gain,
                               float gainhf, float decay, float decay_hfratio,
                               float refl_gain, float refl_delay,
                               float late_gain, float late_delay,
                               float air_absorption_gainhf,
                               float room_rolloff_factor, int8_t decay_hflimit);

/* Create an instance of the EQ effect
 * TODO: Document the variable names
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

/* Remove a filter from usage & free up it's slot
 * ctx - the context containing the filter
 * filter_id - the ID of the filter returned on creation
 * returns: success = 1, fail = 0 */
uint8_t a_filter_destroy(a_ctx* ctx, uint16_t filter_id);

/* Get a direct pointer to the slot of a filter
 * ctx - the context containing the filter
 * filter_id - the ID of the filter returned on creation
 * returns: the filter slot */
a_filter* a_filter_get_slot(a_ctx* ctx, uint16_t filter_id);

#ifdef __cplusplus
}
#endif
#endif

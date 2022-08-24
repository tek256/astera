#include <astera/audio.h>
#include <astera/debug.h>

#if defined(__APPLE__)
#if !defined(ASTERA_AL_NO_FX)
#define ASTERA_AL_NO_FX
#endif
#endif

#if !defined(ASTERA_AL_DISTANCE_MODEL)
#define ASTERA_AL_DISTANCE_MODEL AL_INVERSE_DISTANCE
#endif

#if !defined(ASTERA_AL_ROLLOFF_FACTOR)
#define ASTERA_AL_ROLLOFF_FACTOR 1.f
#endif

#undef STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_MAX_CHANNELS 2
#include <stb_vorbis.c>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct a_ctx {
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
  a_sfx*   sfx;
  uint16_t sfx_count, sfx_capacity;

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
  uint16_t fx_count, fx_capacity;

  a_filter* filter_slots;
  uint16_t  filter_count, filter_capacity;

  // fx_per_source - the OpenAL Defined max FX per source
  // max_fx - the OpenAL defined max FX
  uint16_t fx_per_source, max_fx;

  uint8_t resizable; // allow for dynamic resizing of arrays
  uint8_t allow;     // allow playback
  uint8_t use_fx;    // allow effect usage

  // pcm - a buffer for decoding
  // pcm_length - the number of shorts the pcm can hold
  // pcm_index - the index of the last element in the pcm
  uint16_t* pcm;
  uint32_t  pcm_length, pcm_index;

  // layers - array of layers used to manage playback
  // layer_count - the number of layers in the array currently
  // layer_capacity - the max number of layers in array
  a_layer* layers;
  uint16_t layer_count, layer_capacity;

  // error - the last error value set
  int32_t error;
};

#if !defined(ASTERA_AL_NO_FX)
#include <efx.h>

static LPALGENEFFECTS    alGenEffects;
static LPALGENFILTERS    alGenFilters;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALDELETEFILTERS alDeleteFilters;
static LPALISEFFECT      alIsEffect;
static LPALEFFECTI       alEffecti;
static LPALEFFECTIV      alEffectiv;
static LPALEFFECTF       alEffectf;
static LPALEFFECTFV      alEffectfv;
static LPALGETEFFECTI    alGetEffecti;
static LPALGETEFFECTIV   alGetEffectiv;
static LPALGETEFFECTF    alGetEffectf;
static LPALGETEFFECTFV   alGetEffectfv;

static LPALFILTERF alFilterf;
static LPALFILTERI alFilteri;

static LPALGENAUXILIARYEFFECTSLOTS    alGenAuxiliaryEffectSlots;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
static LPALISAUXILIARYEFFECTSLOT      alIsAuxiliaryEffectSlot;
static LPALAUXILIARYEFFECTSLOTI       alAuxiliaryEffectSloti;
static LPALAUXILIARYEFFECTSLOTIV      alAuxiliaryEffectSlotiv;
static LPALAUXILIARYEFFECTSLOTF       alAuxiliaryEffectSlotf;
static LPALAUXILIARYEFFECTSLOTFV      alAuxiliaryEffectSlotfv;
static LPALGETAUXILIARYEFFECTSLOTI    alGetAuxiliaryEffectSloti;
static LPALGETAUXILIARYEFFECTSLOTIV   alGetAuxiliaryEffectSlotiv;
static LPALGETAUXILIARYEFFECTSLOTF    alGetAuxiliaryEffectSlotf;
static LPALGETAUXILIARYEFFECTSLOTFV   alGetAuxiliaryEffectSlotfv;
#endif

static inline float _a_clamp(float value, float min, float max, float def) {
  return (value == -1.f) ? def
         : (value < min) ? min
         : (value > max) ? max
                         : value;
}

/* static float a_sfx_get_source_gain(a_ctx* ctx, uint16_t source) {
  for (uint16_t i = 0; i < ctx->sfx_capacity; ++i) {
    if (ctx->sfx[i].source == source) {
      if (ctx->sfx[i].req) {
        return ctx->sfx[i].req->gain;
      } else {
        return 0;
      }
    }
  }

  return 0;
} */

static a_layer* _a_get_layer(a_ctx* ctx, uint16_t layer_id) {
  return (ctx->layers[layer_id - 1].id == 0) ? 0 : &ctx->layers[layer_id - 1];
}

static uint8_t _a_layer_add(a_layer* layer, uint32_t id, uint8_t is_sfx) {
  if (is_sfx) {
    if (layer->sfx_count == layer->sfx_capacity - 1) {
      ASTERA_FUNC_DBG("no free sfx slots in layer\n");
      return 0;
    }

    for (uint16_t i = 0; i < layer->sfx_capacity; ++i) {
      if (layer->sfx[i] == id) {
        ASTERA_FUNC_DBG("sfx id already in layer\n");
        return 0;
      }

      if (layer->sfx[i] == 0) {
        layer->sfx[i] = id;
        ++layer->sfx_count;
        return 1;
      }
    }
  } else {
    if (layer->song_count == layer->song_capacity - 1) {
      ASTERA_FUNC_DBG("no free song slots in layer\n");
      return 0;
    }

    for (uint16_t i = 0; i < layer->song_capacity; ++i) {
      if (layer->songs[i] == id) {
        ASTERA_FUNC_DBG("song already in layer\n");
        return 0;
      }

      if (layer->songs[i] == 0) {
        layer->songs[i] = id;
        ++layer->song_count;
        return 1;
      }
    }
  }

  return 0;
}

static uint8_t _a_layer_remove(a_layer* layer, uint32_t id, uint8_t is_sfx) {
  if (!layer) {
    return 0;
  }

  uint8_t start = 0;
  if (is_sfx) {
    if (layer->sfx_count == 0) {
      return 0;
    }

    for (uint16_t i = 0; i < layer->sfx_capacity - 1; ++i) {
      if (layer->sfx[i] == id) {
        start = 1;
      }

      if (start) {
        layer->sfx[i] = layer->sfx[i + 1];
      }
    }

    if (start) {
      layer->sfx[layer->sfx_count] = 0;
      --layer->sfx_count;
      return 1;
    }
  } else {
    if (layer->song_count == 0)
      return 0;

    for (uint16_t i = 0; i < layer->song_capacity - 1; ++i) {
      if (layer->songs[i] == id) {
        start = 1;
      }

      if (start) {
        layer->songs[i] = layer->songs[i + 1];
      }
    }

    if (start) {
      layer->songs[layer->song_count] = 0;
      --layer->song_count;
      return 1;
    }
  }

  return 0;
}

// Get the gain of the containing layer for a resource
static float _a_get_layered_gain(a_ctx* ctx, uint32_t id, uint8_t is_sfx) {
  for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
    a_layer* layer = &ctx->layers[i];

    if (layer->id) {
      if (is_sfx && layer->sfx_count > 0) {
        for (uint32_t j = 0; j < layer->sfx_count; ++j) {
          if (layer->sfx[j] == id) {
            return layer->gain;
          }
        }
      } else if (!is_sfx && layer->song_capacity > 0) {
        for (uint32_t j = 0; j < layer->song_count; ++j) {
          if (layer->songs[j] == id) {
            return layer->gain;
          }
        }
      } else {
        continue;
      }
    }
  }

  return -1.f;
}

static float _a_get_gain(a_ctx* ctx, uint32_t id, uint8_t is_sfx) {
  float gain  = 0.f;
  float lgain = _a_get_layered_gain(ctx, id, is_sfx);

  if (is_sfx) {
    a_sfx* sfx = &ctx->sfx[id - 1];
    if (sfx->req) {
      gain = sfx->req->gain;
    } else {
      return -1.f;
    }
  } else {
    a_song* song = &ctx->songs[id - 1];
    if (song->req) {
      gain = song->req->gain;
    } else {
      return -1.f;
    }
  }

  return (lgain != -1.f) ? lgain * gain : gain;
}

static void _a_layer_destroy(a_layer* layer) {
  if (!layer)
    return;

  layer->id            = 0;
  layer->name          = 0;
  layer->song_count    = 0;
  layer->song_capacity = 0;
  layer->sfx_count     = 0;
  layer->sfx_capacity  = 0;

  if (layer->sfx)
    free(layer->sfx);

  if (layer->songs)
    free(layer->songs);
}

static uint8_t _a_song_reset(a_ctx* ctx, a_song* song) {
  song->delta         = 0.f;
  song->curr          = 0.f;
  song->sample_offset = 0;
  song->curr          = 0;

  stb_vorbis_seek_start(song->vorbis);

  // uint32_t error = stb_vorbis_get_error(song->vorbis);

  // Unqueue anything existing
  alSourceStop(song->source);
  alSourceUnqueueBuffers(song->source, song->buffer_count, song->buffers);

  for (uint8_t i = 0; i < song->buffer_count; ++i) {
    uint32_t buffer = song->buffers[i];
    float    buf_time =
        (float)((float)song->buffer_sizes[0] / song->info.sample_rate) * 1000.f;
    song->curr += buf_time;

    memset(ctx->pcm, 0, ctx->pcm_length * sizeof(uint16_t));
    uint32_t pcm_total_length = 0;

    for (uint16_t p = 0; p < song->packets_per_buffer; ++p) {
      uint32_t pcm_remaining = ctx->pcm_length - pcm_total_length;

      if (pcm_remaining < song->info.max_frame_size) {
        break;
      }

      if (pcm_remaining == 0) {
        break;
      }

      uint32_t num_samples = stb_vorbis_get_samples_short_interleaved(
          song->vorbis, song->channels, ctx->pcm + pcm_total_length,
          pcm_remaining);

      if (num_samples > 0) {
        int32_t sample_count = song->channels * num_samples;
        pcm_total_length += sample_count;
      }

      for (uint8_t i = 0; i < song->buffer_count - 1; ++i) {
        song->buffer_sizes[i] = song->buffer_sizes[i + 1];
      }

      song->buffer_sizes[song->buffer_count - 1] = num_samples;
    }

    stb_vorbis_info info = stb_vorbis_get_info(song->vorbis);

    alBufferData(buffer, song->format, ctx->pcm,
                 pcm_total_length * sizeof(uint16_t), info.sample_rate);
    alSourceQueueBuffers(song->source, 1, &buffer);
  }

  return 1;
}

void a_efx_info(a_ctx* ctx) {
  if (alcIsExtensionPresent(ctx->device, "ALC_EXT_EFX") == AL_FALSE) {
    ASTERA_FUNC_DBG("No ALC_EXT_EFX.\n");
  } else {
    ASTERA_FUNC_DBG("ALC_EXT_EFX Present.\n");
  }

  uint32_t s_sends = 0;
#if !defined(ASTERA_AL_NO_FX) && defined(ALC_MAX_AUXILIARY_SENDS)
  s_sends = ALC_MAX_AUXILIARY_SENDS;
#endif
  ASTERA_FUNC_DBG("MAX_AUXILIARY_SENDS: %i\n", s_sends);
}

const char* a_ctx_get_device(a_ctx* ctx, uint8_t* string_length) {
  const ALchar* name;

  if (ctx->device) {
    name = alcGetString(ctx->device, ALC_DEVICE_SPECIFIER);
  } else {
    name = alcGetString(0, ALC_DEFAULT_DEVICE_SPECIFIER);
  }

  uint8_t length = strlen(name);

  if (string_length) {
    *string_length = length;
  }

  return name;
}

uint8_t a_can_play(a_ctx* ctx) {
  return ctx->allow;
}

a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                    uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                    uint16_t max_filters, uint32_t pcm_size) {
  a_ctx* ctx = (a_ctx*)calloc(1, sizeof(a_ctx));

  if (!ctx) {
    ASTERA_FUNC_DBG("unable to malloc initial space for context.\n");
    return 0;
  }

  ALCdevice* al_device = alcOpenDevice(device);

#if !defined(ASTERA_AL_NO_FX)
  if (alcIsExtensionPresent(al_device, "ALC_EXT_EFX") == AL_FALSE) {
    ctx->use_fx = 0;
  } else {
    ctx->use_fx = 1;
  }

  // Disable effects if we're not using any of them
  if (!max_fx && !max_filters) {
    ctx->use_fx = 0;
  }
#else
  ctx->use_fx = 0;
#endif

  int attribs[4] = {0};

#if !defined(ASTERA_AL_NO_FX) && defined(ALC_MAX_AUXILIARY_SENDS)
  if (ctx->use_fx) {
    attribs[0] = ALC_MAX_AUXILIARY_SENDS;
    attribs[1] = 4;
  }
#endif

  ALCcontext* context = alcCreateContext(al_device, attribs);

  if (!alcMakeContextCurrent(context)) {
    ASTERA_FUNC_DBG("Error creating OpenAL Context\n");
    free(ctx);
    return 0;
  }

  ctx->context = context;
  ctx->device  = al_device;

#if defined(ASTERA_AL_DISTANCE_MODEL)
  alDistanceModel(ASTERA_AL_DISTANCE_MODEL);
#endif

#if !defined(ASTERA_AL_NO_FX)
  if (ctx->use_fx) {
    ALint fx_per_source;
    alcGetIntegerv(al_device, ALC_MAX_AUXILIARY_SENDS, 1, &fx_per_source);
    ctx->fx_per_source = (uint16_t)fx_per_source;

    if (!ctx->fx_per_source) {
      ASTERA_FUNC_DBG("0 effects allowed per source, disabling effects.\n");
      ctx->use_fx = 0;
    }
  }

  if (ctx->use_fx) {
#define LOAD_PROC(T, x) ((x) = (T)alGetProcAddress(#x))
    LOAD_PROC(LPALGENEFFECTS, alGenEffects);
    LOAD_PROC(LPALGENFILTERS, alGenFilters);
    LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
    LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
    LOAD_PROC(LPALISEFFECT, alIsEffect);
    LOAD_PROC(LPALEFFECTI, alEffecti);
    LOAD_PROC(LPALEFFECTIV, alEffectiv);
    LOAD_PROC(LPALEFFECTF, alEffectf);
    LOAD_PROC(LPALEFFECTFV, alEffectfv);
    LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
    LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
    LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
    LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);

    LOAD_PROC(LPALFILTERF, alFilterf);
    LOAD_PROC(LPALFILTERI, alFilteri);

    LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
    LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
    LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
    LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
    LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);

    LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
    LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
    LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
    LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
    LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
    LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);

#undef LOAD_PROC

    ctx->fx_capacity = max_fx;
    ctx->fx_count    = 0;

    if (max_fx) {
      ctx->fx_slots = (a_fx*)calloc(ctx->fx_capacity, sizeof(a_fx));

      if (!ctx->fx_slots) {
        ASTERA_FUNC_DBG("unable to allocate %i fx slots\n", ctx->fx_capacity);

        alcDestroyContext(ctx->context);
        alcCloseDevice(ctx->device);

        free(ctx);
        return 0;
      }

      for (uint16_t i = 0; i < ctx->fx_capacity; ++i) {
        ctx->fx_slots[i].id = i + 1;
      }
    } else {
      ctx->fx_slots = 0;
    }

    ctx->filter_count    = 0;
    ctx->filter_capacity = max_filters;

    if (max_filters) {
      ctx->filter_slots =
          (a_filter*)calloc(ctx->filter_capacity, sizeof(a_filter));

      if (!ctx->filter_slots) {
        ASTERA_FUNC_DBG("unable to allocate %i fx slots\n", ctx->fx_capacity);

        alcDestroyContext(ctx->context);
        alcCloseDevice(ctx->device);

        free(ctx->fx_slots);
        free(ctx);

        return 0;
      }

      for (uint16_t i = 0; i < max_filters; ++i) {
        ctx->filter_slots[i].id = i + 1;
      }
    } else {
      ctx->filter_slots = 0;
    }
  }
#endif

  ctx->pcm_length = pcm_size;
  ctx->pcm_index  = 0;
  if (pcm_size) {
    ctx->pcm = (uint16_t*)calloc(pcm_size, sizeof(uint16_t));
  }

  // Create the resource arrays
  ctx->song_capacity = max_songs;
  ctx->song_count    = 0;
  ctx->song_high     = 0;

  if (max_songs) {
    ctx->songs      = (a_song*)calloc(ctx->song_capacity, sizeof(a_song));
    ctx->song_names = (const char**)calloc(ctx->song_capacity, sizeof(char*));

    for (uint16_t i = 0; i < max_songs; ++i) {
      ctx->songs[i].id = i + 1;
      alGenSources(1, &ctx->songs[i].source);
      ctx->songs[i].req = 0;
    }
  }

  ctx->buffer_count    = 0;
  ctx->buffer_capacity = max_buffers;
  ctx->buffer_high     = 0;

  if (max_buffers) {
    ctx->buffers      = (a_buf*)calloc(max_buffers, sizeof(a_buf));
    ctx->buffer_names = (const char**)calloc(max_buffers, sizeof(char*));

    for (uint16_t i = 0; i < max_buffers; ++i) {
      ctx->buffers[i].id = i + 1;
    }
  }

  ctx->sfx_capacity = max_sfx;
  ctx->sfx_count    = 0;

  if (max_sfx) {
    ctx->sfx = (a_sfx*)calloc(ctx->sfx_capacity, sizeof(a_sfx));

    for (uint16_t i = 0; i < max_sfx; ++i) {
      ctx->sfx[i].id = i + 1;
      alGenSources(1, &ctx->sfx[i].source);
      ctx->sfx[i].req = 0;
    }
  }

  ctx->layer_count    = 0;
  ctx->layer_capacity = layers;

  if (layers) {
    ctx->layers = (a_layer*)calloc(layers, sizeof(a_layer));

    for (uint16_t i = 0; i < layers; ++i) {
      ctx->layers[i].gain = 1.f;
    }
  }

  ctx->allow = 1;
  return ctx;
}

uint8_t a_ctx_destroy(a_ctx* ctx) {
  if (!ctx) {
    ASTERA_FUNC_DBG("no context passed to destroy.\n");
    return 0;
  }

  for (uint16_t i = 0; i < ctx->song_capacity; ++i) {
    a_song* song = &ctx->songs[i];

    if (song->vorbis)
      stb_vorbis_close(song->vorbis);

    if (song->buffers)
      free(song->buffers);

    if (song->buffer_sizes)
      free(song->buffer_sizes);
  }

  if (ctx->fx_capacity && ctx->fx_slots) {
    free(ctx->fx_slots);
  }

  if (ctx->filter_capacity && ctx->filter_slots) {
    free(ctx->filter_slots);
  }

  if (ctx->buffers)
    free(ctx->buffers);

  if (ctx->buffer_names)
    free(ctx->buffer_names);

  if (ctx->songs)
    free(ctx->songs);

  if (ctx->song_names)
    free(ctx->song_names);

  if (ctx->sfx)
    free(ctx->sfx);

  if (ctx->pcm)
    free(ctx->pcm);

  if (ctx->layer_capacity && ctx->layers) {
    for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
      _a_layer_destroy(&ctx->layers[i]);
    }

    free(ctx->layers);
  }

  alcCloseDevice(ctx->device);
  alcDestroyContext(ctx->context);

  free(ctx);

  return 1;
}

void a_song_update_decode(a_ctx* ctx, a_song* song) {
  ALenum state;
  ALint  proc;

  alGetSourcei(song->source, AL_SOURCE_STATE, &state);
  alGetSourcei(song->source, AL_BUFFERS_PROCESSED, &proc);

  float sec_offset;
  alGetSourcef(song->source, AL_SEC_OFFSET, &sec_offset);

  song->delta     = song->curr + (sec_offset * 1000.f);
  song->req->time = song->delta;

  if (proc > 0) {
    uint32_t al_error;
    uint32_t buffer, offset = stb_vorbis_get_sample_offset(song->vorbis);

    if (offset >= song->sample_count) {
      if (song->req->loop) {
        stb_vorbis_seek_start(song->vorbis);
      } else {
        return;
      }
    }

    for (uint8_t i = 0; i < proc; ++i) {
      float buf_time =
          (float)((float)song->buffer_sizes[0] / song->info.sample_rate) *
          1000.f;
      song->curr += buf_time;

      memset(ctx->pcm, 0, ctx->pcm_length * sizeof(uint16_t));
      uint32_t pcm_total_length = 0;

      alSourceUnqueueBuffers(song->source, 1, &buffer);
      for (uint16_t p = 0; p < song->packets_per_buffer; ++p) {
        uint32_t pcm_remaining = ctx->pcm_length - pcm_total_length;

        if ((int)pcm_remaining < song->info.max_frame_size) {
          break;
        }

        if (pcm_remaining == 0) {
          break;
        }

        uint32_t num_samples = stb_vorbis_get_samples_short_interleaved(
            song->vorbis, song->channels, ctx->pcm + pcm_total_length,
            pcm_remaining);

        if (num_samples > 0) {
          int32_t sample_count = song->channels * num_samples;
          pcm_total_length += sample_count;
        }

        for (uint8_t i = 0; i < song->buffer_count - 1; ++i) {
          song->buffer_sizes[i] = song->buffer_sizes[i + 1];
        }

        song->buffer_sizes[song->buffer_count - 1] = num_samples;
      }

      stb_vorbis_info info = stb_vorbis_get_info(song->vorbis);

      alBufferData(buffer, song->format, ctx->pcm,
                   pcm_total_length * sizeof(uint16_t), info.sample_rate);
      alSourceQueueBuffers(song->source, 1, &buffer);

      if ((al_error = alGetError()) == AL_INVALID_VALUE) {
        ASTERA_FUNC_DBG("AL Error %i\n", al_error);
      }
    }
  }

  if (proc == song->buffer_count && state == AL_PLAYING) {
    alSourcePlay(song->source);
  }
}

void a_ctx_update(a_ctx* ctx) {
  float gain = 0.f;

  for (uint16_t i = 0; i < ctx->song_capacity; ++i) {
    a_song* song = &ctx->songs[i];
    if (!song)
      continue;

    if (song->req) {
      if (song->req->stop) {
        a_song_stop(ctx, song->id);
        continue;
      }

      if (song->req->destroy) {
        a_song_destroy(ctx, song->id);
        continue;
      }

      ALenum state;
      alGetSourcei(song->source, AL_SOURCE_STATE, &state);

      song->req->state = state;

      if (state == AL_PLAYING) {
        if (song->sample_count == song->sample_offset) {
          if (song->req->loop) {
            _a_song_reset(ctx, song);
          } else {
            alSourceStop(song->source);
            song->req->state = AL_STOPPED;
            continue;
          }
        }

        a_song_update_decode(ctx, song);
        gain = _a_get_gain(ctx, song->id, 0);
        alSourcef(song->source, AL_GAIN, gain);
        alSource3f(song->source, AL_POSITION, song->req->position[0],
                   song->req->position[1], song->req->position[2]);
        alSource3f(song->source, AL_VELOCITY, song->req->velocity[0],
                   song->req->velocity[1], song->req->velocity[2]);
        alSourcef(song->source, AL_MAX_DISTANCE, song->req->range);
      }
    }
  }

  for (uint16_t i = 0; i < ctx->sfx_capacity; ++i) {
    a_sfx* sfx = &ctx->sfx[i];

    if (!sfx)
      continue;

    if (!sfx->buffer || !sfx->req || !sfx->source) {
      continue;
    }

    ALenum state;
    alGetSourcei(sfx->source, AL_SOURCE_STATE, &state);

    int8_t remove =
        (state != AL_PLAYING && sfx->req->time > 0.f) || sfx->req->stop;

    if (remove) {
      for (uint16_t j = 0; j < ctx->layer_capacity; ++j) {
        _a_layer_remove(&ctx->layers[j], sfx->id, 1);
      }

      --ctx->sfx_count;

      if (sfx->req) {
        sfx->req->valid = 0;
        sfx->req->state = AL_STOPPED;
      }

      sfx->req    = 0;
      sfx->buffer = 0;
      sfx->length = 0;

      alSourceStop(sfx->source);
      alSourcei(sfx->source, AL_BUFFER, 0);

      continue;
    }

    gain = _a_get_gain(ctx, sfx->id, 1);
    alSourcef((ALuint)sfx->source, AL_GAIN, gain);
    alSource3f((ALuint)sfx->source, AL_POSITION, sfx->req->position[0],
               sfx->req->position[1], sfx->req->position[2]);
    alSource3f((ALuint)sfx->source, AL_VELOCITY, sfx->req->velocity[0],
               sfx->req->velocity[1], sfx->req->velocity[2]);
    alSourcef((ALuint)sfx->source, AL_MAX_DISTANCE, sfx->req->range);
    alSourcei((ALuint)sfx->source, AL_LOOPING, sfx->req->loop);

    ALint sample_offset;
    alGetSourcei(sfx->source, AL_SAMPLE_OFFSET, &sample_offset);
    if ((uint32_t)sample_offset >= sfx->length) {
      if (sfx->req->loop)
        ++sfx->req->loop_count;
    }

    alGetSourcef((ALuint)sfx->source, AL_SEC_OFFSET, (ALfloat*)&sfx->req->time);
  }
}

uint16_t a_sfx_play(a_ctx* ctx, uint16_t layer, uint16_t buf_id, a_req* req) {
  if (ctx->sfx_count == ctx->sfx_capacity - 1) {
    ASTERA_FUNC_DBG("no free sfx slots.\n");
    return 0;
  }

  float gain = req->gain;
  if (layer) {
    a_layer* _layer = _a_get_layer(ctx, layer);
    gain *= _layer->gain;
    if (_layer->sfx_count == _layer->sfx_capacity - 1) {
      ASTERA_FUNC_DBG("no free slots in layer\n");
      return 0;
    }
  }

  a_buf* buf = a_buf_get_id(ctx, buf_id);
  if (!buf) {
    ASTERA_FUNC_DBG("unable to find %i\n", buf_id);
    return 0;
  }

  a_sfx* slot = 0;
  for (uint16_t i = 0; i < ctx->sfx_capacity; ++i) {
    if (ctx->sfx[i].source) {
      uint32_t buffer_id;
      alGetSourcei(ctx->sfx[i].source, AL_BUFFER, &buffer_id);
      if (buffer_id == 0) {
        slot = &ctx->sfx[i];
        break;
      }
    } else {
      slot = &ctx->sfx[i];
      break;
    }
  }

  if (!slot) {
    ASTERA_FUNC_DBG("No slots open, really.\n");
    return 0;
  }

  if (!slot->source) {
    alGenSources(1, &slot->source);
  }

  alSourcei(slot->source, AL_BUFFER, buf->buf);

#if !defined(ASTERA_AL_NO_FX)
  // Apply fx
  if (req->fx_count > 0) {
    for (uint16_t i = 0; i < req->fx_count; ++i) {
      // Make sure it's valid within the range of filters
      if (req->fx[i] <= ctx->fx_capacity && req->fx[i] > 0) {
        alSource3i(slot->source, AL_AUXILIARY_SEND_FILTER,
                   (ALint)ctx->fx_slots[req->fx[i] - 1].slot_id, i, 0);
      }
    }
  }

  // Apply filters
  if (req->filter_count > 0) {
    for (uint16_t i = 0; i < req->filter_count; ++i) {
      if (req->filters[i] <= ctx->filter_capacity && req->filters[i] > 0) {
        alSourcei(slot->source, AL_DIRECT_FILTER,
                  ctx->filter_slots[req->filters[i] - 1].al_id);
      }
    }
  }
#endif

  slot->buffer = buf->id;
  slot->length = buf->length;
  slot->req    = req;

  uint32_t size;
  alGetBufferi(buf->buf, AL_SIZE, (ALint*)&size);

  alSourcef(slot->source, AL_GAIN, req->gain);
  alSource3f(slot->source, AL_POSITION, req->position[0], req->position[1],
             req->position[2]);
  alSource3f(slot->source, AL_VELOCITY, req->velocity[0], req->velocity[1],
             req->velocity[2]);
  alSourcei(slot->source, AL_LOOPING, req->loop);

  alSourcePlay(slot->source);

  ++ctx->sfx_count;

  if (layer != 0) {
    _a_layer_add(_a_get_layer(ctx, layer), slot->id, 1);
  }

  return slot->id;
}

uint8_t a_sfx_stop(a_ctx* ctx, uint16_t sfx_id) {
  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  if (sfx->req) {
    sfx->req->valid = 0;
    sfx->req->state = AL_STOPPED;
  }

  if (sfx->source && sfx->buffer) {
    alSourceStop(sfx->source);
  }

  for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
    _a_layer_remove(&ctx->layers[i], sfx->id, 1);
  }

  alSourcei(sfx->source, AL_BUFFER, 0);

  sfx->req    = 0;
  sfx->buffer = 0;
  sfx->length = 0;

  --ctx->sfx_count;

  return 1;
}

uint8_t a_sfx_pause(a_ctx* ctx, uint16_t sfx_id) {
  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  if (sfx->buffer && sfx->source) {
    alSourcePause(sfx->source);
  }

  if (sfx->req) {
    sfx->req->state = AL_PAUSED;
  }

  return 1;
}

uint8_t a_sfx_resume(a_ctx* ctx, uint16_t sfx_id) {
  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  if (sfx->buffer && sfx->source) {
    alSourcePlay(sfx->source);
  }

  return 1;
}

uint16_t a_song_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                       const char* name, uint16_t packets_per_buffer,
                       uint8_t buffers, uint32_t max_buffer_size) {
  if (!data || !data_length || !packets_per_buffer || !buffers ||
      !max_buffer_size) {
    ASTERA_FUNC_DBG("Invalid parameters passed\n");
    return 0;
  }

  a_song* song = 0;

  for (uint16_t i = 0; i < ctx->song_high; ++i) {
    if (!ctx->songs[i].buffers) {
      song               = &ctx->songs[i];
      ctx->song_names[i] = name;
    }
  }

  int8_t new_high = 0;
  if (!song && ctx->song_high < ctx->song_capacity) {
    song                            = &ctx->songs[ctx->song_high];
    ctx->song_names[ctx->song_high] = name;
    new_high                        = 1;
  } else {
    return 0;
  }

  int32_t error;

  song->data   = data;
  song->vorbis = stb_vorbis_open_memory(data, data_length, &error, 0);
  song->req    = 0;
  song->curr   = 0.f;

  if (!song->vorbis) {
    song->data = 0;
    song->req  = 0;

    ASTERA_FUNC_DBG("Unable to load vorbis, that sucks VORBIS Error: %i\n",
                    error);

    free(song->vorbis);

    return 0;
  }

  song->packets_per_buffer = packets_per_buffer;

  song->info = stb_vorbis_get_info(song->vorbis);

  if (max_buffer_size < song->info.max_frame_size) {
    ASTERA_FUNC_DBG("max_buffer_size is smaller than the listed max "
                    "frame size of this OGG file: %i vs %i\n",
                    max_buffer_size, song->info.max_frame_size);

    free(song->vorbis);
    song->data = 0;
    song->req  = 0;

    return 0;
  }

  if (song->info.channels > 2) {
    song->channels = 2;
  }

  song->format   = (song->channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
  song->channels = song->info.channels;

  song->buffers      = (uint32_t*)malloc(sizeof(uint32_t) * buffers);
  song->buffer_sizes = (uint32_t*)malloc(sizeof(uint32_t) * buffers);
  song->buffer_count = buffers;
  song->sample_count = stb_vorbis_stream_length_in_samples(song->vorbis);
  song->length = stb_vorbis_stream_length_in_seconds(song->vorbis) * 1000.f;

  if (!song->buffers) {
    song->data = 0;
    song->req  = 0;

    free(song->vorbis);

    ASTERA_FUNC_DBG("unable to allocate %i bytes for buffer IDs",
                    (buffers * sizeof(int32_t)));

    return 0;
  }

  alGenBuffers(buffers, song->buffers);
  alGenSources(1, &song->source);

  for (uint8_t i = 0; i < buffers; ++i) {
    uint32_t buffer = song->buffers[i], pcm_total_length = 0;

    // clear out the pcm for each buffer
    memset(ctx->pcm, 0, sizeof(uint16_t) * ctx->pcm_length);

    for (uint16_t j = 0; j < packets_per_buffer; ++j) {
      uint32_t pcm_remaining = ctx->pcm_length - pcm_total_length;

      if ((pcm_remaining / song->info.channels) < song->info.max_frame_size) {
        break;
      }

      uint32_t num_samples = stb_vorbis_get_samples_short_interleaved(
          song->vorbis, song->channels, ctx->pcm + pcm_total_length,
          pcm_remaining);

      song->buffer_sizes[i] = num_samples;

      if (num_samples > 0) {
        int32_t sample_count = song->channels * num_samples;
        pcm_total_length += sample_count;
      }
    }

    alBufferData(buffer, song->format, ctx->pcm,
                 pcm_total_length * sizeof(uint16_t), song->info.sample_rate);
    alSourceQueueBuffers(song->source, 1, &buffer);
  }

  if (new_high)
    ++ctx->song_high;

  ++ctx->song_count;

  return song->id;
}

uint8_t a_song_destroy(a_ctx* ctx, uint16_t id) {
  if (ctx->song_high < id - 1) {
    ASTERA_FUNC_DBG("no song in context with ID %i\n", id);
    return 0;
  }

  a_song* song = &ctx->songs[id - 1];

  alDeleteBuffers(song->buffer_count, song->buffers);
  alDeleteSources(1, &song->source);

  stb_vorbis_close(song->vorbis);

  for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
    _a_layer_remove(&ctx->layers[i], id, 0);
  }

  free(song->buffers);
  free(song->vorbis);

  return 1;
}

a_song* a_song_get_ptr(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in context with ID %i\n", song_id);
    return 0;
  }

  return &ctx->songs[song_id - 1];
}

a_song* a_song_get_open(a_ctx* ctx) {
  if (ctx->song_count == ctx->song_capacity) {
    ASTERA_FUNC_DBG("no open song slots.\n");
    return 0;
  }

  for (uint16_t i = 0; i < ctx->song_capacity; ++i) {
    a_song* song = &ctx->songs[i];
    if (!song->source) {
      return song;
    }
  }

  return 0;
}

uint8_t a_song_play(a_ctx* ctx, uint16_t layer_id, uint16_t song_id,
                    a_req* req) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("invalid song ID\n");
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

#if !defined(ASTERA_AL_NO_FX)
  // Apply fx
  if (req->fx_count > 0) {
    for (uint16_t i = 0; i < req->fx_count; ++i) {
      // Make sure it's valid within the range of effects
      if (req->fx[i] <= ctx->fx_capacity && req->fx[i] > 0) {
        alSource3i(song->source, AL_AUXILIARY_SEND_FILTER,
                   (ALint)ctx->fx_slots[req->fx[i] - 1].slot_id, i, 0);
      } else {
        ASTERA_FUNC_DBG("Invalid fx [%i] passed.\n", req->fx[i]);
      }
    }
  }

  // Apply filters
  if (req->filter_count > 0) {
    for (uint16_t i = 0; i < req->filter_count; ++i) {
      // Make sure it's valid within the range of filters
      if (req->filters[i] <= ctx->filter_capacity && req->filters[i] > 0) {
        alSourcei(song->source, AL_DIRECT_FILTER,
                  ctx->filter_slots[req->filters[i] - 1].al_id);
      } else {
        ASTERA_FUNC_DBG("Invalid filter [%i] passed.\n", req->filters[i]);
      }
    }
  }
#endif

  song->delta = 0;
  song->req   = req;

  alSourcef(song->source, AL_GAIN, req->gain);
  alSource3f(song->source, AL_POSITION, req->position[0], req->position[1],
             req->position[2]);
  alSource3f(song->source, AL_VELOCITY, req->velocity[0], req->velocity[1],
             req->velocity[2]);

  alSourcePlay(song->source);

  if (layer_id != 0) {
    _a_layer_add(_a_get_layer(ctx, layer_id), song_id, 0);
  }

  return 1;
}

uint8_t a_song_stop(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourceStop(song->source);

  return 1;
}

uint8_t a_song_pause(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourcePause(song->source);

  return 1;
}

uint8_t a_song_resume(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourcePlay(song->source);

  return 1;
}

time_s a_song_get_time(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];
  return song->delta;
}

uint32_t a_song_get_sample_count(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];

  return song->sample_count;
}

time_s a_song_get_length(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];
  return song->length;
}

uint8_t a_song_set_time(a_ctx* ctx, uint16_t song_id, time_s from_start) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in list with id %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  double   conv_time     = from_start * 1000.0;
  uint32_t approx_sample = conv_time * song->info.sample_rate;

  if (approx_sample > song->sample_count) {
    ASTERA_FUNC_DBG("sample requested %i out of range of song %i\n",
                    approx_sample, song->sample_count);
    return 0;
  }

  stb_vorbis_seek_frame(song->vorbis, approx_sample);

  return 1;
}

uint8_t a_song_reset(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  return _a_song_reset(ctx, song);
}

uint32_t a_song_get_state(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_FUNC_DBG("no song with ID %i\n", song_id);
    return AL_STOPPED;
  }

  ALenum   state;
  uint32_t source = ctx->songs[song_id - 1].source;
  if (!source) {
    return AL_STOPPED;
  } else {
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state;
  }
}

uint16_t a_song_get_id(a_ctx* ctx, const char* name) {
  if (ctx->song_count == 0) {
    ASTERA_FUNC_DBG("no songs in context.\n");
    return 0;
  }

  for (uint16_t i = 0; i < ctx->song_high; ++i) {
    if (strcmp(ctx->song_names[i], name) == 0) {
      // Song IDs are their index + 1
      return i + 1;
    }
  }

  return 0;
}

static int16_t _a_load_int16(unsigned char* data, int offset) {
  return *((int16_t*)&data[offset]);
}

static int32_t _a_load_int32(unsigned char* data, int offset) {
  return *((int32_t*)&data[offset]);
}

uint16_t a_buf_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                      const char* name, uint8_t is_ogg) {
  if (!data || !data_length) {
    ASTERA_FUNC_DBG("no asset passed to load into audio buffer.\n");
    return 0;
  }

  if (ctx->buffer_count == ctx->buffer_capacity) {
    ASTERA_FUNC_DBG("no free buffer slots.\n");
    return 0;
  }

  a_buf* buffer = 0;

  // Find open buffer
  for (uint16_t i = 0; i < ctx->buffer_high; ++i) {
    if (ctx->buffers[i].buf == 0) {
      buffer = &ctx->buffers[i];
      break;
    }
  }

  int8_t new_high = 0;
  if (!buffer && ctx->buffer_high < ctx->buffer_capacity - 1) {
    buffer   = &ctx->buffers[ctx->buffer_high];
    new_high = 1;
  } else {
    ASTERA_FUNC_DBG("no free buffer slots.\n");
    return 0;
  }

  alGenBuffers(1, &buffer->buf);

  if (is_ogg) {
    int32_t error;

    stb_vorbis* vorbis = stb_vorbis_open_memory(data, data_length, &error, 0);

    if (!vorbis) {
      ASTERA_FUNC_DBG("unable to open vorbis header, vorbis error %i\n", error);
      return 0;
    }

    uint16_t* pcm;
    int32_t   format, sample_rate, channels;

    int32_t byte_length = stb_vorbis_decode_memory(data, data_length, &channels,
                                                   &sample_rate, (short**)&pcm);

    format = (channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

    buffer->length      = vorbis->total_samples;
    buffer->sample_rate = sample_rate;
    buffer->channels    = channels;

    alBufferData(buffer->buf, format, pcm, byte_length, sample_rate);

    free(vorbis);
  } else {
    int16_t channels    = _a_load_int16(data, 22);
    int32_t sample_rate = _a_load_int32(data, 24);
    // int32_t byte_rate   = _a_load_int32(data, 28);
    int32_t bps = _a_load_int16(data, 34);

    if (strncmp((const char*)&data[36], "data", 4) != 0) {
      return 0;
    }

    int32_t byte_length = _a_load_int32(data, 40);

    int32_t format = -1;
    if (channels == 2) {
      format = (bps == 16) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
    } else if (channels == 1) {
      format = (bps == 16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
    }

    if (format == -1) {
      ASTERA_FUNC_DBG("Unsupported wave file format.\n");
      return 0;
    }

    buffer->channels    = channels;
    buffer->sample_rate = sample_rate;
    buffer->length      = (uint32_t)byte_length / bps;

    alBufferData(buffer->buf, format, &data[44], byte_length, sample_rate);
  }

  ctx->buffer_names[buffer->id - 1] = name;

  if (new_high)
    ++ctx->buffer_high;

  ++ctx->buffer_count;
  return buffer->id;
}

uint8_t a_buf_destroy(a_ctx* ctx, uint16_t buf_id) {
  if (ctx->buffer_high < buf_id - 1) {
    ASTERA_FUNC_DBG("no buffer in slot %i\n", buf_id);
    return 0;
  }

  a_buf* buffer = &ctx->buffers[buf_id - 1];
  alDeleteBuffers(1, (const ALuint*)&buffer->buf);
  buffer->buf = 0;

  return 1;
}

uint16_t a_buf_get(a_ctx* ctx, const char* name) {
  for (uint16_t i = 0; i < ctx->buffer_capacity; ++i) {
    if (strcmp(name, ctx->buffer_names[i]) == 0) {
      return ctx->buffers[i].id;
    }
  }

  return 0;
}

a_buf* a_buf_get_id(a_ctx* ctx, uint16_t id) {
  if (ctx->buffer_high < id - 1) {
    ASTERA_FUNC_DBG("id is above the high mark for buffers.\n");
    return 0;
  }

  return &ctx->buffers[id - 1];
}

a_buf* a_buf_get_open(a_ctx* ctx) {
  if (ctx->buffer_capacity == ctx->buffer_count) {
    ASTERA_FUNC_DBG("no open buffers.\n");
    return 0;
  }

  for (uint16_t i = 0; i < ctx->buffer_capacity; ++i) {
    a_buf* buf = &ctx->buffers[i];
    if (!buf->buf) {
      return buf;
    }
  }

  return 0;
}

float a_listener_get_gain(a_ctx* ctx) {
  return ctx->listener.gain;
}

void a_listener_get_pos(a_ctx* ctx, vec3 dst) {
  vec3_dup(dst, ctx->listener.position);
}

void a_listener_get_ori(a_ctx* ctx, float dst[6]) {
  for (uint8_t i = 0; i < 6; ++i) {
    dst[i] = ctx->listener._ori[i];
  }
}

void a_listener_get_vel(a_ctx* ctx, vec3 dst) {
  vec3_dup(dst, ctx->listener.velocity);
}

void a_listener_set_gain(a_ctx* ctx, float gain) {
  ctx->listener.gain = gain;
  alListenerf(AL_GAIN, gain);
}

void a_listener_set_pos(a_ctx* ctx, vec3 position) {
  vec3_dup(ctx->listener.position, position);
  alListener3f(AL_POSITION, position[0], position[1], position[2]);
}

void a_listener_set_ori(a_ctx* ctx, float ori[6]) {
  for (uint8_t i = 0; i < 6; ++i) {
    ctx->listener._ori[i] = ori[i];
  }

  alListenerfv(AL_ORIENTATION, ctx->listener._ori);
}

void a_listener_set_vel(a_ctx* ctx, vec3 velocity) {
  vec3_dup(ctx->listener.velocity, velocity);
  alListener3f(AL_VELOCITY, velocity[0], velocity[1], velocity[2]);
}

a_req a_req_create(vec3 position, float gain, float range, uint8_t loop,
                   uint16_t* fx, uint16_t fx_count, uint16_t* filters,
                   uint16_t filter_count) {
  a_req req = (a_req){.gain         = gain,
                      .range        = range,
                      .loop         = loop,
                      .fx           = fx,
                      .filters      = filters,
                      .fx_count     = fx_count,
                      .filter_count = filter_count,
                      0};

  vec3_dup(req.position, position);

  return req;
}

uint16_t a_fx_create(a_ctx* ctx, a_fx_type type, void* data) {
#if !defined(ASTERA_AL_NO_FX)
  if (!(type == FX_REVERB || type == FX_EQ)) {
    ASTERA_FUNC_DBG("invalid type of effect passed.\n");
    return 0;
  }

  if (ctx->fx_count == ctx->fx_capacity) {
    ASTERA_FUNC_DBG("no open fx slots.\n");
    return 0;
  }

  a_fx* slot = 0;

  for (uint16_t i = 0; i < ctx->fx_capacity; ++i) {
    if (ctx->fx_slots[i].effect_id == 0) {
      slot = &ctx->fx_slots[i];
      break;
    }
  }

  if (!slot) {
    ASTERA_FUNC_DBG("Unable to find open slot for fx\n");
    return 0;
  }

  ++ctx->fx_count;
  alGenEffects(1, &slot->effect_id);
  alGenAuxiliaryEffectSlots(1, &slot->slot_id);
  alAuxiliaryEffectSloti(slot->slot_id, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO,
                         AL_TRUE);

  switch (type) {
    case FX_EQ: {
      alEffecti(slot->effect_id, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);

      slot->data = data;
      slot->type = type;

      a_fx_eq* eq = (a_fx_eq*)data;

      alEffectf(slot->effect_id, AL_EQUALIZER_LOW_GAIN, eq->low_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_LOW_CUTOFF, eq->low_cutoff);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_GAIN, eq->mid1_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_CENTER, eq->mid1_center);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_WIDTH, eq->mid1_width);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_GAIN, eq->mid2_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_CENTER, eq->mid2_center);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_WIDTH, eq->mid2_width);
      alEffectf(slot->effect_id, AL_EQUALIZER_HIGH_GAIN, eq->high_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_HIGH_CUTOFF, eq->high_cutoff);
    } break;
    case FX_REVERB: {
      alEffecti(slot->effect_id, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
      slot->data = data;
      slot->type = type;

      a_fx_reverb* rv = (a_fx_reverb*)data;

      alEffectf(slot->effect_id, AL_REVERB_DENSITY, (ALfloat)rv->density);
      alEffectf(slot->effect_id, AL_REVERB_DIFFUSION, (ALfloat)rv->diffusion);
      alEffectf(slot->effect_id, AL_REVERB_GAIN, AL_REVERB_DEFAULT_GAIN);
      alEffectf(slot->effect_id, AL_REVERB_GAINHF, AL_REVERB_DEFAULT_GAINHF);
      alEffectf(slot->effect_id, AL_REVERB_DECAY_TIME, rv->decay);
      alEffectf(slot->effect_id, AL_REVERB_DECAY_HFRATIO, rv->decay_hfratio);
      alEffectf(slot->effect_id, AL_REVERB_REFLECTIONS_GAIN, rv->refl_gain);
      alEffectf(slot->effect_id, AL_REVERB_REFLECTIONS_DELAY, rv->refl_delay);
      alEffectf(slot->effect_id, AL_REVERB_LATE_REVERB_GAIN, rv->late_gain);
      alEffectf(slot->effect_id, AL_REVERB_LATE_REVERB_DELAY, rv->late_delay);
      alEffectf(slot->effect_id, AL_REVERB_AIR_ABSORPTION_GAINHF,
                rv->air_absorption_gainhf);
      alEffectf(slot->effect_id, AL_REVERB_ROOM_ROLLOFF_FACTOR,
                rv->room_rolloff_factor);
      alEffecti(slot->effect_id, AL_REVERB_DECAY_HFLIMIT, rv->decay_hflimit);
    } break;
    case FX_NONE:
      ASTERA_FUNC_DBG("invalid data type.\n");
      return 0;
  }

  alAuxiliaryEffectSloti(slot->slot_id, AL_EFFECTSLOT_EFFECT, slot->effect_id);

  // NOTE: slot->id is the index + 1 of the slot in the context's array of
  // slots
  return slot->id;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

uint8_t a_fx_destroy(a_ctx* ctx, uint16_t fx_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (fx_id <= 0 || fx_id > ctx->fx_capacity) {
    ASTERA_FUNC_DBG("no fx in that slot.\n");
    return 0;
  }

  a_fx* slot = &ctx->fx_slots[fx_id - 1];

  if (!slot) {
    ASTERA_FUNC_DBG("invalid slot pointer.\n");
    return 0;
  }

  if (slot->id == 0) {
    ASTERA_FUNC_DBG("no fx in that slot.\n");
    return 0;
  }

  if (slot->effect_id != 0)
    alDeleteEffects(1, &slot->effect_id);

  if (slot->slot_id != 0)
    alDeleteAuxiliaryEffectSlots(1, &slot->slot_id);

  --ctx->fx_count;
  slot->id = 0;

  return 1;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

uint8_t a_fx_update(a_ctx* ctx, uint16_t fx_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (fx_id <= 0 || fx_id > ctx->fx_capacity) {
    ASTERA_FUNC_DBG("no fx in slot: %i\n", fx_id);
    return 0;
  }

  a_fx* slot = &ctx->fx_slots[fx_id - 1];
  switch (slot->type) {
    case FX_NONE:
      return 0;
    case FX_EQ: {
      a_fx_eq* eq = (a_fx_eq*)slot->data;

      alEffectf(slot->effect_id, AL_EQUALIZER_LOW_GAIN, eq->low_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_LOW_CUTOFF, eq->low_cutoff);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_GAIN, eq->mid1_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_CENTER, eq->mid1_center);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID1_WIDTH, eq->mid1_width);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_GAIN, eq->mid2_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_CENTER, eq->mid2_center);
      alEffectf(slot->effect_id, AL_EQUALIZER_MID2_WIDTH, eq->mid2_width);
      alEffectf(slot->effect_id, AL_EQUALIZER_HIGH_GAIN, eq->high_gain);
      alEffectf(slot->effect_id, AL_EQUALIZER_HIGH_CUTOFF, eq->high_cutoff);
    } break;
    case FX_REVERB: {
      a_fx_reverb* rv = (a_fx_reverb*)slot->data;

      alEffectf(slot->effect_id, AL_REVERB_DENSITY, (ALfloat)rv->density);
      alEffectf(slot->effect_id, AL_REVERB_DIFFUSION, (ALfloat)rv->diffusion);
      alEffectf(slot->effect_id, AL_REVERB_GAIN, AL_REVERB_DEFAULT_GAIN);
      alEffectf(slot->effect_id, AL_REVERB_GAINHF, AL_REVERB_DEFAULT_GAINHF);
      alEffectf(slot->effect_id, AL_REVERB_DECAY_TIME, rv->decay);
      alEffectf(slot->effect_id, AL_REVERB_DECAY_HFRATIO, rv->decay_hfratio);
      alEffectf(slot->effect_id, AL_REVERB_REFLECTIONS_GAIN, rv->refl_gain);
      alEffectf(slot->effect_id, AL_REVERB_REFLECTIONS_DELAY, rv->refl_delay);
      alEffectf(slot->effect_id, AL_REVERB_LATE_REVERB_GAIN, rv->late_gain);
      alEffectf(slot->effect_id, AL_REVERB_LATE_REVERB_DELAY, rv->late_delay);
      alEffectf(slot->effect_id, AL_REVERB_AIR_ABSORPTION_GAINHF,
                rv->air_absorption_gainhf);
      alEffectf(slot->effect_id, AL_REVERB_ROOM_ROLLOFF_FACTOR,
                rv->room_rolloff_factor);
      alEffecti(slot->effect_id, AL_REVERB_DECAY_HFLIMIT, rv->decay_hflimit);
    } break;
  }

  return 1;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

a_fx_type a_fx_get_type(a_ctx* ctx, uint16_t fx_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (fx_id <= 0 || fx_id > ctx->fx_capacity) {
    ASTERA_FUNC_DBG("no fx in slot %i\n", fx_id);
    return 0;
  }

  return ctx->fx_slots[fx_id - 1].type;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

a_fx* a_fx_get_slot(a_ctx* ctx, uint16_t fx_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (fx_id <= 0 || fx_id > ctx->fx_capacity) {
    ASTERA_FUNC_DBG("no fx in slot %i\n", fx_id);
    return 0;
  }

  return &ctx->fx_slots[fx_id - 1];
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

a_fx_reverb a_fx_reverb_default(void) {
#if !defined(ASTERA_AL_NO_FX)
  return (a_fx_reverb){
      .density               = AL_REVERB_DEFAULT_DENSITY,
      .diffusion             = AL_REVERB_DEFAULT_DIFFUSION,
      .gain                  = AL_REVERB_DEFAULT_GAIN,
      .gainhf                = AL_REVERB_DEFAULT_GAINHF,
      .decay                 = AL_REVERB_DEFAULT_DECAY_TIME,
      .decay_hfratio         = AL_REVERB_DEFAULT_DECAY_HFRATIO,
      .refl_gain             = AL_REVERB_DEFAULT_REFLECTIONS_GAIN,
      .refl_delay            = AL_REVERB_DEFAULT_REFLECTIONS_DELAY,
      .late_gain             = AL_REVERB_DEFAULT_LATE_REVERB_GAIN,
      .late_delay            = AL_REVERB_DEFAULT_LATE_REVERB_DELAY,
      .air_absorption_gainhf = AL_REVERB_DEFAULT_AIR_ABSORPTION_GAINHF,
      .room_rolloff_factor   = AL_REVERB_DEFAULT_ROOM_ROLLOFF_FACTOR,
      .decay_hflimit         = AL_REVERB_DEFAULT_DECAY_HFLIMIT};
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return (a_fx_reverb){0};
#endif
}

a_fx_reverb a_fx_reverb_create(float density, float diffusion, float gain,
                               float gainhf, float decay, float decay_hfratio,
                               float refl_gain, float refl_delay,
                               float late_gain, float late_delay,
                               float  air_absorption_gainhf,
                               float  room_rolloff_factor,
                               int8_t decay_hflimit) {
#if !defined(ASTERA_AL_NO_FX)
  a_fx_reverb rev;
  rev = (a_fx_reverb){
      .density   = _a_clamp(density, 0.f, 1.f, AL_REVERB_DEFAULT_DENSITY),
      .diffusion = _a_clamp(diffusion, 0.f, 1.f, AL_REVERB_DEFAULT_DIFFUSION),
      .gain      = _a_clamp(gain, 0.f, 1.f, AL_REVERB_DEFAULT_GAIN),
      .gainhf    = _a_clamp(gainhf, 0.f, 1.f, AL_REVERB_DEFAULT_GAINHF),
      .decay     = _a_clamp(decay, 0.1f, 20.f, AL_REVERB_DEFAULT_DECAY_TIME),
      .decay_hfratio =
          _a_clamp(decay_hfratio, 0.1, 2.0, AL_REVERB_DEFAULT_DECAY_HFRATIO),
      .refl_gain =
          _a_clamp(refl_gain, 0.f, 3.16, AL_REVERB_DEFAULT_REFLECTIONS_GAIN),
      .refl_delay =
          _a_clamp(refl_delay, 0.f, 0.3f, AL_REVERB_DEFAULT_REFLECTIONS_DELAY),
      .late_gain =
          _a_clamp(late_gain, 0.0, 10.0, AL_REVERB_DEFAULT_LATE_REVERB_GAIN),
      .late_delay =
          _a_clamp(late_delay, 0.0, 0.1, AL_REVERB_DEFAULT_LATE_REVERB_DELAY),
      .air_absorption_gainhf =
          _a_clamp(air_absorption_gainhf, 0.892, 1.0,
                   AL_REVERB_DEFAULT_AIR_ABSORPTION_GAINHF),
      .room_rolloff_factor = _a_clamp(room_rolloff_factor, 0.0f, 10.f,
                                      AL_REVERB_DEFAULT_ROOM_ROLLOFF_FACTOR),
      .decay_hflimit = (decay_hflimit) ? 1 : AL_REVERB_DEFAULT_DECAY_HFLIMIT};

  return rev;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return (a_fx_reverb){0};
#endif
}

a_fx_eq a_fx_eq_create(float low_gain, float low_cutoff, float mid1_gain,
                       float mid1_center, float mid1_width, float mid2_gain,
                       float mid2_center, float mid2_width, float high_gain,
                       float high_cutoff) {
#if !defined(ASTERA_AL_NO_FX)
  return (a_fx_eq){.low_gain    = _a_clamp(low_gain, 0.126f, 7.943f, 1.f),
                   .low_cutoff  = _a_clamp(low_cutoff, 50.f, 800.f, 200.f),
                   .mid1_gain   = _a_clamp(mid1_gain, 0.126f, 7.943f, 1.f),
                   .mid1_center = _a_clamp(mid1_center, 200.f, 3000.f, 500.f),
                   .mid1_width  = _a_clamp(mid1_width, 0.01f, 1.f, 1.f),
                   .mid2_gain   = _a_clamp(mid2_gain, 0.126f, 7.943f, 1.f),
                   .mid2_center = _a_clamp(mid2_center, 1000.f, 8000.f, 3000.f),
                   .mid2_width  = _a_clamp(mid2_width, 0.01f, 1.f, 1.f),
                   .high_gain   = _a_clamp(high_gain, 0.126f, 7.943f, 1.f),
                   .high_cutoff =
                       _a_clamp(high_cutoff, 4000.f, 16000.f, 6000.f)};
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return (a_fx_eq){0};
#endif
}

uint16_t a_filter_create(a_ctx* ctx, a_filter_type type, float gain, float hf,
                         float lf) {
#if !defined(ASTERA_AL_NO_FX)
  if (ctx->filter_count == ctx->filter_capacity) {
    ASTERA_FUNC_DBG("no free slots.\n");
    return 0;
  }

  a_filter* slot = 0;

  for (uint16_t i = 0; i < ctx->filter_capacity; ++i) {
    if (!ctx->filter_slots[i].al_id) {
      slot = &ctx->filter_slots[i];
      break;
    }
  }

  if (!slot) {
    return 0;
  }

  ++ctx->filter_count;

  if (!slot->al_id) {
    alGenFilters(1, &slot->al_id);
  }

  slot->gain = _a_clamp(gain, 0.f, 1.f, 1.f);
  slot->type = type;

  switch (type) {
    case FILTER_LOW:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_LOWPASS);

      slot->data.low.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_LOWPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_LOWPASS_GAINHF, slot->data.low.gainhf);

      break;
    case FILTER_HIGH:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);

      slot->data.high.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_HIGHPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_HIGHPASS_GAINLF, slot->data.high.gainlf);

      break;
    case FILTER_BAND:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_BANDPASS);

      slot->data.band.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);
      slot->data.band.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_BANDPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_BANDPASS_GAINLF, slot->data.band.gainlf);
      alFilterf(slot->al_id, AL_BANDPASS_GAINHF, slot->data.band.gainhf);
      break;
  }

  return slot->id;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

uint8_t a_filter_update(a_ctx* ctx, uint16_t filter_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (filter_id > ctx->filter_capacity || filter_id <= 0) {
    ASTERA_FUNC_DBG("no filter in slot %i\n", filter_id);
    return 0;
  }

  a_filter* slot = &ctx->filter_slots[filter_id - 1];

  switch (slot->type) {
    case FILTER_LOW:
      alFilterf(slot->al_id, AL_LOWPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_LOWPASS_GAINHF, slot->data.low.gainhf);
      break;
    case FILTER_HIGH:
      alFilterf(slot->al_id, AL_HIGHPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_HIGHPASS_GAINLF, slot->data.high.gainlf);
      break;
    case FILTER_BAND:
      alFilterf(slot->al_id, AL_BANDPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_BANDPASS_GAINLF, slot->data.band.gainlf);
      alFilterf(slot->al_id, AL_BANDPASS_GAINHF, slot->data.band.gainhf);
      break;
  }

  return 1;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

uint8_t a_filter_destroy(a_ctx* ctx, uint16_t filter_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (filter_id > ctx->filter_capacity || filter_id <= 0) {
    ASTERA_FUNC_DBG("no filter in slot %i\n", filter_id);
    return 0;
  }

  alDeleteFilters(1, &ctx->filter_slots[filter_id - 1].al_id);
  ctx->filter_slots[filter_id - 1].al_id = 0;
  --ctx->filter_count;

  return 1;
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

a_filter* a_filter_get_slot(a_ctx* ctx, uint16_t filter_id) {
#if !defined(ASTERA_AL_NO_FX)
  if (filter_id > ctx->filter_capacity || filter_id <= 0) {
    ASTERA_FUNC_DBG("no filter in slot %i\n", filter_id);
    return 0;
  }

  return &ctx->filter_slots[filter_id - 1];
#else
  ASTERA_FUNC_DBG("ASTERA_AL_NO_FX is defined\n");
  return 0;
#endif
}

uint16_t a_layer_create(a_ctx* ctx, const char* name, uint32_t max_sfx,
                        uint32_t max_songs) {
  if (ctx->layer_count == ctx->layer_capacity) {
    return 0;
  }

  a_layer* layer = 0;
  for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
    if (ctx->layers[i].id == 0) {
      layer     = &ctx->layers[i];
      layer->id = i + 1;
      break;
    }
  }

  if (!layer) {
    return 0;
  }

  layer->name = name;

  layer->sfx          = (uint32_t*)calloc(max_sfx, sizeof(uint32_t));
  layer->sfx_capacity = max_sfx;
  layer->sfx_count    = 0;

  layer->songs         = (uint32_t*)calloc(max_songs, sizeof(uint32_t));
  layer->song_capacity = max_songs;
  layer->song_count    = 0;

  ++ctx->layer_count;

  return layer->id;
}

uint16_t a_layer_get_id(a_ctx* ctx, const char* name) {
  for (uint16_t i = 0; i < ctx->layer_capacity; ++i) {
    if (ctx->layers[i].id != 0 && ctx->layers[i].name) {
      if (strcmp(name, ctx->layers[i].name) == 0) {
        return ctx->layers[i].id;
      }
    }
  }

  return 0;
}

uint8_t a_layer_destroy(a_ctx* ctx, uint16_t layer_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);

  if (!layer) {
    return 0;
  }

  _a_layer_destroy(layer);
  --ctx->layer_count;

  return 1;
}

uint8_t a_layer_add_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);
  if (!layer)
    return 0;

  return _a_layer_add(layer, sfx_id, 1);
}

uint8_t a_layer_add_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);
  if (!layer)
    return 0;

  return _a_layer_add(layer, song_id, 0);
}

uint8_t a_layer_remove_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);
  if (!layer)
    return 0;

  return _a_layer_remove(layer, sfx_id, 1);
}

uint8_t a_layer_remove_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);
  if (!layer)
    return 0;

  return _a_layer_remove(layer, song_id, 0);
}

uint8_t a_layer_set_gain(a_ctx* ctx, uint16_t layer_id, float gain) {
  a_layer* layer = _a_get_layer(ctx, layer_id);
  if (!layer) {
    return 0;
  }

  if (layer->gain == gain) {
    return 0;
  }

  layer->gain = gain;

  return 1;
}

float a_layer_get_gain(a_ctx* ctx, uint16_t layer_id) {
  a_layer* layer = _a_get_layer(ctx, layer_id);

  if (!layer)
    return -1.f;

  return layer->gain;
}


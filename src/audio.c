#include <astera/audio.h>
#include <astera/debug.h>

#if !defined(ASTERA_AL_DISTANCE_MODEL)
#define ASTERA_AL_DISTANCE_MODEL AL_INVERSE_DISTANCE
#endif

#if !defined(ASTERA_AL_ROLLOFF_FACTOR)
#define ASTERA_AL_ROLLOFF_FACTOR 1.f
#endif

#undef STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_MAX_CHANNELS 2
#include <stb_vorbis.c>

#include <string.h>

#if !defined(ASTERA_AL_NO_EFX)
#include <AL/efx.h>

#include <stdio.h>

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
                         : (value < min) ? min : (value > max) ? max : value;
}

void a_efx_info(a_ctx* ctx) {
  if (alcIsExtensionPresent(ctx->device, "ALC_EXT_EFX") == AL_FALSE) {
    ASTERA_DBG("a_efx_info: No ALC_EXT_EFX.\n");
  } else {
    ASTERA_DBG("a_efx_info: ALC_EXT_EFX Present.\n");
  }

  ASTERA_DBG("a_efx_info: MAX_AUXILIARY_SENDS: %i\n", ALC_MAX_AUXILIARY_SENDS);
  ALCint s_sends = 0;
  alcGetIntegerv(ctx->device, ALC_MAX_AUXILIARY_SENDS, 1, &s_sends);
  ASTERA_DBG("a_efx_info: MAX_AUXILIARY_SENDS: %i\n", s_sends);
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

uint8_t a_ctx_play_allowed(a_ctx* ctx) { return ctx->allow; }

a_ctx* a_ctx_create(const char* device, uint8_t layers, uint16_t max_sfx,
                    uint16_t max_buffers, uint16_t max_songs, uint16_t max_fx,
                    uint16_t max_filters, uint32_t pcm_size) {
  a_ctx* ctx = (a_ctx*)malloc(sizeof(a_ctx));

  ASTERA_DBG("Test 2!.\n");

  if (!ctx) {
    ASTERA_DBG("a_ctx_create: unable to malloc initial space for context.\n");
    return 0;
  }

  ALCdevice* al_device = alcOpenDevice(device);

  if (alcIsExtensionPresent(al_device, "ALC_EXT_EFX") == AL_FALSE) {
    ctx->use_fx = 0;
  } else {
    ctx->use_fx = 1;
  }

  ctx->max_fx = max_fx;

  // Disable effects if we're not using any of them
  if (!ctx->max_fx && !max_filters) {
    ctx->use_fx = 0;
  }

  ALint attribs[4] = {0};

  if (ctx->use_fx) {
    attribs[0] = ALC_MAX_AUXILIARY_SENDS;
    attribs[1] = 4;
  }

  ALCcontext* context = alcCreateContext(al_device, attribs);

  if (!alcMakeContextCurrent(context)) {
    ASTERA_DBG("Error creating OpenAL Context\n");
    free(ctx);
    return 0;
  }

  ctx->context = context;
  ctx->device  = al_device;

#if defined(ASTERA_AL_DISTANCE_MODEL)
  alDistanceModel(ASTERA_AL_DISTANCE_MODEL);
#endif

  if (ctx->use_fx) {
    alcGetIntegerv(al_device, ALC_MAX_AUXILIARY_SENDS, 1, &ctx->fx_per_source);

    if (!ctx->fx_per_source) {
      ASTERA_DBG(
          "a_ctx_create: 0 effects allowed per source, disabling effects.\n");
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
    ctx->fx_high     = 0;

    if (max_fx) {
      ctx->fx_slots = (a_fx*)malloc(sizeof(a_fx) * ctx->max_fx);
      if (!ctx->fx_slots) {
        ASTERA_DBG("a_ctx_create: unable to allocate %i fx slots\n",
                   ctx->max_fx);

        alcDestroyContext(ctx->context);
        alcCloseDevice(ctx->device);

        free(ctx);
        return 0;
      }

      for (uint16_t i = 0; i < ctx->max_fx; ++i) {
        ctx->fx_slots[i].id = i + 1;
      }
    } else {
      ctx->fx_slots = 0;
    }

    ctx->filter_count    = 0;
    ctx->filter_capacity = max_filters;
    ctx->filter_high     = 0;

    if (max_filters) {
      ctx->filter_slots =
          (a_filter*)malloc(sizeof(a_filter) * ctx->filter_capacity);

      if (!ctx->filter_slots) {
        ASTERA_DBG("a_ctx_create: unable to allocate %i fx slots\n",
                   ctx->max_fx);

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

  ctx->pcm_length = pcm_size;
  ctx->pcm_index  = 0;
  if (pcm_size) {
    ctx->pcm = (uint16_t*)malloc(sizeof(uint16_t) * pcm_size);
    memset(ctx->pcm, 0, sizeof(uint16_t) * pcm_size);
  }

  // Create the resource arrays
  ctx->song_capacity = max_songs;
  ctx->song_count    = 0;
  ctx->song_high     = 0;

  if (max_songs) {
    ctx->songs      = (a_song*)malloc(sizeof(a_song) * ctx->song_capacity);
    ctx->song_names = (const char**)malloc(sizeof(char*) * ctx->song_capacity);

    // this will do the trick
    memset(ctx->songs, 0, sizeof(a_song) * ctx->song_capacity);
    memset(ctx->song_names, 0, sizeof(char*) * ctx->song_capacity);

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
    ctx->buffers      = (a_buf*)malloc(sizeof(a_buf) * max_buffers);
    ctx->buffer_names = (const char**)malloc(sizeof(char*) * max_buffers);

    for (uint16_t i = 0; i < max_buffers; ++i) {
      ctx->buffers[i].id   = i + 1;
      ctx->buffers[i].buf  = 0;
      ctx->buffer_names[i] = 0;
    }
  }

  ctx->sfx_capacity = max_sfx;
  ctx->sfx_count    = 0;
  ctx->sfx_high     = 0;

  if (max_sfx) {
    ctx->sfx = (a_sfx*)malloc(sizeof(a_sfx) * ctx->sfx_capacity);

    for (uint16_t i = 0; i < max_sfx; ++i) {
      ctx->sfx[i].id = i + 1;
      alGenSources(1, &ctx->sfx[i].source);
      ctx->sfx[i].req = 0;
    }
  }

  ctx->layer_count    = 0;
  ctx->layer_capacity = layers;
  ctx->layer_high     = 0;
  if (layers) {
    ctx->layers      = (a_layer*)malloc(sizeof(a_layer) * layers);
    ctx->layer_names = (const char**)malloc(sizeof(char*) * layers);

    for (uint16_t i = 0; i < layers; ++i) {
      ctx->layers[i].id = i + 1;
    }
  }

  ctx->allow = 1;
  return ctx;
}

uint8_t a_ctx_destroy(a_ctx* ctx) {
  if (!ctx) {
    ASTERA_DBG("a_ctx_destroy: no context passed to destroy.\n");
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

  if (ctx->layers) {
    for (uint32_t i = 0; i < ctx->layer_count; ++i) {
      if (ctx->layers[i].sfx_capacity > 0 && ctx->layers[i].sfx) {
        free(ctx->layers[i].sfx);
      }

      if (ctx->layers[i].song_capacity > 0 && ctx->layers[i].songs) {
        free(ctx->layers[i].songs);
      }
    }

    free(ctx->layers);
  }

  if (ctx->layer_names)
    free(ctx->layer_names);

  if (ctx->pcm)
    free(ctx->pcm);

  alcDestroyContext(ctx->context);
  alcCloseDevice(ctx->device);

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

      if ((al_error = alGetError()) == AL_INVALID_VALUE) {
        ASTERA_DBG("a_song_update_decode: AL Error %i\n", al_error);
      }
    }
  }

  if (proc == song->buffer_count && state == AL_PLAYING) {
    alSourcePlay(song->source);
  }
}

void a_ctx_update(a_ctx* ctx) {
  for (uint16_t i = 0; i < ctx->song_capacity; ++i) {
    a_song* song = &ctx->songs[i];
    if (!song)
      continue;

    if (song->req) {
      if (song->req->stop) {
        a_song_stop(ctx, song->id);
        continue;
      }

      ALenum state;
      alGetSourcei(song->source, AL_SOURCE_STATE, &state);

      song->req->state = state;

      if (state == AL_PLAYING) {
        a_song_update_decode(ctx, song);
        alSourcef(song->source, AL_GAIN, song->req->gain);
        alSource3f(song->source, AL_POSITION, song->req->position[0],
                   song->req->position[1], song->req->position[2]);
        alSource3f(song->source, AL_VELOCITY, song->req->velocity[0],
                   song->req->velocity[1], song->req->velocity[2]);
        alSourcef(song->source, AL_MAX_DISTANCE, song->req->range);
      }
    }
  }

  uint32_t sfx_count = 0, sfx_high = 0;
  for (uint16_t i = 0; i < ctx->sfx_capacity; ++i) {
    a_sfx* sfx = &ctx->sfx[i];

    if (!sfx)
      continue;

    if (!sfx->buffer || !sfx->req || !sfx->source) {
      continue;
    }

    if (!sfx->req)
      continue;

    ALenum state;
    alGetSourcei(sfx->source, AL_SOURCE_STATE, &state);

    int8_t remove = state == AL_STOPPED || sfx->req->stop;

    if (remove) {
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
    } else {
      if (i > ctx->sfx_high) {
        ctx->sfx_high = i;
      }

      ++sfx_count;
      sfx_high = i;
    }

    float gain = sfx->req->gain;

    alSourcef(sfx->source, AL_GAIN, sfx->req->gain);
    alSource3f(sfx->source, AL_POSITION, sfx->req->position[0],
               sfx->req->position[1], sfx->req->position[2]);
    alSource3f(sfx->source, AL_VELOCITY, sfx->req->velocity[0],
               sfx->req->velocity[1], sfx->req->velocity[2]);
    alSourcef(sfx->source, AL_MAX_DISTANCE, sfx->req->range);
    alSourcei(sfx->source, AL_LOOPING, sfx->req->loop);

    uint32_t sample_offset;
    alGetSourcei(sfx->source, AL_SAMPLE_OFFSET, &sample_offset);
    if (sample_offset >= sfx->length) {
      if (sfx->req->loop)
        ++sfx->req->loop_count;
    }

    alGetSourcef(sfx->source, AL_SEC_OFFSET, &sfx->req->time);
  }

  ctx->sfx_count = sfx_count;
  ctx->sfx_high  = sfx_high;

  // Update the layer's gain
  for (uint16_t i = 0; i < ctx->layer_high; ++i) {
    a_layer* layer = &ctx->layers[i];

    if (ctx->layers[i].sfx) {
      for (uint16_t j = 0; j < layer->sfx_count; ++j) {
        if (layer->sfx[j]->req) {
          alSourcef(layer->sfx[j]->source, AL_GAIN,
                    layer->gain * layer->sfx[j]->req->gain);
        }
      }
    }

    if (ctx->layers[i].songs) {
      for (uint16_t j = 0; j < layer->song_count; ++j) {
        if (layer->songs[j]->req) {
          alSourcef(layer->songs[j]->source, AL_GAIN,
                    layer->gain * layer->songs[j]->req->gain);
        }
      }
    }
  }
}

uint16_t a_ctx_layer_create(a_ctx* ctx, const char* name, uint16_t max_sfx,
                            uint16_t max_songs) {
  if (ctx->layer_count == ctx->layer_capacity) {
    ASTERA_DBG("a_ctx_layer_create: no free layer slots.\n");
    return 0;
  }

  if (!max_sfx && !max_songs) {
    ASTERA_DBG("a_ctx_layer_create: not going to create a completely empty "
               "layer.\n");
    return 0;
  }

  a_layer* layer = 0;

  for (uint16_t i = 0; i < ctx->layer_high; ++i) {
    if (!ctx->layers[i].songs && !ctx->layers[i].sfx) {
      layer = &ctx->layers[i];
      break;
    }
  }

  int8_t new_high = 0;
  if (!layer && ctx->layer_high < ctx->layer_capacity) {
    layer    = &ctx->layers[ctx->layer_high];
    new_high = 1;
  } else {
    ctx->layer_count = ctx->layer_capacity;
    ASTERA_DBG("a_ctx_layer_create: no free layer slots.\n");
    return 0;
  }

  layer->sfx_capacity = max_sfx;
  layer->sfx_count    = 0;
  if (max_sfx) {
    layer->sfx = (a_sfx**)malloc(sizeof(a_sfx*) * max_sfx);
  }

  layer->song_capacity = max_songs;
  layer->song_count    = 0;
  if (max_songs) {
    layer->songs = (a_song**)malloc(sizeof(a_song*) * max_songs);
  }

  layer->gain        = 1.f;
  layer->gain_change = 0;

  if (new_high) {
    ctx->layer_high = layer->id - 1;
  }

  ++ctx->layer_count;
  return layer->id;
}

uint16_t a_sfx_play(a_ctx* ctx, uint16_t layer, uint16_t buf_id, a_req* req) {
  if (ctx->sfx_count == ctx->sfx_capacity) {
    ASTERA_DBG("a_sfx_play: no free sfx slots.\n");
    return 0;
  }

  a_buf* buf = a_buf_get_id(ctx, buf_id);
  if (!buf) {
    ASTERA_DBG("a_sfx_play: unable to find %i\n", buf_id);
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
    ASTERA_DBG("No slots open, really.\n");
    return 0;
  }

  if (!slot->source) {
    alGenSources(1, &slot->source);
  }

  alSourcei(slot->source, AL_BUFFER, buf->buf);

  // Apply fx
  if (req->fx_count > 0) {
    for (uint16_t i = 0; i < req->fx_count; ++i) {
      // Make sure it's valid within the range of filters
      if (req->fx[i] < ctx->filter_high) {
        alSource3i(slot->source, AL_AUXILIARY_SEND_FILTER,
                   (ALint)ctx->filter_slots[req->fx[i] - 1].al_id, i, 0);
      }
    }
  }

  // Apply filters
  if (req->filter_count > 0) {
    for (uint16_t i = 0; i < req->filter_count; ++i) {
      if (ctx->filter_high < req->filters[i] - 1) {
        alSourcei(slot->source, AL_DIRECT_FILTER,
                  ctx->filter_slots[req->filters[i] - 1].al_id);
      }
    }
  }

  slot->buffer = buf->id;
  slot->length = buf->length;
  slot->req    = req;

  uint32_t size;
  alGetBufferi(buf->buf, AL_SIZE, &size);

  alSourcef(slot->source, AL_GAIN, req->gain);
  alSource3f(slot->source, AL_POSITION, req->position[0], req->position[1],
             req->position[2]);
  alSource3f(slot->source, AL_VELOCITY, req->velocity[0], req->velocity[1],
             req->velocity[2]);
  alSourcei(slot->source, AL_LOOPING, req->loop);

  alSourcePlay(slot->source);

  ++ctx->sfx_count;

  return slot->id;
}

uint8_t a_sfx_remove(a_ctx* ctx, uint16_t sfx_id) {
  if (ctx->sfx_high < sfx_id - 1) {
    ASTERA_DBG("a_sfx_remove: no sfx in slot %i\n", sfx_id);
    return 0;
  }

  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  if (sfx->req) {
    sfx->req->valid = 0;
    sfx->req->state = AL_STOPPED;
  }

  sfx->buffer = 0;
  sfx->length = 0;

  --ctx->sfx_count;

  if (ctx->sfx_high == sfx->id - 1) {
    if (ctx->sfx_count > 0) {
      for (uint16_t i = sfx->id - 1; i > 0; --i) {
        if (ctx->sfx[i].buffer) {
          ctx->sfx_high = i;
          break;
        }
      }
    } else {
      ctx->sfx_high = 0;
    }
  }

  return 1;
}

uint8_t a_sfx_stop(a_ctx* ctx, uint16_t sfx_id) {
  if (ctx->sfx_high < sfx_id - 1) {
    ASTERA_DBG("a_sfx_pause: no sfx in slot %i\n", sfx_id);
    return 0;
  }

  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  if (sfx->req) {
    sfx->req->valid = 0;
    sfx->req->state = AL_STOPPED;
  }

  alSourceStop(sfx->source);

  sfx->buffer = 0;
  sfx->length = 0;

  --ctx->sfx_count;

  if (ctx->sfx_high == sfx->id - 1) {
    if (ctx->sfx_count > 0) {
      for (uint16_t i = sfx->id - 1; i > 0; --i) {
        if (ctx->sfx[i].buffer) {
          ctx->sfx_high = i;
          break;
        }
      }
    } else {
      ctx->sfx_high = 0;
    }
  }

  return 1;
}

uint8_t a_sfx_pause(a_ctx* ctx, uint16_t sfx_id) {
  if (ctx->sfx_high < sfx_id - 1) {
    ASTERA_DBG("a_sfx_pause: no sfx in slot %i\n", sfx_id);
    return 0;
  }

  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  alSourcePause(sfx->source);

  if (sfx->req) {
    sfx->req->state = AL_PAUSED;
  }

  return 1;
}

uint8_t a_sfx_resume(a_ctx* ctx, uint16_t sfx_id) {
  if (ctx->sfx_high < sfx_id - 1) {
    ASTERA_DBG("a_sfx_resume: no sfx in slot %i\n", sfx_id);
    return 0;
  }

  a_sfx* sfx = &ctx->sfx[sfx_id - 1];

  alSourcePlay(sfx->source);

  return 1;
}

uint16_t a_layer_get_id(a_ctx* ctx, const char* name) {
  for (uint16_t i = 0; i < ctx->layer_high; ++i) {
    if (strcmp(ctx->layer_names[i], name) == 0) {
      // NOTE: ID is the index + 1 of the slot
      return i + 1;
    }
  }

  return 0;
}

uint8_t a_layer_set_gain(a_ctx* ctx, uint16_t layer_id, float gain) {
  if (ctx->layer_high < layer_id - 1) {
    ASTERA_DBG("a_layer_set_gain: no layer in slot %i\n", layer_id);
    return 0;
  }

  a_layer* layer = &ctx->layers[layer_id - 1];

  layer->gain = gain;

  if (layer->song_count) {
    for (uint16_t i = 0; i < layer->song_count; ++i) {
      a_song* song = layer->songs[i];
      if (song->req) {
        alSourcef(song->source, AL_GAIN, gain * song->req->gain);
      }
    }
  }

  if (layer->sfx_count) {
    for (uint16_t i = 0; i < layer->sfx_count; ++i) {
      a_sfx* sfx = layer->sfx[i];
      if (sfx->req) {
        alSourcef(sfx->source, AL_GAIN, gain * sfx->req->gain);
      }
    }
  }

  return 1;
}

uint8_t a_layer_add_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id) {
  if (ctx->layer_high < layer_id - 1) {
    ASTERA_DBG("a_layer_add_song: no layer in slot %i\n", layer_id);
    return 0;
  }

  a_layer* layer = &ctx->layers[layer_id - 1];

  if (layer->song_count == layer->song_capacity) {
    ASTERA_DBG("a_layer_add_song: no open song slots in layer %i\n", layer_id);
    return 0;
  }

  layer->songs[layer->song_count] = &ctx->songs[song_id - 1];
  ++layer->song_count;

  return 1;
}

uint8_t a_layer_add_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id) {
  if (ctx->layer_high < layer_id - 1) {
    ASTERA_DBG("a_layer_add_sfx: no layer in slot %i\n", layer_id);
    return 0;
  }

  a_layer* layer = &ctx->layers[layer_id - 1];

  if (layer->sfx_count == layer->sfx_capacity) {
    ASTERA_DBG("a_layer_add_sfx: no free sfx slots in layer %i\n", layer_id);
    return 0;
  }

  layer->sfx[layer->sfx_count] = &ctx->sfx[sfx_id - 1];
  ++layer->sfx_count;

  return 1;
}

uint8_t a_layer_remove_song(a_ctx* ctx, uint16_t layer_id, uint16_t song_id) {
  if (ctx->layer_high < layer_id - 1) {
    ASTERA_DBG("a_layer_add_song: no layer in slot %i\n", layer_id);
    return 0;
  }

  a_layer* layer = &ctx->layers[layer_id - 1];

  int8_t start = 0;
  for (uint16_t i = 0; i < layer->song_capacity - 1; ++i) {
    if (layer->songs[i]->id == song_id) {
      start = 1;
    }

    if (start) {
      layer->songs[i] = layer->songs[i + 1];
    }
  }

  if (start || layer->songs[layer->song_count - 1]->id == song_id) {
    layer->songs[layer->song_count] = 0;
    layer->song_count--;
  } else {
    ASTERA_DBG("a_layer_remove_song: unable to find song with ID %i\n",
               song_id);
    return 0;
  }

  return 1;
}

uint8_t a_layer_remove_sfx(a_ctx* ctx, uint16_t layer_id, uint16_t sfx_id) {
  if (ctx->layer_high < layer_id - 1) {
    ASTERA_DBG("a_layer_remove_sfx: no layer in slot %i\n", layer_id);
    return 0;
  }

  a_layer* layer = &ctx->layers[layer_id - 1];

  int8_t start = 0;
  for (uint16_t i = 0; i < layer->sfx_capacity - 1; ++i) {
    if (layer->songs[i]->id == sfx_id) {
      start = 1;
    }

    if (start) {
      layer->sfx[i] = layer->sfx[i + 1];
    }
  }

  if (start || layer->sfx[layer->sfx_count - 1]->id == sfx_id) {
    layer->sfx[layer->sfx_count] = 0;
    layer->sfx_count--;
  } else {
    ASTERA_DBG("a_layer_remove_sfx: unable to find sfx in layer with ID %i\n",
               sfx_id);
    return 0;
  }

  return 1;
}

float a_layer_get_gain(a_ctx* ctx, uint16_t layer_id) {
  if (ctx->layer_high > layer_id - 1) {
    ASTERA_DBG("a_layer_get_gain: no layer contained with ID %i\n", layer_id);
    return -1.f;
  }

  return ctx->layers[layer_id - 1].gain;
}

uint16_t a_song_create(a_ctx* ctx, unsigned char* data, uint32_t data_length,
                       const char* name, uint16_t packets_per_buffer,
                       uint8_t buffers, uint32_t max_buffer_size) {
  if (!data || !data_length || !packets_per_buffer || !buffers ||
      !max_buffer_size) {
    ASTERA_DBG("a_song_create: Invalid parameters passed\n");
    return 0;
  }

  a_song* song = 0;

  for (uint16_t i = 0; i < ctx->song_high; ++i) {
    if (!ctx->songs[i].buffers) {
      song = &ctx->songs[i];
    }
  }

  int8_t new_high = 0;
  if (!song && ctx->song_high < ctx->song_capacity) {
    song     = &ctx->songs[ctx->song_high];
    new_high = 1;
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
    ASTERA_DBG("a_song_create: Unable to load vorbis, that sucks.\n");
    return 0;
  }

  song->packets_per_buffer = packets_per_buffer;

  song->info = stb_vorbis_get_info(song->vorbis);

  if (max_buffer_size < song->info.max_frame_size) {
    ASTERA_DBG("a_song_create: max_buffer_size is smaller than the listed max "
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

    ASTERA_DBG("a_song_create: unable to allocate %i bytes for buffer IDs",
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
    ASTERA_DBG("a_song_destroy: no song in context with ID %i\n", id);
    return 0;
  }

  a_song* song = &ctx->songs[id - 1];

  alDeleteBuffers(song->buffer_count, song->buffers);
  alDeleteSources(1, &song->source);

  stb_vorbis_close(song->vorbis);

  free(song->buffers);
  free(song->vorbis);

  return 1;
}

a_song* a_song_get_ptr(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_destroy: no song in context with ID %i\n", song_id);
    return 0;
  }

  return &ctx->songs[song_id - 1];
}

a_song* a_song_get_open(a_ctx* ctx) {
  if (ctx->song_count == ctx->song_capacity) {
    ASTERA_DBG("a_song_get_open: no open song slots.\n");
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
  a_layer* target_layer = 0;

  if (layer_id) {
    if (ctx->layer_high >= layer_id - 1) {
      target_layer = &ctx->layers[layer_id - 1];
    }
  }

  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_play: invalid song ID\n");
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  // Apply fx
  if (req->fx_count > 0) {
    for (uint16_t i = 0; i < req->fx_count; ++i) {
      // Make sure it's valid within the range of filters
      if (req->fx[i] < ctx->filter_high) {
        alSource3i(song->source, AL_AUXILIARY_SEND_FILTER,
                   (ALint)ctx->filter_slots[req->fx[i] - 1].al_id, i, 0);
      }
    }
  }

  // Apply filters
  if (req->filter_count > 0) {
    for (uint16_t i = 0; i < req->filter_count; ++i) {
      if (ctx->filter_high < req->filters[i] - 1) {
        alSourcei(song->source, AL_DIRECT_FILTER,
                  ctx->filter_slots[req->filters[i] - 1].al_id);
      }
    }
  }

  song->delta = 0;
  song->req   = req;

  alSourcef(song->source, AL_GAIN, req->gain);
  alSource3f(song->source, AL_POSITION, req->position[0], req->position[1],
             req->position[2]);
  alSource3f(song->source, AL_VELOCITY, req->velocity[0], req->velocity[1],
             req->velocity[2]);

  alSourcePlay(song->source);

  return 1;
}

uint8_t a_song_stop(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_resume: no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourceStop(song->source);

  return 1;
}

uint8_t a_song_pause(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_resume: no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourcePause(song->source);

  return 1;
}

uint8_t a_song_resume(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_resume: no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  alSourcePlay(song->source);

  return 1;
}

time_s a_song_get_time(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_get_time: no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];
  return song->delta;
}

uint32_t a_song_get_sample_count(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_get_time: no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];

  return song->sample_count;
}

time_s a_song_get_length(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_get_length: no song in slot %i\n", song_id);
    return -1;
  }

  a_song* song = &ctx->songs[song_id - 1];
  return (time_s)(song->length);
}

uint8_t a_song_set_time(a_ctx* ctx, uint16_t song_id, time_s from_start) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_set_time: no song in list with id %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  // TODO test this whole thin
  double  conv_time     = from_start * 1000.0;
  int32_t approx_sample = conv_time * song->info.sample_rate;

  if (approx_sample > song->sample_count) {
    ASTERA_DBG("a_song_set_time: sample requested %i out of range of song %i\n",
               approx_sample, song->sample_count);
    return 0;
  }

  stb_vorbis_seek_frame(song->vorbis, approx_sample);

  return 1;
}

uint8_t a_song_reset(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_resume: no song in slot %i\n", song_id);
    return 0;
  }

  a_song* song = &ctx->songs[song_id - 1];

  song->delta         = 0.f;
  song->curr          = 0.f;
  song->sample_offset = 0;

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

ALenum a_song_get_state(a_ctx* ctx, uint16_t song_id) {
  if (ctx->song_high < song_id - 1) {
    ASTERA_DBG("a_song_get_state: no song with ID %i\n", song_id);
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
    ASTERA_DBG("a_song_get_id: no songs in context.\n");
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
    ASTERA_DBG("a_buf_create: no asset passed to load into audio buffer.\n");
    return 0;
  }

  if (ctx->buffer_count == ctx->buffer_capacity) {
    ASTERA_DBG("a_buf_create: no free buffer slots.\n");
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
    ASTERA_DBG("a_buf_create: no free buffer slots.\n");
    return 0;
  }

  alGenBuffers(1, &buffer->buf);

  if (is_ogg) {
    int32_t error;

    stb_vorbis* vorbis = stb_vorbis_open_memory(data, data_length, &error, 0);

    if (!vorbis) {
      ASTERA_DBG(
          "a_buf_create: unable to open vorbis header, vorbis error %i\n",
          error);
      return 0;
    }

    uint16_t* pcm;
    int32_t   format, sample_rate, channels;

    int32_t byte_length = stb_vorbis_decode_memory(data, data_length, &channels,
                                                   &sample_rate, &pcm);

    format = (channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

    buffer->length      = vorbis->total_samples;
    buffer->sample_rate = sample_rate;
    buffer->channels    = channels;

    alBufferData(buffer->buf, format, pcm, byte_length, sample_rate);

    // TODO test if we can free this buffer directly after
    // free(pcm);

    free(vorbis);
  } else {
    int16_t channels    = _a_load_int16(data, 22);
    int32_t sample_rate = _a_load_int32(data, 24);
    int32_t byte_rate   = _a_load_int32(data, 28);
    int32_t bps         = _a_load_int16(data, 34);

    if (strncmp(&data[36], "data", 4) != 0) {
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
      ASTERA_DBG("a_buf_create: Unsupported wave file format.\n");
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
    ASTERA_DBG("a_buf_destroy: no buffer in slot %i\n", buf_id);
    return 0;
  }

  a_buf* buffer = &ctx->buffers[buf_id - 1];
  alDeleteBuffers(1, buffer->buf);
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
    ASTERA_DBG("a_buf_get_id: id is above the high mark for buffers.\n");
    return 0;
  }

  return &ctx->buffers[id - 1];
}

a_buf* a_buf_get_open(a_ctx* ctx) {
  if (ctx->buffer_capacity == ctx->buffer_count) {
    ASTERA_DBG("a_buf_get_open: no open buffers.\n");
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

float a_listener_get_gain(a_ctx* ctx) { return ctx->listener.gain; }

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
  if (type != REVERB || type != EQ) {
    ASTERA_DBG("a_fx_use: invalid type of effect passed.\n");
    return 0;
  }

  a_fx* slot = 0;

  for (uint16_t i = 0; i < ctx->fx_high; ++i) {
    if (ctx->fx_slots[i].effect_id == 0) {
      slot = &ctx->fx_slots[i];
      break;
    }
  }

  int8_t new_high = 0;
  if (!slot) {
    // Check for a free slot after the high water mark
    if (ctx->fx_high < ctx->fx_capacity - 1) {
      slot     = &ctx->fx_slots[ctx->fx_high];
      new_high = 1;
    } else {
      ASTERA_DBG("a_fx_use: unable to allocate new slot for FX.\n");
      return 0;
    }
  }

  ++ctx->fx_count;
  alGenEffects(1, &slot->effect_id);
  alGenAuxiliaryEffectSlots(1, &slot->slot_id);

  switch (type) {
    case EQ: {
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
    case REVERB: {
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
    case NONE:
      ASTERA_DBG("a_fx_create: invalid data type.\n");
      return 0;
  }

  ++ctx->fx_count;
  if (new_high) {
    ctx->fx_high = slot->id - 1;
  }

  // NOTE: slot->id is the index + 1 of the slot in the context's array of
  // slots
  return slot->id;
}

/* Remove an effect & free up the slot
 * ctx - the context to affect
 * fx_id - the ID of the effect
 * returns: success = 1, fail = 0 */
uint8_t a_fx_destroy(a_ctx* ctx, uint16_t fx_id) {
  if (ctx->fx_high < fx_id - 1) {
    ASTERA_DBG("a_fx_remove: no fx in that slot.\n");
    return 0;
  }

  alDeleteEffects(1, &ctx->fx_slots[fx_id - 1].effect_id);
  alDeleteAuxiliaryEffectSlots(1, &ctx->fx_slots[fx_id - 1].slot_id);

  --ctx->fx_count;
  if (ctx->fx_high == fx_id - 1) {
    if (ctx->fx_count > 0) {
      for (uint16_t i = ctx->fx_high; i > 0; --i) {
        if (ctx->fx_slots[i].slot_id) {
          ctx->fx_high = i;
          break;
        }
      }
    }
  }

  return 1;
}

uint8_t a_fx_update(a_ctx* ctx, uint16_t fx_id) {
  if (ctx->fx_high < fx_id - 1) {
    ASTERA_DBG("a_fx_update: no fx in slot %i\n", fx_id);
    return 0;
  }

  a_fx* slot = &ctx->fx_slots[fx_id - 1];
  switch (slot->type) {
    case EQ: {
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
    case REVERB: {
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
}

a_fx_type a_fx_get_type(a_ctx* ctx, uint16_t fx_id) {
  if (ctx->fx_high < fx_id - 1) {
    ASTERA_DBG("a_fx_get_type: no fx in slot %i\n", fx_id);
    return 0;
  }

  return ctx->fx_slots[fx_id - 1].type;
}

a_fx* a_fx_get_slot(a_ctx* ctx, uint16_t fx_id) {
  if (ctx->fx_high < fx_id - 1) {
    ASTERA_DBG("a_fx_get_slot: no fx in slot %i\n", fx_id);
    return 0;
  }

  return &ctx->fx_slots[fx_id - 1];
}

a_fx_reverb a_fx_reverb_default(void) {
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
}

a_fx_reverb a_fx_reverb_create(float density, float diffusion, float gain,
                               float gainhf, float decay, float decay_hfratio,
                               float refl_gain, float refl_delay,
                               float late_gain, float late_delay,
                               float  air_absorption_gainhf,
                               float  room_rolloff_factor,
                               int8_t decay_hflimit) {
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
}

a_fx_eq a_fx_eq_create(float low_gain, float low_cutoff, float mid1_gain,
                       float mid1_center, float mid1_width, float mid2_gain,
                       float mid2_center, float mid2_width, float high_gain,
                       float high_cutoff) {
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
}

uint16_t a_filter_create(a_ctx* ctx, a_filter_type type, float gain, float hf,
                         float lf) {
  a_filter* slot = 0;
  for (uint16_t i = 0; i < ctx->filter_high; ++i) {
    if (!ctx->filter_slots[i].al_id) {
      slot = &ctx->filter_slots[i];
      break;
    }
  }

  if (!slot && ctx->filter_count < ctx->filter_capacity) {
    slot = &ctx->filter_slots[ctx->filter_high];
  } else {
    return 0;
  }

  ++ctx->filter_count;

  if (!slot->al_id) {
    alGenFilters(1, &slot->al_id);
  }

  slot->gain = _a_clamp(gain, 0.f, 1.f, 1.f);
  slot->type = type;

  switch (type) {
    case LOW:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_LOWPASS);

      slot->data.low.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_LOWPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_LOWPASS_GAINHF, slot->data.low.gainhf);

      break;
    case HIGH:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);

      slot->data.high.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_HIGHPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_HIGHPASS_GAINLF, slot->data.high.gainlf);

      break;
    case BAND:
      alFilteri(slot->al_id, AL_FILTER_TYPE, AL_FILTER_BANDPASS);

      slot->data.band.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);
      slot->data.band.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

      alFilterf(slot->al_id, AL_BANDPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_BANDPASS_GAINLF, slot->data.band.gainlf);
      alFilterf(slot->al_id, AL_BANDPASS_GAINHF, slot->data.band.gainhf);
      break;
  }

  return slot->id;
}

uint8_t a_filter_update(a_ctx* ctx, uint16_t filter_id) {
  if (filter_id - 1 > ctx->filter_high) {
    ASTERA_DBG("a_filter_update: no filter in slot %i\n", filter_id);
    return 0;
  }

  a_filter* slot = &ctx->filter_slots[filter_id - 1];

  switch (slot->type) {
    case LOW:
      alFilterf(slot->al_id, AL_LOWPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_LOWPASS_GAINHF, slot->data.low.gainhf);
      break;
    case HIGH:
      alFilterf(slot->al_id, AL_HIGHPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_HIGHPASS_GAINLF, slot->data.high.gainlf);
      break;
    case BAND:
      alFilterf(slot->al_id, AL_BANDPASS_GAIN, slot->gain);
      alFilterf(slot->al_id, AL_BANDPASS_GAINLF, slot->data.band.gainlf);
      alFilterf(slot->al_id, AL_BANDPASS_GAINHF, slot->data.band.gainhf);
      break;
  }

  return 1;
}

uint8_t a_filter_destroy(a_ctx* ctx, uint16_t filter_id) {
  if (ctx->filter_high <= filter_id - 1) {
    ASTERA_DBG("a_filter_get_slot: no filter in slot %i\n", filter_id);
    return 0;
  }

  alDeleteFilters(1, &ctx->filter_slots[filter_id - 1].al_id);
  ctx->filter_slots[filter_id - 1].al_id = 0;
  --ctx->filter_count;

  if (filter_id - 1 == ctx->filter_high) {
    for (uint16_t i = ctx->filter_high; i > 0; --i) {
      if (ctx->filter_slots[i].al_id) {
        ctx->filter_high = i;
        break;
      }
    }
  }

  return 1;
}

a_filter* a_filter_get_slot(a_ctx* ctx, uint16_t filter_id) {
  if (ctx->filter_high <= filter_id - 1) {
    ASTERA_DBG("a_filter_get_slot: no filter in slot %i\n", filter_id);
    return 0;
  }

  return &ctx->filter_slots[filter_id - 1];
}


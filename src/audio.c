/*#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER 0x1012
#define ALC_ALL_DEVICES_SPECIFIER         0x1013
#endif*/

#include "audio.h"

#include "debug.h"

/* Debug Output Macro*/
#if defined(ASTERA_DEBUG_OUTPUT)
#if !defined(DBG_E)
#define DBG_E(fmt, ...) DBGDBG_E(fmt, ##__VA_ARGS_)
#endif
#else
#define DBG_E(fmt, ...)
#endif

#if !defined(ASTERA_AL_DISTANCE_MODEL)
#define ASTERA_AL_DISTANCE_MODEL AL_INVERSE_DISTANCE
#endif

#if !defined(ASTERA_AL_ROLLOFF_FACTOR)
#define ASTERA_AL_ROLLOFF_FACTOR 1.f
#endif

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include <string.h>

#if !defined(ASTERA_AL_NO_EFX)
#include <AL/efx.h>

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

// TODO: Implement more info
void a_efx_info(void) {
  if (alcIsExtensionPresent(g_a_ctx.device, "ALC_EXT_EFX") == AL_FALSE) {
    _l("No ALC_EXT_EFX.\n");
  } else {
    _l("ALC_EXT_EFX Present.\n");
  }

  _l("MAX_AUXILIARY_SENDS: %i\n", ALC_MAX_AUXILIARY_SENDS);
  ALCint s_sends = 0;
  alcGetIntegerv(g_a_ctx.device, ALC_MAX_AUXILIARY_SENDS, 1, &s_sends);
  _l("MAX AUXILIARY SENDS PER SOURCE: %i\n", s_sends);
}

static inline float _a_clamp(float value, float min, float max, float def) {
  return (value == -1.f) ? def
                         : (value < min) ? min : (value > max) ? max : value;
}

int16_t a_fx_slot_attach(a_fx* fx) {
  if (g_a_map.fx_capacity == g_a_map.fx_count || g_a_map.fx_capacity == 0) {
    DBG_E("a_fx_slot_attach: incomplete fx passed.\n");
    return -1;
  }

  for (uint16_t i = 0; i < g_a_map.fx_capacity; ++i) {
    if (!g_a_map.fx_slots[i].fx) {
      fx->slot               = &g_a_map.fx_slots[i];
      g_a_map.fx_slots[i].fx = fx;
      DBG_E("Attaching effect %i to effect slot %i\n", fx->id, i);
      alAuxiliaryEffectSloti(g_a_map.fx_slots[i].id, AL_EFFECTSLOT_EFFECT,
                             fx->id);
      return i;
    }
  }

  return -1;
}

void a_fx_slot_detach(a_fx* fx) {
  if (!fx || !fx->slot) {
    DBG_E("a_fx_slot_detach: incomplete fx struct passed.\n");
    return;
  }

  alAuxiliaryEffectSloti(fx->slot->id, AL_EFFECTSLOT_EFFECT, 0);
  fx->slot->fx = 0;
  fx->slot     = 0;
}

void a_fx_slot_update(a_fx_slot* slot) {
  if (!slot) {
    DBG_E("a_fx_slot_update: no slot passed.\n");
    return;
  }

  alAuxiliaryEffectSlotf(slot->id, AL_EFFECTSLOT_GAIN, slot->gain);
  alAuxiliaryEffectSloti(slot->id, AL_EFFECTSLOT_EFFECT, 0);
  if (slot->fx)
    alAuxiliaryEffectSloti(slot->id, AL_EFFECTSLOT_EFFECT, slot->fx->id);
}

void a_fx_slot_destroy(a_fx_slot* slot) {
  alDeleteAuxiliaryEffectSlots(1, &slot->id);
  if (slot->fx) {
    slot->fx->slot = 0;
    slot->fx       = 0;
  }
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

void a_fx_update(a_fx fx) {
  if (!fx.data) {
    DBG_E("a_fx_update: invalid fx passed.\n");
    return;
  }

  switch (fx.type) {
  case EQ: {
    a_fx_eq* eq = (a_fx_eq*)fx.data;
    alEffectf(fx.id, AL_EQUALIZER_LOW_GAIN, eq->low_gain);
    alEffectf(fx.id, AL_EQUALIZER_LOW_CUTOFF, eq->low_cutoff);
    alEffectf(fx.id, AL_EQUALIZER_MID1_GAIN, eq->mid1_gain);
    alEffectf(fx.id, AL_EQUALIZER_MID1_CENTER, eq->mid1_center);
    alEffectf(fx.id, AL_EQUALIZER_MID1_WIDTH, eq->mid1_width);
    alEffectf(fx.id, AL_EQUALIZER_MID2_GAIN, eq->mid2_gain);
    alEffectf(fx.id, AL_EQUALIZER_MID2_CENTER, eq->mid2_center);
    alEffectf(fx.id, AL_EQUALIZER_MID2_WIDTH, eq->mid2_width);
    alEffectf(fx.id, AL_EQUALIZER_HIGH_GAIN, eq->high_gain);
    alEffectf(fx.id, AL_EQUALIZER_HIGH_CUTOFF, eq->high_cutoff);
  } break;
  case REVERB: {
    a_fx_reverb* rv = (a_fx_reverb*)fx.data;

    alEffectf(fx.id, AL_REVERB_DENSITY, rv->decay);
    alEffectf(fx.id, AL_REVERB_DIFFUSION, rv->diffusion);
    alEffectf(fx.id, AL_REVERB_GAIN, rv->gain);
    alEffectf(fx.id, AL_REVERB_GAINHF, rv->gainhf);
    alEffectf(fx.id, AL_REVERB_DECAY_TIME, rv->decay);
    alEffectf(fx.id, AL_REVERB_DECAY_HFRATIO, rv->decay_hfratio);
    alEffectf(fx.id, AL_REVERB_REFLECTIONS_GAIN, rv->refl_gain);
    alEffectf(fx.id, AL_REVERB_REFLECTIONS_DELAY, rv->refl_delay);
    alEffectf(fx.id, AL_REVERB_LATE_REVERB_GAIN, rv->late_gain);
    alEffectf(fx.id, AL_REVERB_LATE_REVERB_DELAY, rv->late_delay);
    alEffectf(fx.id, AL_REVERB_AIR_ABSORPTION_GAINHF,
              rv->air_absorption_gainhf);
    alEffectf(fx.id, AL_REVERB_ROOM_ROLLOFF_FACTOR, rv->room_rolloff_factor);
    alEffecti(fx.id, AL_REVERB_DECAY_HFLIMIT, rv->decay_hflimit);

  } break;
  default:
    break;
  }
}

a_fx a_fx_create(a_fx_type type, void* data) {
  a_fx fx = (a_fx){0};

  if (!data) {
    DBG_E("a_fx_create: no data passed.\n");
    return fx;
  }

  switch (type) {
  case EQ:
    alGenEffects(1, &fx.id);
    alEffecti(fx.id, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
    fx.data = data;
    fx.type = type;

    a_fx_eq* eq = (a_fx_eq*)fx.data;
    alEffectf(fx.id, AL_EQUALIZER_LOW_GAIN, eq->low_gain);
    alEffectf(fx.id, AL_EQUALIZER_LOW_CUTOFF, eq->low_cutoff);
    alEffectf(fx.id, AL_EQUALIZER_MID1_GAIN, eq->mid1_gain);
    alEffectf(fx.id, AL_EQUALIZER_MID1_CENTER, eq->mid1_center);
    alEffectf(fx.id, AL_EQUALIZER_MID1_WIDTH, eq->mid1_width);
    alEffectf(fx.id, AL_EQUALIZER_MID2_GAIN, eq->mid2_gain);
    alEffectf(fx.id, AL_EQUALIZER_MID2_CENTER, eq->mid2_center);
    alEffectf(fx.id, AL_EQUALIZER_MID2_WIDTH, eq->mid2_width);
    alEffectf(fx.id, AL_EQUALIZER_HIGH_GAIN, eq->high_gain);
    alEffectf(fx.id, AL_EQUALIZER_HIGH_CUTOFF, eq->high_cutoff);

    break;
  case REVERB:
    alGenEffects(1, &fx.id);

    alEffecti(fx.id, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    fx.data = data;
    fx.type = type;

    a_fx_reverb* rv = (a_fx_reverb*)fx.data;

    alEffectf(fx.id, AL_REVERB_DENSITY, (ALfloat)rv->density);
    alEffectf(fx.id, AL_REVERB_DIFFUSION, (ALfloat)rv->diffusion);
    alEffectf(fx.id, AL_REVERB_GAIN, AL_REVERB_DEFAULT_GAIN);
    alEffectf(fx.id, AL_REVERB_GAINHF, AL_REVERB_DEFAULT_GAINHF);
    alEffectf(fx.id, AL_REVERB_DECAY_TIME, rv->decay);
    alEffectf(fx.id, AL_REVERB_DECAY_HFRATIO, rv->decay_hfratio);
    alEffectf(fx.id, AL_REVERB_REFLECTIONS_GAIN, rv->refl_gain);
    alEffectf(fx.id, AL_REVERB_REFLECTIONS_DELAY, rv->refl_delay);
    alEffectf(fx.id, AL_REVERB_LATE_REVERB_GAIN, rv->late_gain);
    alEffectf(fx.id, AL_REVERB_LATE_REVERB_DELAY, rv->late_delay);
    alEffectf(fx.id, AL_REVERB_AIR_ABSORPTION_GAINHF,
              rv->air_absorption_gainhf);
    alEffectf(fx.id, AL_REVERB_ROOM_ROLLOFF_FACTOR, rv->room_rolloff_factor);
    alEffecti(fx.id, AL_REVERB_DECAY_HFLIMIT, rv->decay_hflimit);

    break;
  default:
    return fx;
    break;
  }
  return fx;
}

void a_fx_destroy(a_fx fx) {
  alDeleteEffects(1, &fx.id);
}

a_filter a_filter_create(a_filter_type type, float gain, float hf, float lf) {
  a_filter fl = (a_filter){0};
  switch (type) {
  case LOW:
    alGenFilters(1, &fl.id);
    alFilteri(fl.id, AL_FILTER_TYPE, AL_FILTER_LOWPASS);

    fl.data.low.gain   = _a_clamp(gain, 0.f, 1.f, 1.f);
    fl.data.low.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

    alFilterf(fl.id, AL_LOWPASS_GAIN, fl.data.low.gain);
    alFilterf(fl.id, AL_LOWPASS_GAINHF, fl.data.low.gainhf);

    break;
  case HIGH:
    alGenFilters(1, &fl.id);
    alFilteri(fl.id, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);

    fl.data.high.gain   = _a_clamp(gain, 0.f, 1.f, 1.f);
    fl.data.high.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);

    alFilterf(fl.id, AL_HIGHPASS_GAIN, fl.data.high.gain);
    alFilterf(fl.id, AL_HIGHPASS_GAINLF, fl.data.high.gainlf);

    break;
  case BAND:
    alGenFilters(1, &fl.id);
    alFilteri(fl.id, AL_FILTER_TYPE, AL_FILTER_BANDPASS);

    fl.data.band.gain   = _a_clamp(gain, 0.f, 1.f, 1.f);
    fl.data.band.gainlf = _a_clamp(lf, 0.f, 1.f, 1.f);
    fl.data.band.gainhf = _a_clamp(hf, 0.f, 1.f, 1.f);

    alFilterf(fl.id, AL_BANDPASS_GAIN, fl.data.band.gain);
    alFilterf(fl.id, AL_BANDPASS_GAINLF, fl.data.band.gainlf);
    alFilterf(fl.id, AL_BANDPASS_GAINHF, fl.data.band.gainhf);
    break;
  default:
    return fl;
  }

  fl.type = type;

  return fl;
}

void a_filter_update(a_filter fl) {
  switch (fl.type) {
  case LOW:
    alFilterf(fl.id, AL_LOWPASS_GAIN, fl.data.low.gain);
    alFilterf(fl.id, AL_LOWPASS_GAINHF, fl.data.low.gainhf);
    break;
  case HIGH:
    alFilterf(fl.id, AL_HIGHPASS_GAIN, fl.data.high.gain);
    alFilterf(fl.id, AL_HIGHPASS_GAINLF, fl.data.high.gainlf);

    break;
  case BAND:
    alFilterf(fl.id, AL_BANDPASS_GAIN, fl.data.band.gain);
    alFilterf(fl.id, AL_BANDPASS_GAINLF, fl.data.band.gainlf);
    alFilterf(fl.id, AL_BANDPASS_GAINHF, fl.data.band.gainhf);
    break;
  default:
    DBG_E("a_filter_update: invalid filter type passed.\n");
    break;
  }
}

void a_filter_destroy(a_filter filter) {
  alDeleteFilters(1, &filter.id);
}

int a_can_play(void) {
  return g_a_ctx.allow;
}

int a_init(const char* device, uint32_t master, uint32_t sfx, uint32_t music) {
  if (!a_ctx_create(device)) {
    DBG_E("Unable to create audio context.\n");
    return -1;
  }

  g_a_map.layer_count = MAX_AUDIO_LAYERS;

#if defined(ASTERA_AL_DISTANCE_MODEL)
  alDistanceModel(ASTERA_AL_DISTANCE_MODEL);
#endif

#if !defined(ASTERA_AUDIO_NO_EFX)
  if (g_a_ctx.max_fx > 0) {
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

    g_a_map.fx_slots = (a_fx_slot*)malloc(sizeof(a_fx_slot) * MAX_FX);

    if (!g_a_map.fx_slots) {
      DBG_E("a_init: unable to allocate %i fx slots\n", g_a_map.max_fx);
      a_exit();
      return -1;
    }

    for (uint16_t i = 0; i < g_a_ctx.max_fx; ++i) {
      alGenAuxiliaryEffectSlots(1, &g_a_map.fx_slots[i].id);
    }

    g_a_map.fx_capacity = MAX_FX;
    g_a_map.fx_count    = 0;
  }
#endif

  // Initialize cached audio subsystem
  for (int i = 0; i < MAX_SFX; ++i) {
    memset(&g_a_map.sfx[i], 0, sizeof(a_sfx));
    alGenSources(1, &g_a_map.sfx[i].id);

    g_a_map.sfx[i].range = DEFAULT_SFX_RANGE;
    g_a_map.sfx[i].gain  = 1.f;

    alSourcef(g_a_map.sfx[i].id, AL_GAIN, 1.f);

    alSourcef(g_a_map.sfx[i].id, AL_MAX_DISTANCE, g_a_map.sfx[i].range);
    alSourcef(g_a_map.sfx[i].id, AL_REFERENCE_DISTANCE,
              g_a_map.sfx[i].range / 2.f);
    alSourcef(g_a_map.sfx[i].id, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);

    alSource3f(g_a_map.sfx[i].id, AL_POSITION, g_a_map.sfx[i].position[0],
               g_a_map.sfx[i].position[2], g_a_map.sfx[i].position[1]);
    g_a_map.sfx[i].has_req = 0;
  }

  g_a_map.sfx_count = MAX_SFX;

  unsigned int sources[MAX_SONGS];
  unsigned int buffers[MAX_SONGS * AUDIO_BUFFERS_PER_MUSIC];

  alGenSources(MAX_SONGS, sources);
  alGenBuffers(MAX_SONGS * AUDIO_BUFFERS_PER_MUSIC, buffers);

  for (int i = 0; i < MAX_SONGS; ++i) {
    g_a_map.songs[i].source = sources[i];

    for (int j = 0; j < AUDIO_BUFFERS_PER_MUSIC; ++j) {
      g_a_map.songs[i].buffers[j] = buffers[(i * AUDIO_BUFFERS_PER_MUSIC) + j];
    }

    g_a_map.songs[i].packets_per_buffer = AUDIO_DEFAULT_FRAMES_PER_BUFFER;
    g_a_map.songs[i].gain               = 1.f;
    g_a_map.songs[i].has_req            = 0;
  }

  g_a_map.song_count = MAX_SONGS;

  float master_f  = master / 100.f;
  g_listener.gain = master_f;
  alListenerf(AL_GAIN, g_listener.gain);
  a_set_pos(g_listener.pos);
  a_set_ori(g_listener.ori);

  for (int i = 0; i < MAX_AUDIO_LAYERS; ++i) {
    g_a_map.layers[i].id   = i;
    g_a_map.layers[i].gain = 1.f;
  }

#ifdef AUDIO_MUSIC_LAYER
  if (g_a_map.layer_count > AUDIO_MUSIC_LAYER) {
    g_a_map.layers[AUDIO_MUSIC_LAYER].gain = (music / 100.f);
  } else {
    DBG_E("Not enough layers generated to set volume for music layer on: %i\n",
          AUDIO_MUSIC_LAYER);
  }
#endif
#ifdef AUDIO_SFX_LAYER
  if (g_a_map.layer_count > AUDIO_SFX_LAYER) {
    g_a_map.layers[AUDIO_SFX_LAYER].gain = (sfx / 100.f);
  } else {
    DBG_E("Not enough layers generated to set volume for sfx layer on: %i\n",
          AUDIO_SFX_LAYER);
  }
#endif
#ifdef AUDIO_MISC_LAYER
  if (g_a_map.layer_count > AUDIO_MISC_LAYER) {
    g_a_map.layers[AUDIO_MISC_LAYER].gain = AUDIO_MISC_GAIN;
  } else {
    DBG_E("Not enough layers generated to set volume for misc layer on: %i\n",
          AUDIO_MISC_LAYER);
  }
#endif
#ifdef AUDIO_UI_LAYER
  if (g_a_map.layer_count > AUDIO_UI_LAYER) {
    g_a_map.layers[AUDIO_UI_LAYER].gain = AUDIO_UI_GAIN;
  } else {
    DBG_E("Not enough layers generated to set volume for UI layer on: %i\n",
          AUDIO_UI_LAYER);
  }
#endif
  g_a_ctx.allow = 1;
  return 1;
}

void a_set_pos(vec3 p) {
  vec3_dup(g_listener.pos, p);
  alListener3f(AL_POSITION, p[0], p[2], p[1]);
}

void a_set_ori(vec3 d) {
  g_listener._ori[0] = d[0];
  g_listener._ori[1] = d[1];
  g_listener.ori[2]  = d[2];
  vec3_dup(g_listener.ori, d);
  alListenerfv(AL_ORIENTATION, g_listener._ori);
}

void a_set_orif(float v[6]) {
  if (!v) {
    return;
  }

  memcpy(g_listener._ori, v, sizeof(float) * 6);
  g_listener.ori[0] = v[0];
  g_listener.ori[1] = v[1];
  g_listener.ori[2] = v[2];
  alListener3f(AL_VELOCITY, v[0], v[1], v[2]);
}

void a_set_vel(vec3 v) {
  vec3_dup(g_listener.vel, v);
  alListener3f(AL_VELOCITY, v[0], v[1], v[2]);
}

void a_get_pos(vec3* p) {
  if (p) {
    vec3_dup(*p, g_listener.pos);
  }
}

void a_get_ori(vec3* o) {
  if (o) {
    vec3_dup(*o, g_listener.ori);
  }
}

void a_get_orif(float* f) {
  if (f) {
    memcpy(f, g_listener._ori, sizeof(float) * 6);
  }
}

void a_get_vel(vec3* v) {
  if (v) {
    vec3_dup(*v, g_listener.vel);
  }
}

void a_set_vol(uint32_t master, uint32_t sfx, uint32_t music) {
  a_set_vol_master(master);
  a_set_vol_sfx(sfx);
  a_set_vol_music(music);
}

void a_set_vol_master(uint32_t master) {
  g_listener.gain = master / 100.f;
  alListenerf(AL_GAIN, g_listener.gain);
}

void a_set_vol_sfx(uint32_t sfx) {
#ifdef AUDIO_SFX_LAYER
  if (g_a_map.layer_count > AUDIO_SFX_LAYER) {
    float value = sfx / 100.f;
    if (g_a_map.layers[AUDIO_SFX_LAYER].gain != value) {
      g_a_map.layers[AUDIO_SFX_LAYER].gain        = value;
      g_a_map.layers[AUDIO_SFX_LAYER].gain_change = 1;
    }
  } else {
    DBG_E("Unable to set SFX vol on layer out of bounds: %i\n",
          AUDIO_SFX_LAYER);
  }
#endif
}

void a_set_vol_music(uint32_t music) {
#ifdef AUDIO_MUSIC_LAYER
  if (g_a_map.layer_count > AUDIO_MUSIC_LAYER) {
    float value = (music / 100.f);
    if (value != g_a_map.layers[AUDIO_MUSIC_LAYER].gain) {
      g_a_map.layers[AUDIO_MUSIC_LAYER].gain        = value;
      g_a_map.layers[AUDIO_MUSIC_LAYER].gain_change = 1;
    }
  } else {
    DBG_E("Unable to set music vol on layer out of bounds: %i\n",
          AUDIO_MUSIC_LAYER);
  }
#endif
}

float a_get_vol_master(void) {
  return g_listener.gain;
}

float a_get_vol_sfx(void) {
#ifdef AUDIO_SFX_LAYER
  if (AUDIO_SFX_LAYER < g_a_map.layer_count) {
    return g_a_map.layers[AUDIO_SFX_LAYER].gain;
  } else {
    DBG_E("Unable to get volume for SFX layer, [%i] out of bounds.\n",
          AUDIO_SFX_LAYER);
    return 0.f;
  }
#else
  return 0.f;
#endif
}

float a_get_vol_music(void) {
#ifdef AUDIO_MUSIC_LAYER
  if (AUDIO_MUSIC_LAYER < g_a_map.layer_count) {
    return g_a_map.layers[AUDIO_MUSIC_LAYER].gain;
  } else {
    DBG_E("Unable to get volume for music layer, [%i] out of bounds.\n",
          AUDIO_MUSIC_LAYER);
    return 0.f;
  }
#else
  return 0.f;
#endif
}

void a_exit(void) {
  if (!g_a_ctx.context) {
    return;
  }

  for (int i = 0; i < g_a_map.song_count; ++i) {
    alDeleteSources(1, &g_a_map.songs[i].source);
    alDeleteBuffers(AUDIO_BUFFERS_PER_MUSIC,
                    (const ALuint*)&g_a_map.songs[i].buffers);
  }

  for (int i = 0; i < g_a_map.sfx_count; ++i) {
    alDeleteSources(1, &g_a_map.sfx[i].id);
  }

  for (uint16_t i = 0; i < g_a_map.fx_capacity; ++i) {
    if (g_a_map.fx_slots[i].id != 0) {
      alDeleteAuxiliaryEffectSlots(1, &g_a_map.fx_slots[i].id);
    }
  }

  alcMakeContextCurrent(0);
  alcDestroyContext(g_a_ctx.context);
  alcCloseDevice(g_a_ctx.device);
}

void a_update(time_s delta) {
  for (int i = 0; i < g_a_map.layer_count; ++i) {
    a_layer* layer = &g_a_map.layers[i];

    for (int j = 0; j < layer->sfx_count; ++j) {
      a_sfx* sfx = layer->sources[j];
      if (sfx->has_req) {
        a_req* req = sfx->req;
        if (sfx->gain != req->gain || layer->gain_change) {
          float n_gain = layer->gain * req->gain;
          sfx->gain    = req->gain;
          alSourcef(sfx->id, AL_GAIN, n_gain);
        }

        if (!vec3_cmp(sfx->position, req->pos)) {
          vec3_dup(sfx->position, req->pos);
          alSource3f(sfx->id, AL_POSITION, req->pos[0], req->pos[2],
                     req->pos[1]);
        }

        ALenum state;
        alGetSourcei(sfx->id, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
          alSourcei(sfx->id, AL_BUFFER, 0);

          // TODO Effect path filtering
#if !defined(ASTERA_AL_NO_EFX)
          if (sfx->req->fx_count > 0 && sfx->req->fx) {
            for (uint16_t e = 0; e < sfx->req->fx_count; ++e) {
              alSource3i(sfx->id, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL,
                         e, 0);
            }
          }

          if (sfx->req->filter) {
            alSourcei(sfx->id, AL_DIRECT_FILTER, AL_FILTER_NULL);
          }
#endif

          sfx->req     = 0;
          sfx->has_req = 0;
        } else if (req) {
          if (req->stop) {
            req->playing = 0;
            alSourcei(g_a_map.sfx[i].id, AL_BUFFER, 0);

#if !defined(ASTERA_AL_NO_EFX)
            if (sfx->req->fx_count > 0 && sfx->req->fx) {
              for (uint16_t e = 0; e < sfx->req->fx_count; ++e) {
                alSource3i(sfx->id, AL_AUXILIARY_SEND_FILTER,
                           AL_EFFECTSLOT_NULL, e, 0);
              }
            }

            if (sfx->req->filter) {
              alSourcei(sfx->id, AL_DIRECT_FILTER, AL_FILTER_NULL);
            }
#endif

            sfx->req     = 0;
            sfx->has_req = 0;
          } else {
            req->playing = 1;
          }
        }
      }
    }

    for (int j = 0; j < MAX_LAYER_SONGS; ++j) {
      a_music* mus = layer->musics[j];
      if (mus) {
        if (mus->req) {
          a_req* req = mus->req;

          if (req->range != 0.f) {
            alSourcef(mus->source, AL_MAX_DISTANCE, req->range);
            alSourcef(mus->source, AL_REFERENCE_DISTANCE, req->range / 2.f);
            alSourcef(mus->source, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);
          } else {
            alSourcef(mus->source, AL_MAX_DISTANCE, 0.f);
          }

          float n_gain = layer->gain * req->gain;
          alSourcef(mus->source, AL_GAIN, n_gain);

          alSource3f(mus->source, AL_POSITION, req->pos[0], req->pos[2],
                     req->pos[1]);

          ALenum state;
          ALint  proc;
          alGetSourcei(mus->source, AL_SOURCE_STATE, &state);
          alGetSourcei(mus->source, AL_BUFFERS_PROCESSED, &proc);

          if (state == AL_PLAYING) {
            mus->req->playing = 1;
          } else {
            mus->req->playing = 0;
          }

          if (req->stop) {
            alSourcei(mus->source, AL_BUFFER, 0);
            alSourceStop(mus->source);
            mus->req->playing = 0;
            mus->req          = 0;
            mus->has_req      = 0;
          } else if (proc > 0) {
            if (mus->data_length == mus->data_offset) {
              if (mus->loop) {
                mus->data_offset = mus->header_end;
              } else {
                mus->req->stop = 1;
                break;
              }
            }
            int max_samples;
            for (int k = 0; k < proc; ++k) {
              if (mus->samples_left > AUDIO_FRAME_SIZE) {
                max_samples = AUDIO_FRAME_SIZE;
              } else {
                max_samples = mus->samples_left;
              }

              uint32_t buffer;
              int32_t  al_err;

              alSourceUnqueueBuffers(mus->source, 1, &buffer);

              if (state != AL_PLAYING && !req->stop && k == 0) {
                alSourcePlay(mus->source);
              }

              memset(mus->pcm, 0, mus->pcm_length * sizeof(short));
              int32_t pcm_total_length = 0;
              int32_t pcm_index = 0, frame_size = 0;
              int32_t bytes_used = 0, num_samples = 0, num_channels = 0;
              float** out;

              int fail_counter = 0;
              for (int p = 0; p < mus->packets_per_buffer; ++p) {
                frame_size = mus->data_length - mus->data_offset;
                if (frame_size > AUDIO_FRAME_SIZE)
                  frame_size = AUDIO_FRAME_SIZE;

#if defined(AUDIO_MUSIC_MAX_FAILS)
                if (fail_counter >= AUDIO_MUSIC_MAX_FAILS)
                  break;
#endif

                bytes_used = stb_vorbis_decode_frame_pushdata(
                    mus->vorbis, mus->data + mus->data_offset, frame_size,
                    &num_channels, &out, &num_samples);
                if (!bytes_used) {
                  DBG_E("Unable to load samples from [%i] bytes.\n",
                        frame_size);
                  ++fail_counter;
                  continue;
                }

                mus->data_offset += bytes_used;

                if (num_samples > 0) {
                  int short_count = num_samples * num_channels;
                  int pcm_length  = sizeof(short) * short_count;
                  pcm_total_length += pcm_length;
                  if (pcm_length + pcm_index > mus->pcm_length) {
                    DBG_E("Uhhh.\n");
                    break;
                  }

                  for (int s = 0; s < num_samples; ++s) {
                    for (int c = 0; c < num_channels; ++c) {
                      mus->pcm[pcm_index] = out[c][s] * 32767;
                      ++pcm_index;
                    }
                  }
                }
              }

              alBufferData(buffer, mus->format, mus->pcm, pcm_total_length,
                           mus->vorbis->sample_rate);

              if ((al_err = alGetError()) != AL_INVALID_VALUE) {
                alSourceQueueBuffers(mus->source, 1, &buffer);
              } else {
                DBG_E("Unable to unqueue audio buffer for music err [%i]: %i\n",
                      al_err, buffer);
              }
            }

            if (proc == AUDIO_BUFFERS_PER_MUSIC) {
              alSourcePlay(mus->source);
            }
          }
        }
      }
    }
  }
}

uint32_t a_get_device_name(char* dst, int capacity) {
  const ALchar* name;

  if (g_a_ctx.device) {
    name = alcGetString(g_a_ctx.device, ALC_DEVICE_SPECIFIER);
  } else {
    name = alcGetString(0, ALC_DEFAULT_DEVICE_SPECIFIER);
  }

  int length = strlen(name);
  int limit  = (length > capacity) ? capacity - 1 : length;
  memcpy(dst, name, sizeof(char) * limit);
  dst[limit] = '\0';

  return limit;
}

int8_t a_layer_add_music(uint32_t id, a_music* music) {
  a_layer* layer = 0;
  for (int i = 0; i < g_a_map.layer_count; ++i) {
    if (g_a_map.layers[i].id == id) {
      layer = &g_a_map.layers[i];
      break;
    }
  }

  if (!layer) {
    DBG_E("Unable to find layer: %i\n", id);
    return -1;
  }

  if (layer->music_count == MAX_LAYER_SONGS) {
    DBG_E("No space left in layer: %i\n", id);
    return -1;
  }

  for (int i = 0; i < MAX_LAYER_SONGS; ++i) {
    if (!layer->musics[i]) {
      layer->musics[i] = music;
      ++layer->music_count;
      return 1;
    }
  }

  return 0;
}

int8_t a_layer_add_sfx(uint32_t id, a_sfx* sfx) {
  a_layer* layer = 0;
  for (int i = 0; i < g_a_map.layer_count; ++i) {
    if (g_a_map.layers[i].id == id) {
      layer = &g_a_map.layers[i];
      break;
    }
  }

  if (!layer) {
    return -1;
  }

  if (layer->sfx_count == MAX_LAYER_SFX) {
    return -1;
  }

  for (int i = 0; i < MAX_LAYER_SFX; ++i) {
    if (!layer->sources[i]) {
      layer->sources[i] = sfx;
      ++layer->sfx_count;
      return 1;
    }
  }

  return 0;
}

a_req a_req_create(uint16_t layer, vec3 pos, float gain, int8_t loop) {
  a_req req = (a_req){0};
  vec3_dup(req.pos, pos);
  req.gain  = gain;
  req.layer = layer;
  vec3_clear(req.vel);
  req.loop = loop ? 1 : 0;
  req.stop = 0;
  req.time = 0;
  return req;
}

int a_ctx_create(const char* device_name) {
  ALCdevice* device = 0;

  if (device_name) {
    device = alcOpenDevice(device_name);
  } else {
    device = alcOpenDevice(0);
  }

#if !defined(ASTERA_AUDIO_NO_EFX)
  if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_FALSE) {
    g_a_ctx.efx = 0;
  } else {
    g_a_ctx.efx = 1;
  }

  ALint attribs[4] = {0};

  g_a_ctx.max_fx = MAX_FX;

  if (g_a_ctx.efx) {
    attribs[0] = ALC_MAX_AUXILIARY_SENDS;
    attribs[1] = 4;
  }

  ALCcontext* context = alcCreateContext(device, attribs);
#else
  ALCcontext* context = alcCreateContext(device, 0);
#endif

  if (!alcMakeContextCurrent(context)) {
    DBG_E("Error creating OpenAL Context\n");
    return 0;
  }

  g_a_ctx.context = context;
  g_a_ctx.device  = device;

#if !defined(ASTERA_AUDIO_NO_EFX)
  alcGetIntegerv(g_a_ctx.device, ALC_MAX_AUXILIARY_SENDS, 1,
                 &g_a_ctx.fx_per_source);

  if (g_a_ctx.fx_per_source == 0) {
    DBG_E("a_ctx_create: 0 effects allowed per source.\n");
  }
#endif

  DBG_E("a_ctx_create: context created.\n");

  return 1;
}

static int16_t a_load_int16(asset_t* asset, int offset) {
  return *((int16_t*)&asset->data[offset]);
}

static int32_t a_load_int32(asset_t* asset, int offset) {
  return *((int32_t*)&asset->data[offset]);
}

a_buf a_buf_create(asset_t* asset) {
  if (!asset || !asset->data) {
    DBG_E("No asset passed to load into audio buffer.\n");
    return (a_buf){0};
  }

  // format, don't need to use it at the moment tho
  // int16_t format          = a_load_int16(asset, 20);
  int16_t channels        = a_load_int16(asset, 22);
  int32_t sample_rate     = a_load_int32(asset, 24);
  int32_t byte_rate       = a_load_int32(asset, 28);
  int16_t bits_per_sample = a_load_int16(asset, 34);

  // NOTE: Just tests if it's valid file
  assert(strncmp(&asset->data[36], "data", 4) == 0);

  int32_t byte_length = a_load_int32(asset, 40);

  int al_format = -1;
  if (channels == 2) {
    if (bits_per_sample == 16) {
      al_format = AL_FORMAT_STEREO16;
    } else if (bits_per_sample == 8) {
      al_format = AL_FORMAT_STEREO8;
    }
  } else if (channels == 1) {
    if (bits_per_sample == 16) {
      al_format = AL_FORMAT_MONO16;
    } else if (bits_per_sample == 8) {
      al_format = AL_FORMAT_MONO8;
    }
  }

  if (al_format == -1) {
    DBG_E("Unsupported wave file format.\n");
    return (a_buf){0};
  }

  a_buf buffer;
  alGenBuffers(1, &buffer.id);

  buffer.channels    = channels;
  buffer.sample_rate = sample_rate;
  buffer.length      = (float)(byte_length / byte_rate);

  alBufferData(buffer.id, al_format, &asset->data[44], byte_length,
               sample_rate);

  return buffer;
}

void a_buf_destroy(a_buf buffer) {
  int rm_index = -1;
  for (int i = 0; i < g_a_map.buf_count; ++i) {
    if (g_a_map.bufs[i].id == buffer.id) {
      rm_index = i;
      break;
    }
  }

  if (rm_index != -1) {
    int end = (g_a_map.buf_count == g_a_map.buf_capacity)
                  ? g_a_map.buf_capacity - 1
                  : g_a_map.buf_count;
    for (int i = rm_index; i < end; ++i) {
      g_a_map.bufs[i] = g_a_map.bufs[i + 1];
    }

    for (int i = rm_index; i < end; ++i) {
      g_a_map.buf_names[i] = g_a_map.buf_names[i + 1];
    }

    if (end == g_a_map.buf_capacity - 1) {
      g_a_map.bufs[end + 1].id   = 0;
      g_a_map.buf_names[end + 1] = 0;
    }
  }

  alDeleteBuffers(1, &buffer.id);
}

a_buf* a_buf_get(const char* name) {
  if (!name) {
    DBG_E("Unable to get null name buffer.\n");
    return 0;
  }

  for (int i = 0; i < g_a_map.buf_count; ++i) {
    if (strcmp(g_a_map.buf_names[i], name) == 0) {
      return &g_a_map.bufs[i];
    }
  }

  DBG_E("No buffer found named: %s\n", name);
  return 0;
}

a_sfx* a_play_sfxn(const char* name, a_req* req) {
  if (!name) {
    DBG_E("No buffer name passed to play.\n");
    return 0;
  }

  a_buf* buff = a_buf_get(name);

  int index = -1;
  for (int i = 0; i < MAX_SFX; ++i) {
    if (!g_a_map.sfx[i].has_req) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    return 0;
  }

  a_sfx* src = &g_a_map.sfx[index];

  alSourcei(src->id, AL_BUFFER, buff->id);

  if (req) {
    alSource3f(src->id, AL_POSITION, req->pos[0], req->pos[2], req->pos[1]);
    alSourcefv(src->id, AL_VELOCITY, req->vel);

    // Adjust to the layer's gain
    float layer_gain = g_a_map.layers[req->layer].gain;
    alSourcef(src->id, AL_GAIN, req->gain * layer_gain);
    // TODO implement range
    alSourcef(src->id, AL_MAX_DISTANCE, src->range);
    alSourcef(src->id, AL_REFERENCE_DISTANCE, src->range / 2.f);
    alSourcef(src->id, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);
    alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);

    // TODO Effect path filtering
#if !defined(ASTERA_AL_NO_EFX)
    if (req->fx && req->fx_count > 0) {
      for (uint16_t e = 0; e < req->fx_count; ++e) {
        if (req->fx[e]->slot) {
          alSource3i(src->id, AL_AUXILIARY_SEND_FILTER,
                     (ALint)req->fx[e]->slot->id, e, 0);
        }
      }
    }

    if (req->filter) {
      alSourcei(src->id, AL_DIRECT_FILTER, req->filter->id);
    }
#endif

  } else {
    alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
    alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});

    // Adjust to the layer's gain
    alSourcef(src->id, AL_GAIN, 1.f);
    alSourcei(src->id, AL_LOOPING, AL_FALSE);
    alSourcef(src->id, AL_MAX_DISTANCE, src->range);
    alSourcef(src->id, AL_REFERENCE_DISTANCE, src->range / 2.f);
    alSourcef(src->id, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);
  }

  alSourcePlay(src->id);
  return src;
}

a_sfx* a_play_sfx(a_buf* buff, a_req* req) {
  if (!buff) {
    DBG_E("No buffer passed to play.\n");
    return 0;
  }

  int index = -1;
  for (int i = 0; i < MAX_SFX; ++i) {
    if (!g_a_map.sfx[i].has_req) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    return 0;
  }

  a_sfx* src = &g_a_map.sfx[index];

  alSourcei(src->id, AL_BUFFER, buff->id);

  if (req) {
    alSource3f(src->id, AL_POSITION, req->pos[0], req->pos[2], req->pos[1]);
    alSourcefv(src->id, AL_VELOCITY, req->vel);

    // Adjust to the layer's gain
    float layer_gain = g_a_map.layers[req->layer].gain;
    alSourcef(src->id, AL_GAIN, req->gain * layer_gain);

    alSourcef(src->id, AL_MAX_DISTANCE, src->range);
    alSourcef(src->id, AL_REFERENCE_DISTANCE, src->range / 2.f);
    alSourcef(src->id, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);

    alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);

    // TODO Effect path filtering
#if !defined(ASTERA_AL_NO_EFX)
    if (req->fx && req->fx_count > 0) {
      for (uint16_t e = 0; e < req->fx_count; ++e) {
        if (req->fx[e]->slot) {
          alSource3i(src->id, AL_AUXILIARY_SEND_FILTER,
                     (ALint)req->fx[e]->slot->id, e, 0);
        }
      }
    }

    if (req->filter) {
      alSourcei(src->id, AL_DIRECT_FILTER, req->filter->id);
    }
#endif

  } else {
    alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
    alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});

    // Adjust to the layer's gain
    alSourcef(src->id, AL_GAIN, 1.f);

    alSourcei(src->id, AL_LOOPING, AL_FALSE);
    alSourcef(src->id, AL_MAX_DISTANCE, src->range);
    alSourcef(src->id, AL_REFERENCE_DISTANCE, src->range / 2.f);
    alSourcef(src->id, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);
  }

  alSourcePlay(src->id);
  return src;
}

a_music* a_music_create(asset_t* asset, a_meta* meta, a_req* req) {
  if (!asset) {
    DBG_E("No data passed to create music.\n");
    return 0;
  }

  // Get the first free open song slot
  int index = -1;
  for (int i = 0; i < MAX_SONGS; ++i) {
    if (!g_a_map.songs[i].req && g_a_map.songs[i].source != 0) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    DBG_E("No available music slots.\n");
    return 0;
  }

  a_music* music = &g_a_map.songs[index];

  int error, used;

  music->data_length = asset->data_length;
  music->data        = asset->data;
  music->vorbis      = stb_vorbis_open_pushdata(asset->data, asset->data_length,
                                           &used, &error, 0);

  if (!music->vorbis) {
    music->data_length = 0;
    music->data        = 0;
    music->req         = 0;
    DBG_E("Unable to load vorbis, that sucks.\n");
    return 0;
  }

  music->header_end = used;
  music->data_offset += used;

  stb_vorbis_info info = stb_vorbis_get_info(music->vorbis);
  music->sample_rate   = info.sample_rate;

  if (info.channels > 2) {
    DBG_E("Invalid channel size for audio system: %i.\n", info.channels);
    music->data_length = 0;
    music->data        = 0;
    music->req         = 0;
    return 0;
  }

  music->format = (info.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  music->pcm_length =
      AUDIO_FRAME_SIZE * music->packets_per_buffer * info.channels;

  music->pcm = malloc(sizeof(short) * music->pcm_length);

  if (!music->pcm) {
    music->data_length = 0;
    music->data        = 0;
    music->req         = 0;
    DBG_E("Unable to allocate %i shorts.\n", music->pcm_length);
    return 0;
  }

  if (req) {
#if !defined(ASTERA_AL_NO_EFX)
    if (req->fx && req->fx_count > 0) {
      for (uint16_t e = 0; e < req->fx_count; ++e) {
        if (req->fx[e]->slot) {
          alSource3i(music->source, AL_AUXILIARY_SEND_FILTER,
                     (ALint)req->fx[e]->slot->id, e, 0);
        }
      }
    }

    if (req->filter) {
      alSourcei(music->source, AL_DIRECT_FILTER, req->filter->id);
    }
#endif
    if (req->range != 0.f) {
      alSourcef(music->source, AL_MAX_DISTANCE, req->range);
      alSourcef(music->source, AL_REFERENCE_DISTANCE, req->range / 2.f);
      alSourcef(music->source, AL_ROLLOFF_FACTOR, ASTERA_AL_ROLLOFF_FACTOR);
    } else {
      alSourcef(music->source, AL_MAX_DISTANCE, 0.f);
    }
  }

  alSourcef(music->source, AL_PITCH, 1.f);
  if (meta) {
    music->meta          = meta;
    music->total_samples = meta->total_samples;
    music->samples_left  = meta->total_samples;
  }

  for (int i = 0; i < AUDIO_BUFFERS_PER_MUSIC; ++i) {
    unsigned int buffer = music->buffers[i];

    int pcm_index, bytes_used, num_channels, num_samples, frame_size;
    pcm_index = bytes_used = num_channels = num_samples = frame_size = 0;

    int pcm_total_length = 0;

    float** out;

    memset(music->pcm, 0, sizeof(short) * music->pcm_length);

    for (int j = 0; j < music->packets_per_buffer; ++j) {
      frame_size = music->data_length - music->data_offset;
      if (frame_size > AUDIO_FRAME_SIZE)
        frame_size = AUDIO_FRAME_SIZE;

      bytes_used = stb_vorbis_decode_frame_pushdata(
          music->vorbis, music->data + music->data_offset, frame_size,
          &num_channels, &out, &num_samples);

      if (bytes_used == 0) {
        DBG_E("Unable to process samples from %i bytes.\n", frame_size);
        break;
      }

      music->data_offset += bytes_used;

      if (num_samples > 0) {
        int sample_count = num_channels * num_samples;
        int pcm_size     = sample_count * sizeof(short);
        pcm_total_length += pcm_size;
        for (int s = 0; s < num_samples - 1; ++s) {
          for (int c = 0; c < num_channels; ++c) {
            if (pcm_index >= music->pcm_length) {
              break;
            }

            music->pcm[pcm_index] = out[c][s] * 32676;
            ++pcm_index;
          }
        }
      }
    }

    alBufferData(buffer, music->format, music->pcm, pcm_total_length,
                 music->vorbis->sample_rate);
    alSourceQueueBuffers(music->source, 1, &buffer);
  }

  if (req) {
    music->req = req;
    alSourcef(music->source, AL_GAIN, req->gain);
    music->loop    = req->loop;
    music->has_req = 1;
  }

  return music;
}

void a_music_reset(a_music* music) {
  // TODO finish this implementation
  // NOTE: Playing -> load back buffer first, then push to front & fill back
  // buffer again
  // Stopped -> throw out both buffers and fill normally
  /*music->data_offset = 0;
  for (int i = 0; i < AUDIO_BUFFERS_PER_MUSIC; ++i) {
    unsigned int buffer = music->buffers[i];

    int pcm_index, bytes_used, num_channels, num_samples, frame_size;
    pcm_index = bytes_used = num_channels = num_samples = frame_size = 0;

    int pcm_total_length = 0;

    float **out;

    memset(music->pcm, 0, sizeof(short) * music->pcm_length);

    for (int j = 0; j < music->packets_per_buffer; ++j) {
      frame_size = music->data_length - music->data_offset;
      if (frame_size > AUDIO_FRAME_SIZE)
        frame_size = AUDIO_FRAME_SIZE;

      bytes_used = stb_vorbis_decode_frame_pushdata(
          music->vorbis, music->data + music->data_offset, frame_size,
          &num_channels, &out, &num_samples);

      if (bytes_used == 0) {
        DBG_E("Unable to process samples from %i bytes.\n", frame_size);
        break;
      }

      music->data_offset += bytes_used;

      if (num_samples > 0) {
        int sample_count = num_channels * num_samples;
        int pcm_size = sample_count * sizeof(short);
        pcm_total_length += pcm_size;
        for (int s = 0; s < num_samples - 1; ++s) {
          for (int c = 0; c < num_channels; ++c) {
            if (pcm_index >= music->pcm_length) {
              break;
            }

            music->pcm[pcm_index] = out[c][s] * 32676;
            ++pcm_index;
          }
        }
      }
    }

    alBufferData(buffer, music->format, music->pcm, pcm_total_length,
                 music->vorbis->sample_rate);
    alSourceQueueBuffers(music->source, 1, &buffer);
  }*/
}

void a_music_destroy(a_music* music) {
  stb_vorbis_close(music->vorbis);
}

time_s a_music_time(a_music* music) {
  uint32_t samples = music->total_samples - music->samples_left;
  return (time_s)(samples / music->sample_rate);
}

time_s a_music_len(a_music* music) {
  return (time_s)(music->total_samples / music->sample_rate);
}

void a_music_play(a_music* music) {
  alSourcePlay(music->source);
}

void a_music_stop(a_music* music) {
  alSourceStop(music->source);
  stb_vorbis_seek_start(music->vorbis);
  music->samples_left = music->total_samples;
}

void a_resume_music(a_music* music) {
  ALenum state;
  alGetSourcei(music->source, AL_SOURCE_STATE, &state);
  if (state == AL_PAUSED) {
    alSourcePlay(music->source);
  }
}

void a_music_pause(a_music* music) {
  alSourcePause(music->source);
}

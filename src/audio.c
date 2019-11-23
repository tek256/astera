#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER 0x1012
#define ALC_ALL_DEVICES_SPECIFIER 0x1013
#endif

#ifndef ALC_EXT_EFX
#define ALC_EFX_MAJOR_VERSION 0x20001
#define ALC_EFX_MINOR_VERSION 0x20002
#define ALC_MAX_AUXILIARY_SENDS 0x20003
#endif

#include "audio.h"
#include "asset.h"

#undef STB_VORBIS_HEADER_ONLY
#include <misc/stb_vorbis.c>

#include <string.h>

int a_allow_play(void) { return _ctx.allow; }

int a_init(u32 master, u32 sfx, u32 music) {
  if (!a_create_context(0)) {
    _e("Unable to create audio context.\n");
    return -1;
  }

  _map.layer_count = MAX_AUDIO_LAYERS;

  // Initialize cached audio subsystem
  for (int i = 0; i < MAX_SFX; ++i) {
    u32 id;
    alGenSources(1, &id);
    memset(&_map.sfx[i], 0, sizeof(a_sfx));
    _map.sfx[i].id = id;
    _map.sfx[i].range = DEFAULT_SFX_RANGE;
    _map.sfx[i].gain = 1.f;
    alSourcef(id, AL_GAIN, 1.f);

    // TODO implement range
    alSourcefv(id, AL_GAIN, _map.sfx[i].position);
    _map.sfx[i].has_req = 0;
  }

  _map.sfx_count = MAX_SFX;

  unsigned int sources[MAX_SONGS];
  unsigned int buffers[MAX_SONGS * AUDIO_BUFFERS_PER_MUSIC];

  alGenSources(MAX_SONGS, &sources);
  alGenBuffers(MAX_SONGS * AUDIO_BUFFERS_PER_MUSIC, &buffers);

  for (int i = 0; i < MAX_SONGS; ++i) {

    _map.songs[i].source = sources[i];

    for (int j = 0; j < AUDIO_BUFFERS_PER_MUSIC; ++j) {
      _map.songs[i].buffers[j] = buffers[(i * AUDIO_BUFFERS_PER_MUSIC) + j];
    }

    _map.songs[i].gain = 1.f;
    _map.songs[i].has_req = 0;
  }

  _map.song_count = MAX_SONGS;

  float master_f = master / 100.f;
  _listener.gain = master_f;
  alListenerf(AL_GAIN, _listener.gain);
  a_set_pos(_listener.pos);

  for (int i = 0; i < MAX_AUDIO_LAYERS; ++i) {
    _map.layers[i].gain = 1.f;
  }

  // TODO convert to processor bounds check
#ifdef AUDIO_MUSIC_LAYER
  if (_map.layer_count > AUDIO_MUSIC_LAYER) {
    _map.layers[AUDIO_MUSIC_LAYER].gain = (music / 100.f);
  } else {
    _e("Not enough layers generated to set volume for music layer on: %i\n",
       AUDIO_MUSIC_LAYER);
  }
#endif
#ifdef AUDIO_SFX_LAYER
  if (_map.layer_count > AUDIO_SFX_LAYER) {
    _map.layers[AUDIO_SFX_LAYER].gain = (sfx / 100.f);
  } else {
    _e("Not enough layers generated to set volume for sfx layer on: %i\n",
       AUDIO_SFX_LAYER);
  }
#endif
#ifdef AUDIO_MISC_LAYER
  if (_map.layer_count > AUDIO_MISC_LAYER) {
    _map.layers[AUDIO_MISC_LAYER].gain = AUDIO_MISC_GAIN;
  } else {
    _e("Not enough layers generated to set volume for misc layer on: %i\n",
       AUDIO_MISC_LAYER);
  }
#endif
#ifdef AUDIO_UI_LAYER
  if (_map.layer_count > AUDIO_UI_LAYER) {
    _map.layers[AUDIO_UI_LAYER].gain = AUDIO_UI_GAIN;
  } else {
    _e("Not enough layers generated to set volume for UI layer on: %i\n",
       AUDIO_UI_LAYER);
  }
#endif
  return 1;
}

void a_set_pos(vec3 p) { alListener3f(AL_POSITION, p[0], p[1], p[2]); }

void a_set_vol(u32 master, u32 sfx, u32 music) {
  a_set_vol_master(master);
  a_set_vol_sfx(sfx);
  a_set_vol_music(music);
}

void a_set_vol_master(u32 master) {
  _listener.gain = master / 100.f;
  alListenerf(AL_GAIN, _listener.gain);
}

void a_set_vol_sfx(u32 sfx) {
#ifdef AUDIO_SFX_LAYER
  if (_map.layer_count > AUDIO_SFX_LAYER) {
    float value = sfx / 100.f;
    if (_map.layers[AUDIO_SFX_LAYER].gain != value) {
      _map.layers[AUDIO_SFX_LAYER].gain = value;
      _map.layers[AUDIO_SFX_LAYER].gain_change = 1;
    }
  } else {
    _e("Unable to set SFX vol on layer out of bounds: %i\n", AUDIO_SFX_LAYER);
  }
#endif
}

void a_set_vol_music(u32 music) {
#ifdef AUDIO_MUSIC_LAYER
  if (_map.layer_count > AUDIO_MUSIC_LAYER) {
    float value = (music / 100.f);
    if (value != _map.layers[AUDIO_MUSIC_LAYER].gain) {
      _map.layers[AUDIO_MUSIC_LAYER].gain = value;
      _map.layers[AUDIO_MUSIC_LAYER].gain_change = 1;
    }
  } else {
    _e("Unable to set music vol on layer out of bounds: %i\n",
       AUDIO_MUSIC_LAYER);
  }
#endif
}

f32 a_get_vol_master(void) { return _listener.gain; }

f32 a_get_vol_sfx(void) {
#ifdef AUDIO_SFX_LAYER
  if (AUDIO_SFX_LAYER < _map.layer_count) {
    return _map.layers[AUDIO_SFX_LAYER].gain;
  } else {
    _e("Unable to get volume for SFX layer, [%i] out of bounds.\n",
       AUDIO_SFX_LAYER);
    return 0.f;
  }
#else
  return 0.f;
#endif
}

f32 a_get_vol_music(void) {
#ifdef AUDIO_MUSIC_LAYER
  if (AUDIO_MUSIC_LAYER < _map.layer_count) {
    return _map.layers[AUDIO_MUSIC_LAYER].gain;
  } else {
    _e("Unable to get volume for music layer, [%i] out of bounds.\n",
       AUDIO_MUSIC_LAYER);
    return 0.f;
  }
#else
  return 0.f;
#endif
}

void a_exit(void) {
  if (_ctx.context == NULL) {
    return;
  }

  for (int i = 0; i < _map.song_count; ++i) {
    alDeleteSources(1, &_map.songs[i].source);
    alDeleteBuffers(AUDIO_BUFFERS_PER_MUSIC, &_map.songs[i].buffers);
  }

  for (int i = 0; i < _map.sfx_count; ++i) {
    alDeleteSources(1, &_map.sfx[i].id);
  }

  alcMakeContextCurrent(NULL);
  alcDestroyContext(_ctx.context);
  alcCloseDevice(_ctx.device);
}

void a_update(long delta) {
  for (int i = 0; i < MAX_AUDIO_LAYERS; ++i) {
    a_layer *layer = &_map.layers[i];
    for (int j = 0; j < layer->sfx_count; ++j) {
      a_sfx *sfx = &layer->sources[j];
      if (sfx->has_req) {
        a_req *req = sfx->req;
        if (sfx->gain != req->gain || layer->gain_change) {
          float n_gain = layer->gain * req->gain;
          sfx->gain = req->gain;
          alSourcef(sfx->id, AL_GAIN, n_gain);
        }

        if (!vec3_cmp(sfx->position, req->pos)) {
          vec3_dup(sfx->position, req->pos);
          alSource3f(sfx->id, AL_POSITION, req->pos[0], req->pos[1],
                     req->pos[2]);
        }

        ALenum state;
        alGetSourcei(sfx->id, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
          alSourcei(sfx->id, AL_BUFFER, 0);
          sfx->req = NULL;
          sfx->has_req = 0;
        } else if (req) {
          if (req->stop) {
            alSourcei(_map.sfx[i].id, AL_BUFFER, 0);
            sfx->req = NULL;
            sfx->has_req = 0;
          }
        }
      }
    }

    for (int j = 0; j < layer->song_count; ++j) {
      a_music *mus = &layer->musics[j];
      if (mus->has_req) {
        a_req *req = mus->req;

        if (mus->gain != req->gain || layer->gain_change) {
          float n_gain = layer->gain * req->gain;
          mus->gain = req->gain;
          // NOTE: n_gain is a calculated product, not what we want to store
          alSourcef(mus->source, AL_GAIN, n_gain);
        }

        // TODO update position on change

        alSource3f(mus->source, AL_POSITION, req->pos[0], req->pos[1],
                   req->pos[2]);

        ALenum state;
        ALint proc;
        alGetSourcei(mus->source, AL_SOURCE_STATE, &state);
        alGetSourcei(mus->source, AL_BUFFERS_PROCESSED, &proc);

        if (req->stop) {
          alSourcei(mus->source, AL_BUFFER, 0);
          alSourceStop(mus->source);
          mus->req = NULL;
          mus->has_req = 0;
        } else if (proc > 0) {
          if (mus->samples_left <= 0 && req->loop) {
            mus->data_offset = mus->header_end;
            mus->samples_left = mus->total_samples;
          } else if (mus->samples_left <= 0 && !req->loop) { // Stop music
            alSourceStop(mus->source);
            mus->data_offset = 0;
            req->stop = 1;
          } else {
            int max_samples;
            for (int k = 0; k < proc; ++k) {
              if (mus->samples_left > AUDIO_FRAME_SIZE) {
                max_samples = AUDIO_FRAME_SIZE;
              } else {
                max_samples = mus->samples_left;
              }

              u32 buffer;
              s32 al_err;

              alSourceUnqueueBuffers(mus->source, 1, &buffer);

              if (state != AL_PLAYING && !req->stop && k == 0) {
                alSourcePlay(mus->source);
              }

              memset(mus->pcm, 0, mus->pcm_length * sizeof(short));
              s32 pcm_total_length = 0;
              s32 pcm_index = 0, frame_size = 0;
              s32 bytes_used = 0, num_samples = 0, num_channels = 0;
              float **out;

              for (int p = 0; p < mus->packets_per_buffer; ++p) {
                frame_size = mus->data_length - mus->data_offset;
                if (frame_size > AUDIO_FRAME_SIZE)
                  frame_size = AUDIO_FRAME_SIZE;
                bytes_used = stb_vorbis_decode_frame_pushdata(
                    mus->vorbis, mus->data + mus->data_offset, frame_size,
                    &num_channels, &out, &num_samples);
                if (!bytes_used) {
                  _l("Unable to load samples from [%i] bytes.\n", frame_size);
                  continue;
                }

                mus->data_offset += bytes_used;

                if (num_samples > 0) {
                  int short_count = num_samples * num_channels;
                  int pcm_length = sizeof(short) * short_count;
                  pcm_total_length += pcm_length;
                  if (pcm_length + pcm_index > mus->pcm_length) {
                    _e("Uhhh.\n");
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
                _e("Unable to unqueue audio buffer for music err [%i]: %i\n",
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

u32 a_get_device_name(char *dst, int capacity) {
  ALchar *name;
  if (_ctx.device) {
    name = alcGetString(_ctx.device, ALC_DEVICE_SPECIFIER);
  } else {
    name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  }

  int length = strlen(name);
  int limit = (length > capacity) ? capacity : length;
  memcpy(dst, name, sizeof(char) * limit);

  return limit;
}

int a_create_context(const char *device_name) {
  ALCdevice *device = NULL;

  if (device_name != NULL) {
    device = alcOpenDevice(device_name);
  } else {
    device = alcOpenDevice(NULL);
  }

  ALCcontext *context = alcCreateContext(device, NULL);

  if (!alcMakeContextCurrent(context)) {
    _e("Error creating OpenAL Context\n");
    return 0;
  }
  _ctx = (a_ctx){context, device};

#if defined(INIT_DEBUG)
  _l("Loaded audio device.\n");
#endif

  return 1;
}

a_buf a_get_buf(unsigned char *data, u32 length) {
  if (!data || !length) {
    _e("No data passed to create.\n");
    return (a_buf){0};
  }

  int id;
  alGenBuffers(1, &id);

  // TODO variant sample size
  int sample_size = 16;
  int channels, len, rate;

  void *n_buff = malloc(4096 * sizeof(char));
  int err;
  stb_vorbis_alloc _alloc = (stb_vorbis_alloc){n_buff, 4096 * sizeof(char)};

  // TODO fix vorbis implementaiton
  stb_vorbis *vorbis = stb_vorbis_open_memory(data, length, &err, NULL);

  if (!vorbis) {
    _e("Unable to open vorbis for music file: %i\n", err);
    free(vorbis);
    return (a_buf){0};
  }

  stb_vorbis_info info = stb_vorbis_get_info(vorbis);

  int format;
  if (info.channels == 1) {
    switch (sample_size) {
    case 8:
      format = AL_FORMAT_MONO8;
      break;
    case 16:
      format = AL_FORMAT_MONO16;
      break;
    default:
      _e("Unsupported format: channels: %i sample_size: %i\n", info.channels,
         sample_size);
      break;
    }
  } else if (info.channels == 2) {
    switch (sample_size) {
    case 8:
      format = AL_FORMAT_STEREO8;
      break;
    case 16:
      format = AL_FORMAT_STEREO16;
      break;
    default:
      _e("Unsupported format: channels: %i sample_size: %i\n", info.channels,
         sample_size);
      break;
    }
  }

  u32 total_samples;

  total_samples = (u32)stb_vorbis_stream_length_in_samples(vorbis);
  int sample_count =
      (MAX_MUSIC_RUNTIME > total_samples) ? total_samples : MAX_MUSIC_RUNTIME;

  // int num_samples = stb_vorbis_get_samples_short_interleaved(vorbis,
  // info.channels, (short*)pcm, sample_count*info.channels);
  // TODO framebuffer decoding in short sounds

  // void* pcm = calloc(MAX_MUSIC_RUNTIME * music->sample_size / 8 *
  // music->channel, 1);

  // lBufferData(id, format, pcm, sample_count*(sample_size/8*info.channels),
  // info.sample_rate);

  stb_vorbis_close(vorbis);
  // free(pcm);

  return (a_buf){id, channels, len, rate};
}

void a_destroy_buffer(a_buf buffer) {
  int rm_index = -1;
  for (int i = 0; i < _map.buf_count; ++i) {
    if (_map.bufs[i].id == buffer.id) {
      rm_index = i;
      break;
    }
  }

  if (rm_index != -1) {
    int end = (_map.buf_count == _map.buf_capacity) ? _map.buf_capacity - 1
                                                    : _map.buf_count;
    for (int i = rm_index; i < end; ++i) {
      _map.bufs[i] = _map.bufs[i + 1];
    }

    for (int i = rm_index; i < end; ++i) {
      _map.buf_names[i] = _map.buf_names[i + 1];
    }

    if (end == _map.buf_capacity - 1) {
      _map.bufs[end + 1].id = 0;
      _map.buf_names[end + 1] = NULL;
    }
  }

  alDeleteBuffers(1, &buffer.id);
}

a_buf *a_get_bufn(const char *name) {
  if (!name) {
    _e("Unable to get null name buffer.\n");
    return NULL;
  }

  for (int i = 0; i < _map.buf_count; ++i) {
    if (strcmp(_map.buf_names[i], name) == 0) {
      return &_map.bufs[i];
    }
  }

  _e("No buffer found named: %s\n", name);
  return NULL;
}

a_sfx *a_play_sfxn(const char *name, a_req *req) {
  if (!name) {
    _e("No buffer name passed to play.\n");
    return NULL;
  }

  a_buf *buff = a_get_bufn(name);

  int index = -1;
  for (int i = 0; i < MAX_SFX; ++i) {
    if (!_map.sfx[i].has_req) {
      index = i;
      break;
    }
  }

  if (!index) {
    return NULL;
  }

  a_sfx *src = &_map.sfx[index];

  alSourcei(src->id, AL_BUFFER, buff->id);

  if (req) {
    alSourcefv(src->id, AL_POSITION, req->pos);
    alSourcefv(src->id, AL_VELOCITY, req->vel);

    // Adjust to the layer's gain
    f32 layer_gain = _map.layers[req->layer].gain;
    alSourcef(src->id, AL_GAIN, req->gain * layer_gain);
    // TODO implement range
    alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);
  } else {
    alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
    alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});

    // Adjust to the layer's gain
    alSourcef(src->id, AL_GAIN, 1.f);
    // TODO implement range
    alSourcei(src->id, AL_LOOPING, AL_FALSE);
  }

  alSourcePlay(src->id);
  return src;
}

a_sfx *a_play_sfx(a_buf *buff, a_req *req) {
  if (!buff) {
    _e("No buffer passed to play.\n");
    return NULL;
  }

  int index = -1;
  for (int i = 0; i < MAX_SFX; ++i) {
    if (!_map.sfx[i].has_req) {
      index = i;
      break;
    }
  }

  if (!index) {
    return NULL;
  }

  a_sfx *src = &_map.sfx[index];

  alSourcei(src->id, AL_BUFFER, buff->id);

  if (req) {
    alSourcefv(src->id, AL_POSITION, req->pos);
    alSourcefv(src->id, AL_VELOCITY, req->vel);

    // Adjust to the layer's gain
    f32 layer_gain = _map.layers[req->layer].gain;
    alSourcef(src->id, AL_GAIN, req->gain * layer_gain);
    // TODO implement range
    alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);
  } else {
    alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
    alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});

    // Adjust to the layer's gain
    alSourcef(src->id, AL_GAIN, 1.f);
    // TODO implement range
    alSourcei(src->id, AL_LOOPING, AL_FALSE);
  }

  alSourcePlay(src->id);
  return src;
}

a_keyframes a_get_keyframes(const char *name) { a_keyframes keyframes; }

a_music *a_create_music(unsigned char *data, u32 length, s32 sample_count,
                        s32 *keyframes, s32 *keyframe_offsets,
                        s32 keyframe_count, a_req *req) {
  if (!data || !length) {
    _e("No data passed to create music.\n");
    return NULL;
  }

  int index = -1;
  for (int i = 0; i < MAX_SONGS; ++i) {
    if (_map.songs[i].has_req == 0) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    _e("No available music slots.\n");
    return NULL;
  }

  a_music *music = &_map.songs[index];

  if (!music) {
    _e("Unable to get music buffer to initialize song.\n");
    return NULL;
  }

  if (music->source == 0) {
    _e("Unable to get source for music initialization.\n");
  }

  s32 error, used;

  music->data_length = length;
  music->data = data;
  music->vorbis =
      stb_vorbis_open_pushdata(music->data, length, &used, &error, NULL);

  if (!music->vorbis) {
    music->data = 0;
    music->req = 0;
    music->data_length = 0;
    _l("Unable to load header from %i bytes.\n", length);
    return NULL;
  }

  music->header_end = used;

  music->data_offset += used;

  stb_vorbis_info info = stb_vorbis_get_info(music->vorbis);

  music->sample_rate = info.sample_rate;

  if (info.channels < 0 || info.channels > 3) {
    _e("Invalid channel size for audio system: %i\n", info.channels);
    return 0;
  }

  if (info.channels == 1) {
    music->format = AL_FORMAT_MONO16;
  } else if (info.channels == 2) {
    music->format = AL_FORMAT_STEREO16;
  }

  music->pcm_length =
      AUDIO_FRAME_SIZE * AUDIO_DEFAULT_FRAMES_PER_BUFFER * info.channels;
  music->pcm = malloc(music->pcm_length * sizeof(short));
  memset(music->pcm, 0, sizeof(short) * music->pcm_length);
  // music->pcm = calloc(music->pcm_len, 1);

  if (!music->pcm) {
    _e("Unable to allocate space for PCM in music: %i\n", music->pcm_length);
    music->pcm = 0;
    music->data = 0;
    music->data_length = 0;
    music->pcm_length = 0;
    return 0;
  }

  alSourcef(music->source, AL_PITCH, 1.f);
  music->total_samples = sample_count;

  music->keyframes = keyframes;
  music->keyframe_offsets = keyframe_offsets;
  music->keyframe_count = keyframe_count;

  for (int i = 0; i < AUDIO_BUFFERS_PER_MUSIC; ++i) {
    unsigned int buffer = music->buffers[i];
    int pcm_index = 0, pcm_total_length = 0;
    int bytes_used, num_channels, num_samples;
    int frame_size = 0;
    float **out;

    memset(music->pcm, 0, music->pcm_length);

    for (int j = 0; j < music->packets_per_buffer; ++j) {
      frame_size = music->data_length - music->data_offset;
      if (frame_size > AUDIO_FRAME_SIZE)
        frame_size = AUDIO_FRAME_SIZE;
      bytes_used = stb_vorbis_decode_frame_pushdata(
          music->vorbis, music->data + music->data_offset, frame_size,
          &num_channels, &out, &num_samples);

      if (bytes_used == 0) {
        _e("Unable to process samples out of [%i] bytes.\n", frame_size);
        break;
      }

      music->data_offset += bytes_used;

      if (num_samples) {
        int sample_count = num_channels * num_samples;
        int pcm_size = sample_count * sizeof(short);
        pcm_total_length += pcm_size;

        for (int s = 0; s < num_samples; ++s) {
          for (int c = 0; c < num_channels; ++c) {
            music->pcm[pcm_index] = out[c][s] * 32676;
            ++pcm_index;
          }
        }
      }
    }

    alBufferData(buffer, music->format, music->pcm, pcm_total_length,
                 music->vorbis->sample_rate);
    alSourceQueueBuffers(music->source, 1, buffer);
  }

  // music->total_samples =
  // (u32)stb_vorbis_stream_length_in_samples(music->vorbis);
  // music->samples_left = music->total_samples;

  if (req) {
    alSourcef(music->source, AL_GAIN, req->gain);
    music->loop = req->loop;

    // NOTE: AL_LOOPING will repeat a buffer, not a multi buffered object like a
    // song alSourcei(music->source, AL_LOOPING, AL_TRUE);
  }

  return music;
}

void a_destroy_music(a_music *music) { stb_vorbis_close(music->vorbis); }

f32 a_get_music_time(a_music *music) {
  u32 samples = music->total_samples - music->samples_left;
  return (f32)samples / music->sample_rate;
}

f32 a_get_music_len_time(a_music *music) {
  return (f32)(music->total_samples / music->sample_rate);
}

void a_play_music(a_music *music) { alSourcePlay(music->source); }

void a_stop_music(a_music *music) {
  alSourceStop(music->source);
  stb_vorbis_seek_start(music->vorbis);
  music->samples_left = music->total_samples;
}

void a_resume_music(a_music *music) {
  ALenum state;
  alSourcei(music->source, AL_SOURCE_STATE, &state);
  if (state == AL_PAUSED) {
    alSourcePlay(music->source);
  }
}

void a_pause_music(a_music *music) { alSourcePause(music->source); }

static int a_get_open_sfx(void) {
  for (int i = 0; i < MAX_SFX; ++i) {
    if (!_map.sfx[i].has_req) {
      return i;
    }
  }

  return -1;
}

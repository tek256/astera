#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

#ifndef ALC_EXT_EFX
#define ALC_EFX_MAJOR_VERSION                    0x20001
#define ALC_EFX_MINOR_VERSION                    0x20002
#define ALC_MAX_AUXILIARY_SENDS                  0x20003
#endif

#include "audio.h"

#undef STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

#include <string.h>

int a_init(u32 master, u32 sfx, u32 music){
	if(!a_load_devices()){
		_e("Unable to load audio devices\n");
		return -1;
	}

	if(!a_create_context(NULL)){
		_e("Unable to create audio context.\n");
		return -1;
	}
	
	//Initialize cached audio subsystem
	for(int i=0;i<MAX_SFX;++i){
		u32 id;
		alGenSources(1, &id);
		memset(&_map.sfx[i], 0, sizeof(a_sfx));
		_map.sfx[i].id = id;
		_map.sfx[i].range = DEFAULT_SFX_RANGE;	
		_map.sfx[i].gain = 1.f;
		alSourcef(id, AL_GAIN, 1.f);

		//TODO implement range	
		alSourcefv(id, AL_GAIN, _map.sfx[i].position);
		_map.sfx[i].has_req = 0;
	}

	for(int i=0;i<MAX_SONGS;++i){
		u32 src_id;
		u32 buff_ids[2];		
		
		alGenSources(1, &src_id);
		alGenBuffers(2, &buff_ids);
		
		_map.songs[i].gain = 1.f;
		_map.songs[i].source = src_id;
		_map.songs[i].buffers[0] = buff_ids[0];
		_map.songs[i].buffers[1] = buff_ids[1];
		_map.songs[i].has_req = 0;
	}

	for(int i=0;i<MAX_AUDIO_LAYERS;++i){
		_map.layers[i].gain = 1.f;

#ifdef AUDIO_MUSIC_LAYER
		if(i == AUDIO_MUSIC_LAYER){
			_map.layers[i].gain = AUDIO_MUSIC_GAIN;  
		}
#endif
#ifdef AUDIO_SFX_LAYER
		if(i == AUDIO_SFX_LAYER){
			_map.layers[i].gain = AUDIO_SFX_GAIN; 
		}
#endif
#ifdef AUDIO_MISC_LAYER
		if(i == AUDIO_MISC_LAYER){
			_map.layers[i].gain = AUDIO_MISC_GAIN;
		}
#endif
#ifdef AUDIO_UI_LAYER
		if(i == AUDIO_UI_LAYER){
			_map.layers[i].gain = AUDIO_UI_GAIN;
		}
#endif
	}		

	return 1;
}

void a_exit(){
	if(_ctx.context == NULL){
		return;
	}

	alcDestroyContext(_ctx.context);
	alcCloseDevice(_ctx.device);
}

void a_update(long delta){
	for(int i=0;i<_map.sfx_capacity;++i){
		if(_map.sfx[i].has_req){
			a_req* req = _map.sfx[i].req;

			ALenum state;
			alGetSourcei(_map.sfx[i].id, AL_SOURCE_STATE, &state);
			if(state == AL_STOPPED){
				alSourcei(_map.sfx[i].id, AL_BUFFER, 0);
				_map.sfx[i].req = NULL;
				_map.sfx[i].has_req = 0;
			}else if(req){
				if(req->stop){
					alSourcei(_map.sfx[i].id, AL_BUFFER, 0);
					_map.sfx[i].req = NULL;
					_map.sfx[i].has_req = 0;					
				}
			}	
		}	
	}

	for(int i=0;i<_map.song_capacity;++i){
		if(_map.songs[i].has_req){
			a_req* req = _map.songs[i].req;
			ALenum state;
			ALint proc;
			alGetSourcei(_map.songs[i].source, AL_SOURCE_STATE, &state);
			alGetSourcei(_map.songs[i].source, AL_BUFFERS_PROCESSED, &proc);

			if(state == AL_STOPPED){
				alSourcei(_map.songs[i].source, AL_BUFFER, 0);
				_map.songs[i].req = NULL;	
				_map.songs[i].has_req = 0;
			} else if(req){
				alSourcei(_map.songs[i].source, AL_BUFFER, 0);
				_map.songs[i].req = NULL;
				_map.songs[i].has_req = NULL;
			} else if(proc > 0){
				a_music* mus = &_map.songs[i];
				int ending = 0;
				int sample_count = 0;
				int num_left = proc;

				
				//Stop music
				if(mus->samples_left <= 0){
					alSourceStop(mus->source);

					int offset = rand();
					if(offset < 0) offset = -offset;
					offset %= (mus->len / MAX_MUSIC_RUNTIME);

					mus->pos = offset;

					mus->samples_left = mus->total_samples;
				}else{
					for(int i=0;i<num_left;++i){
						if(_map.songs[i].samples_left >= MAX_MUSIC_RUNTIME){
							sample_count = MAX_MUSIC_RUNTIME;
						}else {
							sample_count = _map.songs[i].samples_left;
						}

						int k = 0;
						int n;
						float* l, *r;
						float** out;
						int num_c;
						int q = MAX_MUSIC_RUNTIME;
						if(q > mus->len - mus->pos)
						   q = mus->len - mus->pos;

						int num_samples = stb_vorbis_decode_frame_pushdata(mus->vorbis, mus->data+mus->pos, q, &num_c, &out, &n);	
						a_interleave_output(mus->channels, mus->pcm, num_c, out, mus->pos, num_samples);
						if(num_samples == 0){
							if(mus->loop){
								if(mus->pos + q == mus->len){
									mus->pos = rand();
									if(mus->pos < 0){
										mus->pos = -mus->pos;
									}
									mus->pos %= (mus->len - MAX_MUSIC_RUNTIME);
									stb_vorbis_flush_pushdata(mus->vorbis);
									q = 128;
									num_samples = stb_vorbis_decode_frame_pushdata(mus->vorbis, mus->data+mus->pos, q, &num_c, &out, &n);

									if(!num_samples){
										_e("Unable to decode on loop from frame pushdata.\n");
									}
								}else{
									if(q < 128){
										q = 128;
									}
									q *= 2;

									int num_samples = stb_vorbis_decode_frame_pushdata(mus->vorbis, mus->data + mus->pos, q, &num_c, &out, &n);
									if(!num_samples){
										_e("Unable to decode on loop from frame pushdata.\n");
									}
								}
							}else{
								mus->req->stop = 1;
							}
							break;	
						}

						mus->pos += num_samples;

						//Update buffers for the music by rotating it out
						ALuint buff = 0;
						alSourceUnqueueBuffers(mus->source, 1, &buff);
						if(alGetError() != AL_INVALID_VALUE){
							//alBufferData(buff, mus->format, out, num_samples * (mus->sample_size / 8 * mus->channels), mus->sample_rate);
								
							alSourceQueueBuffers(mus->source, 1, &buff);
						}else{
							_e("Unable to unqueue audio buffer for music.\n");
						}

						mus->samples_left -= num_samples;
					}
				}	
			}	
		}	
	}

}

/*
 *ALenum state;
		int num_left = processed;
		int sample_count = 0;

		for(int i=0;i<num_left;++i){
			if(music->samples_left >= MAX_MUSIC_RUNTIME){
				sample_count = MAX_MUSIC_RUNTIME;
			}else{
				sample_count = music->samples_left;
			}

		}

		free(pcm);

		if(ending){
			a_stop_music(music);
			if(music->loop){
				a_play_music(music);
			}
		}
	}

 */

int a_load_devices(){
	char* device_list;

	if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE){
		device_list = (char*)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	}else{
		device_list = (char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	}

	if(!device_list){
		_e("No device list\n");
	}

	char* start = &device_list[0];

	char** list;
	int count = 0;
	int totalSize = 0;
	while(*device_list != '\0'){
		++count;
		size_t len = strlen(device_list) + 1;
		totalSize += sizeof(char) * len;
		device_list += len;
	}

	device_list = start;
	list = malloc(sizeof(char) * totalSize);

	int index = 0;
	while(*device_list != '\0'){
		size_t len = strlen(device_list) + 1;
		list[index] = device_list;
		device_list += len;
		++index;
	}

	a_device_list = list;
	a_device_list_count = count;

	return 1;
}

int a_create_context(const char* device_name){
	ALCdevice* device = NULL;

	if(device_name != NULL){
		device = alcOpenDevice(device_name);
	}else{
		device = alcOpenDevice(NULL);
	}

	ALCcontext* context = alcCreateContext(device, NULL);

	if(!alcMakeContextCurrent(context)){
		_e("Error creating OpenAL Context\n");
		return 0;
	}
	_ctx = (a_ctx){context, device};

	_l("Loaded audio device.\n");

	return 1;
}

const char** a_get_devices(int* count){
	const char** device_names = malloc(sizeof(const char*) * a_device_list_count);
	for(int i=0;i<a_device_list_count; ++i){
		device_names[i] = a_device_list[i];
	}
	*count = a_device_list_count;
	return device_names;
}

void a_swap_device(const char* device_name){
	_ctx.allow = 0;

	alcDestroyContext(_ctx.context);
	alcCloseDevice(_ctx.device);

	ALCdevice* device = NULL;
	if(device_name != NULL){
		for(int i=0;i<a_device_list_count;++i){
			if(strcmp(a_device_list[i], device_name) == 0){
				device_name = a_device_list[i];
				break;
			}
		}
	}
	alcOpenDevice(device_name);

	ALCcontext* context = NULL;
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);

	_ctx.context = context;
	_ctx.device = device;

	_ctx.allow = 1;
}

static void a_compute_stereo(short* output, int num_c, float** data, int d_offset, int len){
	#define BUFFER_SIZE  32
	float buffer[BUFFER_SIZE];
	int i,j,o,n = BUFFER_SIZE >> 1;
	// o is the offset in the source data
	check_endianness();
	for (o = 0; o < len; o += BUFFER_SIZE >> 1) {
		// o2 is the offset in the output data
		int o2 = o << 1;
		memset(buffer, 0, sizeof(buffer));
		if (o + n > len) n = len - o;
		for (j=0; j < num_c; ++j) {
			int m = channel_position[num_c][j] & (PLAYBACK_LEFT | PLAYBACK_RIGHT);
			if (m == (PLAYBACK_LEFT | PLAYBACK_RIGHT)) {
				for (i=0; i < n; ++i) {
					buffer[i*2+0] += data[j][d_offset+o+i];
					buffer[i*2+1] += data[j][d_offset+o+i];
				}
			} else if (m == PLAYBACK_LEFT) {
				for (i=0; i < n; ++i) {
					buffer[i*2+0] += data[j][d_offset+o+i];
				}
			} else if (m == PLAYBACK_RIGHT) {
				for (i=0; i < n; ++i) {
					buffer[i*2+1] += data[j][d_offset+o+i];
				}
			}
		}
		for (i=0; i < (n<<1); ++i) {
			FASTDEF(temp);
			int v = FAST_SCALED_FLOAT_TO_INT(temp,buffer[i],15);
			if ((unsigned int) (v + 32768) > 65535)
				v = v < 0 ? -32768 : 32767;
			output[o2+i] = v;
		}
	}
}

static void a_interleave_output(int buffer_c, short* buffer, int data_c, float** data, int data_offset, int length){
	int i;
	if(buffer_c != data_c && buffer_c <= 2 && data_c <= 6){
		if(!buffer_c == 2){
			_e("Invalid buffer channel count.\n");
			return;
		}
		for(int i=0;i<buffer_c;++i){
			a_compute_stereo(buffer, data_c, data, data_offset, length);
		}
	}else{
		int limit = buffer_c < data_c ? buffer_c : data_c;
		int j;
		for(j=0;j<length;++j){
			for(i=0;i<limit;++i){
				FASTDEF(temp);
				float f = data[i][data_offset+j];
				float v = FAST_SCALED_FLOAT_TO_INT(temp, f, 15);
				if ((unsigned int)(v + 32768) > 65535){ //short max
					v = v < 0 ? -32768 : 32767;
				}
				*buffer++ = v;
			}

			for( ; i < buffer_c; ++i){
				*buffer++ = 0;
			}
		}	
	}
}

a_buf a_get_buf(unsigned char* data, u32 length){
	if(!data || !length) {
		_e("No data passed to create.\n");
		return (a_buf){0};
	}

	int id;
	alGenBuffers(1, &id);

	//TODO variant sample size
	int sample_size = 16;
	int channels, len, rate;

	void* n_buff = malloc(4096 * sizeof(char));
	int err;
	stb_vorbis_alloc _alloc = (stb_vorbis_alloc){n_buff, 4096 * sizeof(char)};

	//TODO fix vorbis implementaiton
	stb_vorbis* vorbis = stb_vorbis_open_memory(data, length, &err, NULL);

	if(!vorbis){
		_e("Unable to open vorbis for music file: %i\n", err);
		free(vorbis);
		return (a_buf){0};
	}

	stb_vorbis_info info = stb_vorbis_get_info(vorbis);

	int format;
	if(info.channels == 1){
		switch(sample_size){
			case 8:
				format = AL_FORMAT_MONO8;
				break;
			case 16:
				format = AL_FORMAT_MONO16;
				break;
			default:
				_e("Unsupported format: channels: %i sample_size: %i\n", info.channels, sample_size);
				break;
		}
	}else if(info.channels == 2){
		switch(sample_size){
			case 8:
				format = AL_FORMAT_STEREO8;
				break;
			case 16:
				format = AL_FORMAT_STEREO16;
				break;
			default:
				_e("Unsupported format: channels: %i sample_size: %i\n", info.channels, sample_size);
				break;
		}
	}

	u32 total_samples;

	total_samples = (u32)stb_vorbis_stream_length_in_samples(vorbis);
	int sample_count = (MAX_MUSIC_RUNTIME > total_samples) ? total_samples : MAX_MUSIC_RUNTIME;

	//int num_samples = stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, (short*)pcm, sample_count*info.channels);
	//TODO framebuffer decoding in short sounds
	
	//void* pcm = calloc(MAX_MUSIC_RUNTIME * music->sample_size / 8 * music->channel, 1);
	
	//lBufferData(id, format, pcm, sample_count*(sample_size/8*info.channels), info.sample_rate);

	stb_vorbis_close(vorbis);
	//free(pcm);

	return (a_buf){id, channels, len, rate};
}

void a_destroy_buffer(a_buf buffer){
	int rm_index = -1;
	for(int i=0;i<_map.buf_count;++i){
		if(_map.bufs[i].id == buffer.id){
			rm_index = i;
			break;
		}	
	}

	if(rm_index != -1){
		int end = (_map.buf_count == _map.buf_capacity) ? _map.buf_capacity-1 : _map.buf_count;
		for(int i=rm_index;i<end;++i){
			_map.bufs[i] = _map.bufs[i+1];
		}

		for(int i=rm_index;i<end;++i){
			_map.buf_names[i] = _map.buf_names[i+1];
		}

		if(end == _map.buf_capacity-1){
			_map.bufs[end+1].id = 0;
			_map.buf_names[end+1] = NULL;
		}
	}

	alDeleteBuffers(1, &buffer.id);
}

a_buf* a_get_bufn(const char* name){
	if(!name){
		_e("Unable to get null name buffer.\n");
		return NULL;
	}

	for(int i=0;i<_map.buf_count;++i){
		if(strcmp(_map.buf_names[i], name) == 0){
			return &_map.bufs[i];
		}
	}

	_e("No buffer found named: %s\n", name); 
	return NULL;
}

a_sfx* a_play_sfxn(const char* name, a_req* req){
	if(!name){
		_e("No buffer name passed to play.\n");
		return NULL;
	}

	a_buf* buff = a_get_bufn(name);

	int index = -1;
	for(int i=0;i<MAX_SFX;++i){
		if(!_map.sfx[i].has_req){
			index = i;
			break;
		}
	}

	if(!index){
		return NULL;
	}	

	a_sfx* src = &_map.sfx[index];

	alSourcei(src->id, AL_BUFFER, buff->id);

	if(req){
		alSourcefv(src->id, AL_POSITION, req->pos);
		alSourcefv(src->id, AL_VELOCITY, req->vel);
	
		//Adjust to the layer's gain
		f32 layer_gain = _map.layers[req->layer].gain;
		alSourcef(src->id, AL_GAIN, req->gain * layer_gain);
		//TODO implement range
		alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);
	}else{
		alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
		alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});
	
		//Adjust to the layer's gain
		alSourcef(src->id, AL_GAIN, 1.f);
		//TODO implement range
		alSourcei(src->id, AL_LOOPING, AL_FALSE);	
	}

	alSourcePlay(src->id);
	return src;
}

a_sfx* a_play_sfx(a_buf* buff, a_req* req){
	if(!buff){
		_e("No buffer passed to play.\n");
		return NULL;
	}

	int index = -1;
	for(int i=0;i<MAX_SFX;++i){
		if(!_map.sfx[i].has_req){
			index = i;
			break;
		}
	}

	if(!index){
		return NULL;
	}	

	a_sfx* src = &_map.sfx[index];

	alSourcei(src->id, AL_BUFFER, buff->id);

	if(req){
		alSourcefv(src->id, AL_POSITION, req->pos);
		alSourcefv(src->id, AL_VELOCITY, req->vel);
	
		//Adjust to the layer's gain
		f32 layer_gain = _map.layers[req->layer].gain;
		alSourcef(src->id, AL_GAIN, req->gain * layer_gain);
		//TODO implement range
		alSourcei(src->id, AL_LOOPING, (req->loop) ? AL_TRUE : AL_FALSE);
	}else{
		alSourcefv(src->id, AL_POSITION, (vec3){0.f, 0.f, 0.f});
		alSourcefv(src->id, AL_VELOCITY, (vec3){0.f, 0.f, 0.f});
	
		//Adjust to the layer's gain
		alSourcef(src->id, AL_GAIN, 1.f);
		//TODO implement range
		alSourcei(src->id, AL_LOOPING, AL_FALSE);	
	}

	alSourcePlay(src->id);
	return src;
}



a_music* a_create_music(unsigned char* data, u32 length, a_req* req){
	if(!data || !length){
		_e("No data passed to create music.\n");
		return NULL;
	}

	int index = -1;
   	for(int i=0;i<MAX_SONGS;++i){
		if(_map.songs[i].has_req == 0){
			index = i;
			break;
		}
	}	

	if(index == -1){
		_e("No available music slots.\n");
		return NULL;
	}

	a_music* music = &_map.songs[index];

	if(!music){
		_e("Unable to get music buffer to initialize song.\n");
		return NULL;
	}

	if(music->source == 0){
		_e("Unable to get source for music initialization.\n");
	}

	s32 p, q, error;

	p = 0;
	q = 1;

	music->len = length;
	music->data = data;
	music->vorbis = stb_vorbis_open_pushdata(music->data, q, &music->used, &error, NULL);
	
	while(!music->vorbis){
		if(error == VORBIS_need_more_data){
			q += 1;
			music->vorbis = stb_vorbis_open_pushdata(music->data, q, &music->used, &error, NULL);
		}else{
			_e("Unable to load vorbis data. errno: %i\n", error);
			break;	
		}
	}

	music->used += p;

	if(music->vorbis == NULL){
		_e("Audio data could not be loaded.\n");
		data = 0;
		return music;
	}else{
		stb_vorbis_info info = stb_vorbis_get_info(music->vorbis);

		music->sample_rate = info.sample_rate;
		music->sample_size = 16;

		if(info.channels > 0 && info.channels < 3){
			music->channels = info.channels;
		}else {
			_e("Invalid channel size for audio system: %i\n", info.channels);
			return 0;
		}

		if(info.channels == 1){
			music->format = AL_FORMAT_MONO16;
		}else if(info.channels == 2){
			music->format = AL_FORMAT_STEREO16;
		}

		music->pcm_len = MAX_MUSIC_RUNTIME * music->sample_size / 8 * music->channels;
		//music->pcm = calloc(music->pcm_len, 1);

		if(!music->pcm){
			_e("Unable to allocate space for PCM in music: %i\n", music->pcm_len);
			music->pcm = 0;
			return 0;
		}


		alSourcef(music->source, AL_PITCH, 1.f);

		if(music->channels == 2){
			u32 id[2];
			alGenBuffers(2, &id);

			music->buffers[2] = id[0];
			music->buffers[3] = id[1];	
		}

		music->total_samples = (u32)stb_vorbis_stream_length_in_samples(music->vorbis);
		music->samples_left = music->total_samples;

		if(req){
			alSourcef(music->source, AL_GAIN, req->gain);
			music->loop = req->loop;

			if(req->loop){
				alSourcei(music->source, AL_LOOPING, AL_TRUE);
			}
		}
	}

	return music;
}

void a_destroy_music(a_music* music){
	stb_vorbis_close(music->vorbis);
}

f32 a_get_music_time(a_music* music){
	u32 samples = music->total_samples - music->samples_left;
	return (f32)samples / music->sample_rate;
}

f32 a_get_music_len_time(a_music* music){
	return (f32)(music->total_samples/music->sample_rate);
}

void a_play_music(a_music* music){
	alSourcePlay(music->source);
}

void a_stop_music(a_music* music){
	alSourceStop(music->source);
	stb_vorbis_seek_start(music->vorbis);
	music->samples_left = music->total_samples;
}

void a_resume_music(a_music* music){
	ALenum state;
   	alSourcei(music->source, AL_SOURCE_STATE, &state);
	if(state == AL_PAUSED){
		alSourcePlay(music->source);
	}
}

void a_pause_music(a_music* music){
	alSourcePause(music->source);
}

static int a_get_open_sfx(){
	for(int i=0;i<MAX_SFX;++i){
		if(!_map.sfx[i].has_req){
			return i;
		}
	}

	return -1;
}


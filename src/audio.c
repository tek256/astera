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

/*typedef struct {
    unsigned int id;
    a_src source;
} a_sfx;*/

//TODO fix initialization
int a_init(){
	if(!a_load_devices()){
		_e("Unable to load audio devices\n");
		return -1;
	}

	if(!a_create_context(NULL)){
		_e("Unable to create audio context.\n");
		return -1;
	}

	static int sfx_srcs[MAX_QUICK_SFX];
	alGenSources(MAX_QUICK_SFX, sfx_srcs);

	for(int i=0;i<MAX_QUICK_SFX;++i){
		a_qsfx[i].source.id = sfx_srcs[i];	
		alSourcei(sfx_srcs[i], AL_BUFFER, 0);	
	}

	return 1;
}

void a_exit(){
	//TODO
}

void a_update(long delta){
	a_update_sfx(delta);
}

#ifdef CACHE_AUDIO_FILES
a_file* a_gen_file(const char* fp, a_buf* buffer){
    audio_files[audio_file_count] = (a_file){fp, buffer};
    audio_file_count ++;
    return &audio_files[audio_file_count];
}

int a_get_file_index(const char* fp){
    for(int i=0;i<audio_file_count;i++){
        if(strcmp(audio_files[i].path, fp) == 0){
            return i;
        }
    }
    return -1;
}

a_file* a_get_file(const char* fp){
    int index = a_get_file_index(fp);
    if(index != -1){
        return &audio_files[index];
    }
    return 0;
}

void a_destroy_file(const char* fp){
    int index = a_get_file_index(fp);

    if(index != -1){
        free(audio_files[index].buffer);
        for(int i=index;i<audio_file_count-1;i++){
            audio_files[i] = audio_files[i+1];
        }
        audio_file_count --;
    }
}

a_buf* a_get_buffer(const char* fp){
    a_file* file = a_get_file(fp);
    if(file == NULL){
        a_buf buffer = a_create_buffer(fp);
        file = a_gen_file(fp, &buffer);
    }
    return file->buffer;
}

a_file** a_load_sfx(const char** fp, int count){
    a_file** files = (a_file**)malloc(sizeof(a_file) * count);

    for(int i=0;i<count;++i){
        if(fp[i] != NULL){
            a_buf buffer = a_create_buffer(fp[i]);
            if(buffer.id != 0 && audio_file_count < MAX_AUDIO_FILES){
                audio_files[audio_file_count].buffer = &buffer;
                audio_files[audio_file_count].path = fp[i];
                files[i] = &audio_files[audio_file_count];
                audio_file_count ++;
            }
        }
    }

    return files;
}

void a_destroy_file_cache(){
    for(int i=0;i<audio_file_count;++i){
        a_destroy_buffer(*audio_files[i].buffer);
    }
}
#endif

int a_load_devices(){
    char* device_list;

    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE){
        device_list = (char*)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    }else{
        device_list = (char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    }

    if(!device_list){
        printf("No device list\n");
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

int a_create_context(const char* deviceName){
    ALCdevice* device = NULL;

    if(deviceName != NULL){
        device = alcOpenDevice(deviceName);
    }else{
        device = alcOpenDevice(NULL);
    }

    ALCcontext* context = alcCreateContext(device, NULL);

    if(!alcMakeContextCurrent(context)){
        printf("Error creating OpenAL Context\n");
        return 0;
    }
    _a_ctx = (a_ctx){context, device};

    for(int i=0;i<MAX_QUICK_SFX;++i){
        a_qsfx[i].id = i;
        a_qsfx[i].source = a_create_source((v3){0.f}, 10.f, 0);
    }

    return 1;
}

void a_destroy_context(){
    if(_a_ctx.context == NULL){
        return;
    }

    for(int i=0;i<MAX_QUICK_SFX;++i){
        alDeleteSources(1, &a_qsfx[i].source.id);
    }

    alcDestroyContext(_a_ctx.context);
    alcCloseDevice(_a_ctx.device);
}

const char** a_get_devices(int* count){
    const char** deviceNames = malloc(sizeof(const char*) * a_device_list_count);
    for(int i=0;i<a_device_list_count; ++i){
        deviceNames[i] = a_device_list[i];
    }
    *count = a_device_list_count;
    return deviceNames;
}

void a_swap_device(const char* deviceName){
    a_allow = 0;

    alcDestroyContext(_a_ctx.context);
    alcCloseDevice(_a_ctx.device);

    ALCdevice* device = NULL;
    if(deviceName != NULL){
        for(int i=0;i<a_device_list_count;++i){
            if(strcmp(a_device_list[i], deviceName) == 0){
                deviceName = a_device_list[i];
                break;
            }
        }
    }
    alcOpenDevice(deviceName);

    ALCcontext* context = NULL;
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    _a_ctx.context = context;
    _a_ctx.device = device;

    a_allow = 1;
}

void a_clean_sources(a_src* sources, int count){
    for(int i=0;i<count;++i){
        alDeleteSources(1, &sources[i].id);
    }
}

void a_clean_buffers(a_buf* buffers, int count){
    for(int i=0;i<count;++i){
        alDeleteBuffers(1, &buffers[i].id);
    }
}

a_buf a_create_buffer(const char* path){
    if(path == NULL){
        printf("NULL Audio Buffer.\n");
        return (a_buf){0, 0, 0, 0, 0};
    }
    int id;

    alGenBuffers(1, &id);

    //TODO variant sample size
    int sample_size = 16;

    int channels, len, rate;
    short* data;

    stb_vorbis* vorbis;
    vorbis = stb_vorbis_open_filename(path, NULL, NULL);

    if(!vorbis){
        printf("Unable to open audio file with vorbis: %s\n", path);
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
                printf("Unsupported format: channels: %i sample_size: %i\n", info.channels, sample_size);
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
                printf("Unsupported format: channels: %i sample_size: %i\n", info.channels, sample_size);
                break;
        }
    }

    unsigned int total_samples;
    int loop = 0;

    total_samples = (unsigned int)stb_vorbis_stream_length_in_samples(vorbis);
    int sample_count = (MAX_MUSIC_RUNTIME > total_samples) ? total_samples : MAX_MUSIC_RUNTIME;

    void* pcm = calloc(total_samples * sample_size/8*info.channels, 1);

    int num_samples = stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, (short*)pcm, sample_count*info.channels);

    alBufferData(id, format, pcm, sample_count*(sample_size/8*info.channels), info.sample_rate);

    free(pcm);

    stb_vorbis_close(vorbis);

    return (a_buf){id, 1, channels, len, rate};
}

a_buf* a_create_buffers(const char** paths, int p_count, int* b_count){
    if(p_count == 0){
        return 0;
    }

    int buffered_count;
    a_buf* buffered = malloc(sizeof(a_buf) * p_count);
    for(int i=0;i<p_count;i++){
        if(buffered->buffered == 1){
            buffered_count ++;
        }
    }

    *b_count = buffered_count;

    return buffered;
}

void a_destroy_buffer(a_buf buffer){
    alDeleteBuffers(1, &buffer.id);
}

a_stream a_create_stream(int sample_rate, int sample_size, int channels){
    a_stream stream = {0};
    stream.sample_rate = sample_rate;
    stream.sample_size = sample_size;
    if((channels > 0) && (channels < 3)){
        stream.channels = channels;
    }else{
        printf("Invalid channel count in audio stream: %i\n", channels);
    }

    if(channels == 1){
        switch(sample_size){
            case 8: stream.format = AL_FORMAT_MONO8; break;
            case 16: stream.format = AL_FORMAT_MONO16; break;
            default: printf("Invalid sample size in stream %i, all sizese must be multiple of 8.\n", sample_size);
        }
    }else if(channels == 2){
        switch(sample_size){
            case 8: stream.format = AL_FORMAT_STEREO8; break;
            case 16: stream.format = AL_FORMAT_STEREO16; break;
            default: printf("Invalid sample size in stream %i, all sizese must be multiple of 8.\n", sample_size);
        }
    }

    alGenSources(1, &stream.source);
    alSourcef(stream.source, AL_PITCH, 1.0f);
    alSourcef(stream.source, AL_GAIN,  1.0f);
    alSource3f(stream.source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(stream.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    alGenBuffers(2, stream.buffers);

    void* pcm = calloc(MAX_MUSIC_RUNTIME * stream.sample_size/8*stream.channels, 1);
    for(int i=0;i<2;++i){
        alBufferData(stream.buffers[i], stream.format, pcm, stream.sample_size/8*stream.channels, stream.sample_rate);
    }

    free(pcm);
    alSourceQueueBuffers(stream.source, 2, stream.buffers);
    return stream;
}

void a_destroy_stream(a_stream stream){
    alSourceStop(stream.source);

    int queued = 0;
    alGetSourcei(stream.source, AL_BUFFERS_QUEUED, &queued);

    ALuint buffer = 0;
    while(queued > 0){
        alSourceUnqueueBuffers(stream.source, 1, &buffer);
        queued --;
    }

    alDeleteSources(1, &stream.source);
    alDeleteBuffers(2, stream.buffers);
}

void a_update_stream(a_stream stream, const void* data, int sample_count){
    ALuint buffer = 0;
    alSourceUnqueueBuffers(stream.source, 1, &buffer);
    if(alGetError() != AL_INVALID_VALUE){
        alBufferData(buffer, stream.format, data, sample_count*(stream.sample_size/8*stream.channels), stream.sample_rate);
        alSourceQueueBuffers(stream.source, 1, &buffer);
    }else{
        printf("Audio buffer not available for unqueueing: %i\n", buffer);
    }
}

a_music* a_create_music(const char* path){
    a_music* music = (a_music*)malloc(sizeof(a_music));
    music->vorbis = stb_vorbis_open_filename(path, NULL, NULL);
    if(music->vorbis == NULL){
        printf("Audio File could not be opened: %s\n", path);
    }else{
        stb_vorbis_info info = stb_vorbis_get_info(music->vorbis);

        music->stream = a_create_stream(info.sample_rate, 16, info.channels);

        if(music->stream.source == 0){
            printf("Unable to create music stream for: %s\n", path);
        }

        music->total_samples = (unsigned int)stb_vorbis_stream_length_in_samples(music->vorbis);
        music->samples_left = music->total_samples;
        music->loop = 0;
    }

    return music;
}

int a_update_music(a_music* music){
    ALenum state;
    ALint processed = 0;
    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);
    alGetSourcei(music->stream.source, AL_BUFFERS_PROCESSED, &processed);

    if(processed > 0){
        int ending = 0;
        void* pcm = calloc(MAX_MUSIC_RUNTIME * music->stream.sample_size/8*music->stream.channels, 1);
        int num_left = processed;
        int sample_count = 0;

        for(int i=0;i<num_left;++i){
            if(music->samples_left >= MAX_MUSIC_RUNTIME){
                sample_count = MAX_MUSIC_RUNTIME;
            }else{
                sample_count = music->samples_left;
            }

            int num_samples = stb_vorbis_get_samples_short_interleaved(music->vorbis, music->stream.channels, (short*)pcm, sample_count*music->stream.channels);
            a_update_stream(music->stream, pcm, sample_count);
            music->samples_left -= sample_count;

            if(music->samples_left <= 0){
                ending = 1;
                break;
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
}

void a_destroy_music(a_music* music){
    a_destroy_stream(music->stream);
    stb_vorbis_close(music->vorbis);
    free(music);
}

float a_get_music_time(a_music* music){
    unsigned int samples = music->total_samples - music->samples_left;
    return (float)samples / music->stream.sample_rate;
}

float a_get_music_len_time(a_music* music){
    return (float)(music->total_samples/music->stream.sample_rate);
}

void a_play_music(a_music* music){
    alSourcePlay(music->stream.source);
}

void a_stop_music(a_music* music){
    alSourceStop(music->stream.source);
    stb_vorbis_seek_start(music->vorbis);
    music->samples_left = music->total_samples;
}

void a_resume_music(a_music* music){
    ALenum state = a_get_music_state(music);
    if(state == AL_PAUSED){
        alSourcePlay(music->stream.source);
    }
}

void a_pause_music(a_music* music){
    alSourcePause(music->stream.source);
}

int a_get_music_state(a_music* music){
    int state;
    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);
    return state;
}

a_src a_create_source(v3 position, float range, unsigned int buffer){
    unsigned id;
    alGenSources(1, &id);

    if(&position == NULL){
        position = (v3){0.0f, 0.0f, 0.0f};
    }

    alSourcefv(id, AL_POSITION, &position.v[0]);
    alSourcef(id, AL_GAIN, 1.0f);
    alSourcei(id, AL_LOOPING, AL_FALSE);

    if(buffer != 0){
        alSourcei(id, AL_BUFFER, buffer);
        return (a_src){id, position, range, 1.0f, 1, 0};
    }
    return (a_src){id, position, range, 1.0f, 0, 0};
}

void a_destroy_source(a_src source){
    alDeleteSources(1, &source.id);
}

void a_play_source(a_src* source){
    alSourcePlay(source->id);
}

void a_pause_source(a_src* source){
    alSourcePause(source->id);
}

void a_stop_source(a_src* source){
    alSourceStop(source->id);
}

ALenum a_src_state(a_src* source){
    ALenum state;
    alGetSourcei(source->id, AL_SOURCE_STATE, &state);
    return state;
}

static int a_get_open_sfx(){
    for(int i=0;i<MAX_QUICK_SFX;++i){
        int buffer;
        alGetSourcei(a_qsfx[i].source.id, AL_BUFFER, &buffer);
        if(!buffer){
            return i;
        }
		_l("Checking slot: %d, contains buffer: %d\n", i, buffer);
    }
    printf("No open quick sfx slots\n");
    return -1;
}

int a_play_sfx(a_buf* buffer, float gain, v2 pos){
    int sfx_slot = 1;

    if(sfx_slot == -1){
        printf("Unable to play Quick SFX\n");
        return -1;
    }

    alSourcef (a_qsfx[sfx_slot].source.id, AL_GAIN, 1.f);
    alSource3f(a_qsfx[sfx_slot].source.id, AL_POSITION, pos.x, pos.y, 0.f);
    alSource3f(a_qsfx[sfx_slot].source.id, AL_VELOCITY, 0.f, 0.f, 0.f);

    alSourcei(a_qsfx[sfx_slot].source.id, AL_BUFFER, buffer->id);
    a_qsfx[sfx_slot].source.has_sound = 1;
    a_play_source(&a_qsfx[sfx_slot].source);

    return sfx_slot;
}

void a_pause_sfx(int sfx_slot){
    a_pause_source(&a_qsfx[sfx_slot].source);
}

void a_stop_sfx(int sfx_slot){
    a_stop_source(&a_qsfx[sfx_slot].source);
}

int a_is_sfx_buffer(int index, a_buf* buffer){
    int buf;
    alGetSourcei(a_qsfx[index].source.id, AL_BUFFER, &buf);
    return buf == buffer->id;
}

ALenum a_get_sfx_state(int index){
    ALenum state;
    alGetSourcei(a_qsfx[index].source.id, AL_SOURCE_STATE, &state);
    return state;
}

void a_update_sfx(){
    for(int i=0;i<MAX_QUICK_SFX;++i){
        ALenum state = a_get_sfx_state(i);
        if(state == AL_STOPPED){
            int buffer;
            alGetSourcei(a_qsfx[i].source.id, AL_BUFFER, &buffer);
            if(buffer != 0){
                alSourcei(a_qsfx[i].source.id, AL_BUFFER, 0);
                a_qsfx[i].source.has_sound = 0;
            }
        }
    }
}

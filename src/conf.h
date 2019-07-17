#ifndef CONF_H
#define CONF_H

typedef struct {
	int render;
	int audio;
	int debug;
	int verbose;
} c_args;

typedef struct {
	unsigned int width, height;
	unsigned int fullscreen, refreshRate;
	unsigned int vsync, borderless;
	unsigned int master;
	unsigned int music, sfx;	
} c_conf;


static c_args conf_flags;

void c_parse_args(int argc, const char** argv);
c_conf c_parse_file(const char* f);

int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif

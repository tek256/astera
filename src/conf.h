#ifndef CONF_H
#define CONF_H

typedef struct {
	int render;
	int audio;
	int debug;
	int verbose;
} c_args;

static c_args conf_flags;

void c_parse_args(int argc, const char** argv);

int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif

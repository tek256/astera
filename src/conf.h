#ifndef CONF_H
#define CONF_H

typedef struct {
} c_user;

typedef struct {
	int render;
	int audio;
	int debug;
	int verbose;
} c_args;

typedef struct {
	const char** keys;
	const char** values;
	unsigned int count;
	unsigned int size;
} c_map;

static c_args conf_flags;

void c_parse_args(int argc, const char** argv);
void c_parse_file(const char* f, c_map* map);
void c_unload_map(c_map* map);

int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif

#ifndef CONF_H
#define CONF_H

typedef struct {
	int render;
	int audio;
	int debug;
	int verbose;
	char* prefs;
} c_args;

typedef struct {
	unsigned int width, height;
	unsigned int fullscreen, refreshRate;
	unsigned int vsync, borderless;
	unsigned int master;
	unsigned int music, sfx;
	char* path;
} c_conf;

void c_parse_args(int argc, const char** argv);
c_conf c_parse_file(const char* f, int prefs);

#ifndef EXCLUDE_CREATE
#include "game.h"
void c_write_level(const char* fp, g_level* level);
#endif

//void c_write_table(const char* table, char* keys, char* values, int count);

unsigned char* c_get_file_contents(const char* fp, int* size);

int c_has_prefs();
char* c_get_pref_p();
int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif

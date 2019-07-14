#ifndef CONF_H
#define CONF_H

#include <toml.h>

typedef struct {
} c_user;

typedef struct {
	int render;
	int audio;
	int debug;
	int verbose;
} c_args;

typedef struct {
	char** keys;
	void** values;
	unsigned int count;
	unsigned int capacity;
} c_map;

static c_args conf_flags;

void c_alloc_map(c_map* map, int len);

void c_add_to_map(c_map* map, char* val, void* value);
static void c_get_val(const char* var, toml_table_t* table, unsigned char type, void* ptr);
static void c_get_vali(const char* raw, unsigned char type, void* ptr);

void c_parse_args(int argc, const char** argv);
void c_parse_file(char* f, c_map* map);
void c_unload_map(c_map* map);

int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif

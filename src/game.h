#ifndef GAME_H
#define GAME_H 

#include "phys.h"

typedef struct {
	vec2 pos, size, vel;
	//p_entity* phys;
	unsigned int uid;
	unsigned int type;
} g_entity;

typedef struct {
	vec2 pos, size;
	unsigned int uid;
	unsigned int type;
} g_obj;

typedef struct {
	g_entity* entities;
	unsigned int count;
	unsigned int capacity;

	g_obj* objs;
	unsigned int obj_count;
	unsigned int obj_capacity;	
} g_chunk;

typedef struct {
	g_chunk* chunks;
	unsigned int chunk_count;
	const char* fp;
} g_level;

g_level g_init_level(int entity_count, int obj_count);

int g_init();
void g_exit(); 
void g_input(long delta);
void g_update(long delta);
void g_render(long delta);

#endif

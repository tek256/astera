#ifndef GAME_H
#define GAME_H 

#include "phys.h"

typedef struct {
	vec2 pos, size, vel;
	//p_entity* phys;
	unsigned int uid;
} g_entity;

int g_init();
void g_exit(); 
void g_input(long delta);
void g_update(long delta);
void g_render(long delta);

#endif

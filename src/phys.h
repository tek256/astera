#ifndef PHYS_H
#define PHYS_H

#define P_CIRCLE 0
#define P_AABB   1
#define P_PLANE  2
#define P_BOX    3

#define P_MAX_EVENTS 128

#define MAX_REC_COLS 128

#include "platform.h"
#include "game.h"
#include <linmath.h>

typedef struct {
	vec2 pos;
	vec2 size;
} aabb;

typedef struct {
	vec2 pos;
	float radius; 
} circ;

typedef struct {
	vec2 pos;
	float rot;
	float length;
} plane;

typedef struct {
	vec2 pos;
	vec2 size;
	float rot;
} box;

typedef struct {
	union {
		box b;
		plane p;
		circ c;
	} collider;	
	u8 collider_type;
} world_col;

typedef struct {
	world_col cols[MAX_REC_COLS];
	u16 _ccount;
} p_world_map;

typedef struct {
	union {
		aabb _aabb;
		circ _circ;
		plane _plane;
		box _box;
	} col;
	char type;
} collider;

typedef	char p_layer;

typedef struct {
	collider col;
	vec2 vel;
	unsigned int uid;
	p_layer tags;
	p_layer layer;
	int active : 1;
	int event : 1;
} p_entity;

typedef struct {
	p_entity* entities;
	unsigned int uid;
	unsigned int count, capcity;
} p_ent_list;

typedef struct {
	vec2 pos;
	vec2 size;
	p_entity** entities;
	unsigned int count;
	unsigned int capacity;	
} p_block;

typedef struct {
	long delta_ago;
	int uid_a, uid_b;
	vec2 solution;
	vec2 contact;
} p_event;

typedef struct {
	p_event events[P_MAX_EVENTS];
	unsigned int count, capacity;
} p_event_stack;

typedef struct {
	p_block* blocks;
	unsigned int count;
} p_map;

/*void p_init(vec2 size, vec2 block_size);
void p_load(g_entity* entities, unsigned int count);
void p_swap(g_entity* entities, unsigned int count);
void p_update(long delta);
void p_selective_update(p_block* block, long delta);
int p_interact(vec2 potential, p_entity* a, p_entity* b);
void p_resolve(vec2 potential, p_entity* a, p_entity* b);
void p_exit();

void p_circle_aabb(vec2 sol, circ* _ci, aabb* _aa);
void p_circle_plane(vec2 sol, circ* _ci, plane* _pl);
void p_circle_box(vec2 sol, circ* _ci, box* _b);
void p_circle_circle(vec2 sol, circ* _ci, circ* _ci2);
void p_box_aabb(vec2 sol, box* _b, aabb* _aa);
void p_box_plane(vec2 sol, box* _b, plane* _pl);
void p_box_box(vec2 sol, box* _b, box* _b2);
void p_aabb_aabb(vec2 sol, aabb* _aa, aabb* _bb);
void p_aabb_plane(vec2 sol, aabb* _aa, plane* _pl);
void p_plane_plane(vec2 sol, plane* _pl, plane* _pl2);
*/
#endif

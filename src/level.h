#ifndef LEVEL_H
#define LEVEL_H

#include "platform.h"

#include "render.h"
#include "audio.h"

#define CREATE_MODE

#define MAX_REC_TEX 
#define MAX_REC_SHD 4
#define MAX_REC_CHUNK 32

//per chunk
#define MAX_REC_OBJ 128
#define MAX_REC_ENT 64

#define MIN_ENT_BUFF 4
#define MIN_OBJ_BUFF 16

typedef struct {
	u16 uid;
	u16 gid;
	vec2 pos;
} l_ent_res;

typedef struct {
	u16 uid;
	u16 gid;
	vec2 pos;
} l_obj_res;

typedef struct {
	u16 uid, gid;
	vec2 pos, size;		
} l_ent;

typedef struct {
	u16 uid, gid;
	vec2 pos, size;
} l_obj;

#ifdef CREATE_MODE
typedef struct {
	l_obj_res objs[MAX_REC_OBJ * MAX_REC_CHUNK];
	u16 obj_count;

	l_ent_res ents[MAX_REC_ENT * MAX_REC_CHUNK];
	u16 ent_count;

	//r_resource_map* r_map;
	//p_world_map* p_map;
	a_resource_map* a_map;
} l_resource;
#endif

typedef struct {
	u32 x, y;

	l_obj** objs;
	l_ent** ents;
	
	//array attributes
	u16 obj_count;
	u16 obj_cap;

	u16 ent_count;
	u16 ent_cap;
} l_chunk;

typedef struct {
	l_chunk* chunks;
	u16 	 _count;
	u16      _cap;

	//actual storage for entities & objects
	l_ent* ents;
	u16 ent_count;
	u16 ent_cap;

	l_obj* objs;
	u16 obj_count;
	u16 obj_cap; 

#ifdef CREATE_MODE
	l_resource _res;
#endif

} l_level;

typedef struct {
	u16 x, y;

	l_obj_res* objs;
	l_ent_res* ents;

	u16 ent_count;
	u16 obj_count;	
} l_chunk_data;

typedef struct {
	//in chunks
	u16 x, y;
	u16 width, height;
} l_observer;

typedef struct {
	l_chunk_data* chunks;
	u16 chunk_count;	
	
	//# of chunks per axis	
	u16 chunks_x, chunks_y;

	//chunk size in gamespace
	u16 chunk_width, chunk_height;
	
	//get the level's default observer
	l_observer observer;
} l_data;

typedef struct {
	union {
		u16 eid;
		u16 oid;
	};
    u16 type;
	u16 event;	
} l_event; 

typedef struct {
	l_event* events;
	u32 count;
	u32 capacity;
} l_diff;

static l_observer _observer;

l_ent* l_create_ent(l_ent* out, l_ent_res res);
l_obj* l_create_obj(l_obj* out, l_obj_res res);

void l_set_observer(u16 x, u16 y, u16 width, u16 height);
void l_move_observer(u16 x, u16 y);
void l_get_observer(l_observer* ob);

int l_load(l_level* dst, l_data* data);
l_level l_create_lvl(u16 chunks_w, u16 chunks_h);

//data, info, chunks (int[]*), number of chunk arrays / positions
int l_stream(l_data* data);

int l_init();

#ifdef CREATE_MODE
int l_write_level(const char* fp, l_level* level);
#endif

#endif

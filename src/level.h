#ifndef LEVEL_H
#define LEVEL_H

#include "platform.h"

#include "render.h"

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
	vec2 pos, size;		
} l_ent;

typedef struct {
	vec2 pos, size;
} l_obj;

#ifdef CREATE_MODE
typedef struct {
	l_obj_res objs[MAX_REC_OBJ * MAX_REC_CHUNK];
	u16 _ocount;

	l_ent_res ents[MAX_REC_ENT * MAX_REC_CHUNK];
	u16 _ecount;

	r_resource_map* r_map;
	p_world_map* p_map;
	a_res_map* a_map;
} l_resource;
#endif

typedef struct {
	u32 x, y;

	l_obj** objs;
	l_ent** ents;
	
	//array attributes
	u16 _ocount;
	u16 _ecount;
	u16 _ocap;
	u16 _ecap;
} l_chunk;

typedef struct {
	l_chunk* chunks;
	u16 	 _count;
	u16      _cap;
	
	#ifdef CREATE_MODE
	l_resource _res;
	#endif	
} l_level;

#ifdef CREATE_MODE
l_resource l_create_res();
#endif

l_ent l_create_ent(l_ent* out, u32 id);
l_obj l_create_obj(l_obj* out, u32 id);

l_level l_create_lvl(u16 chunks_w, u16 chunks_h);

#endif

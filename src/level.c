#include "level.h"
/*
#include <assert.h>
#include <stdio.h>

void l_create_ent(l_ent* out, u32 id){
	switch(id){

	}
}

void l_create_obj(l_obj* out, u32 id){
	switch(id){

	}
}

#ifdef CREATE_MODE 
l_resource l_create_res(){
	l_resource res;
}
#endif

l_level l_create_lvl(u16 chunks_w, u16 chunks_h){
	l_level level;

	int _chunk_count = chunks_w * chunks_h;

	l_chunk* _chunks = (l_chunk*)malloc(sizeof(l_chunk) * _chunk_count);
	for(int x=0;x<chunks_w;++x){
		for(int y=0;y<chunks_h;++y){
			l_chunk* chunk = &_chunks[x+(x*y)];
			chunk->x = x;
			chunk->y = y;
			chunk->_ents = (l_ent*)malloc(sizeof(l_ent) * MIN_ENT_BUFF);
			chunk->_objs = (l_obj*)malloc(sizeof(l_obj) * MIN_OBJ_BUFF);

			chunk->_ecap = MIN_ENT_BUFF;
			chunk->_ocap = MIN_OBJ_BUFF;
		}
	}	

	level.chunks = _chunks;
	level._count = _chunk_count;
	level._cap = _chunk_count;

	#ifdef CREATE_MODE
	level._res = l_create_res();
	#endif

	return level;	
}

int l_write_level(const char* fp, l_level* level){
	char str_buff[512];
	char table_buff[32];
	char type_buff[32];

	int w1, w2, w3;

	//insure the buffers are 0'd out
	memset(str_buff, 0, sizeof(char) * 512);
	memset(table_buff, 0, sizeof(char) * 32);
	memset(type_buff, 0, sizeof(char) * 32);

	l_resource res = level->_res;

	FILE* f = fopen(fp, "rw+");
	if(!f){
		_e("Unable to open file to write: %s\n", fp);
		return 0;
	}
	
	//entity writing	
	for(int i=0;i<res._ecount;++i){
		l_ent_res* ent = &res.ents[i];
		w1 = sprintf(type_buff, "%d,%d,%f,%f:", ent->uid, ent->gid, ent->pos[0], ent->pos[1]);
		w2 = fwrite(type_buff, sizeof(char), size, f);	
		if(!w2){
			_e("Unable to write [%s] to %s.\n", type_buff, fp);
			continue;
		}
		memset(type_buff, 0, sizeof(char) * written);
	}

	//object writing
	for(int i=0;i<res._ocount;++i){
		l_obj_res* obj = &res.objs[i];
		w1 = sprintf(type_buff, "%d,%d,%f,%f:", obj->uid, obj->gid, obj->pos[0], obj->pos[1]);
		w2 = fwrite(type_buff, sizeof(char), size, f);	
		if(!w2){
			_e("Unable to write [%s] to %s.\n", type_buff, fp);
			continue;
		}
		memset(type_buff, 0, sizeof(char) * written);
	}

	/*
	 *typedef struct {

	const char* vertex_shaders[RENDER_SHADER_CACHE];
	const char* fragment_shaders[RENDER_SHADER_CACHE];
	r_shader shader_ids[RENDER_SHADER_CACHE];
	u32 shader_count;
	
	u32 anim_ids[RENDER_ANIM_CACHE];
	r_anim anims[RENDER_ANIM_CACHE];
	u32 anim_count;

	const char* sheet_paths[RENDER_SHEET_CACHE];
	u32 sheet_ids[RENDER_SHEET_CACHE];
	u32 sheet_subwidths[RENDER_SHEET_CACHE];
	u32 sheet_subheights[RENDER_SHEET_CACHE];
	u32 sheet_count;

	const char* tex_paths[RENDER_SHEET_CACHE];
	u32 tex_ids[RENDER_SHEET_CACHE];
	u32 tex_count;
	//*\/ 
	r_resource_map* r_map = res.r_map;
	
	for(int i=0;i<r_map->shader_count;++i){
		const char* vert, frag;
		vert = r_map->vertex_shaders[i];
		frag = r_map->fragment_shaders[i];

		char* vert_data = (char*)c_get_file_contents(vert, NULL);
		char* frag_data = (char*)c_get_file_contents(frag, NULL);

		//Hopefully theres are null terminated.
		w1 = sprintf(str_buff, "%s%s%s\n", r_map->shader_names[i], vert_data, frag_data);

	   	free(vert_data);
   		free(frag_data);

		fwrite(str_buff, sizeof(char), w1, f);
		memset(str_buff, 0, sizeof(char) * w1);
	}

	for(int i=0;i<r_map->anim_count;++i){
		const char* name = r_map->anim_names[i];
		r_anim* anim = &r_map->anims[i];	
		w1 = sprintf(str_buff, "%d,%d,%d,%s,",anim->frame_count, anim->frame_rate, anim->sheet_id, name);
		for(int j=0;j<anim->frame_count;++j){
			w2 += sprintf(type_buff,"%d,",anim->frames[j]);
			strcat(str_buff, type_buff);
			memset(type_buff, 0, w2 * sizeof(char));
			w1 += w2;
		}
		strcat(str_buff, "\n");
		w1 += 1;

		fwrite(str_buff, sizeof(char), w1, f);
		memset(str_buff, 0, sizeof(char) * w1);
	}

	for(int i=0;i<r_map->sheet_count;++i){
		char* sheet_data = c_get_file_contents(r_map->sheet_paths[i], NULL);
		
	}

	a_resource_map* a_map = res.a_map;
}*/

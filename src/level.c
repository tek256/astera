#include "level.h"
#include "debug.h"

#include <assert.h>
#include <stdio.h>

#ifdef CREATE_MODE
static l_resource* res;
#endif

l_ent* l_create_ent(l_ent* out, l_ent_res ent){
	if(!out) return NULL;
	switch(ent.uid){
   	default:
		_e("Unknown ent type: %i\n", ent.uid);
	   break;	
	}

	return out;
}

l_obj* l_create_obj(l_obj* out, l_obj_res obj){
	if(!out) return NULL;
	switch(obj.uid){
	default:
		_e("Unknown obj type: %i\n", obj.uid);
		break;
	}

	return out;
}

int l_init(){
#ifdef CREATE_MODE
	res = (l_resource*)malloc(sizeof(l_resource));
	if(!res){
		_e("Unable to malloc level resources.\n");
		return 0;
	}
#endif	

	return 1;
}

l_level l_create_lvl(u16 chunks_w, u16 chunks_h){
	/*l_level level;

	int _chunk_count = chunks_w * chunks_h;

	l_chunk* _chunks = (l_chunk*)malloc(sizeof(l_chunk) * _chunk_count);
	for(int x=0;x<chunks_w;++x){
		for(int y=0;y<chunks_h;++y){
			l_chunk* chunk = &_chunks[x+(x*y)];
			chunk->x = x;
			chunk->y = y;
			chunk->ents = (l_ent*)malloc(sizeof(l_ent*) * MIN_ENT_BUFF);
			chunk->objs = (l_obj*)malloc(sizeof(l_obj*) * MIN_OBJ_BUFF);

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

	return level;*/
	return (l_level){0};
}

void l_set_observer(u16 x, u16 y, u16 width, u16 height){
	_observer.x = x;
	_observer.y = y;
	_observer.width = width;
	_observer.height = height;	
}

void l_move_observer(u16 x, u16 y){
	_observer.x = x;
	_observer.y = y;
}

void l_get_observer(l_observer* ob){
	if(ob){
		*ob = _observer;
	}
}

int l_load(l_level* dst, l_data* data){
	if(!data){
		_e("No level data passed.\n");
		return 0;
	}

	u16 min_x, max_x;
	u16 min_y, max_y;
	u16 chunk_w, chunk_h; 

	l_observer ob = data->observer;

	min_x = (ob.x - (ob.width / 2) <= 0) ? 0 : ob.x - (ob.width / 2);
	min_y = (ob.y - (ob.height / 2) <= 0) ? 0 : ob.y - (ob.height / 2);
	max_x = ob.x + (ob.width / 2);
	max_y = ob.y + (ob.height / 2);

	chunk_w = max_x - min_x;
	chunk_h = max_y - min_y;

	u16 to_process =  chunk_w * chunk_h;
	u16 processed = 0;

	u16 ent_count;
	u16 obj_count;

	l_level level = (l_level){0};
	level.chunks = (l_chunk*)malloc(sizeof(l_chunk) * to_process);
	if(!level.chunks){
		_e("Unable to malloc chunks.\n");
		return 0;
	}

	for(int i=0;i<data->chunk_count;++i){
		l_chunk_data* chunk_data = &data->chunks[i];
		l_chunk* chunk;

		if(chunk_data->x >= min_x && chunk_data->x < max_x){
			if(chunk_data->y >= min_y && chunk_data->y < max_y){
				ent_count += chunk_data->ent_count;
				obj_count += chunk_data->obj_count;

				++processed;
				if(processed >= to_process) break;
			}
		}	
	}

	level.ents = (l_ent*)malloc(sizeof(l_ent) * ent_count);
	level.objs = (l_obj*)malloc(sizeof(l_obj) * obj_count);

	level.obj_cap = obj_count;
	level.ent_cap = ent_count;
		
	for(int i=0;i<data->chunk_count;++i){
		l_chunk_data* chunk_data = &data->chunks[i];	
		l_chunk* chunk = &level.chunks[level._count];

		u16 in_bounds = 0;
		if(chunk_data->x >= min_x && chunk_data->x < max_x){
			if(chunk_data->y >= min_y && chunk_data->y < max_y){
				in_bounds = 1;
			}
		}	

		if(!in_bounds) continue;

		for(int j=0;j<chunk_data->obj_count;++j){
			l_obj* obj = l_create_obj(&level.objs[level.obj_count], chunk_data->objs[j]);		
			chunk->objs[chunk->obj_count] = obj;
			++chunk->obj_count;	

			if(chunk->obj_count >= chunk->obj_cap)
				break;
		}

		for(int j=0;j<chunk_data->ent_count;++j){
			l_ent* ent = l_create_ent(&level.ents[level.ent_count], chunk_data->ents[j]);		
			chunk->ents[chunk->ent_count] = ent;
			++chunk->ent_count;	

			if(chunk->ent_count >= chunk->ent_cap)
				break;
		}

		++processed;
		if(processed >= to_process) break;
	}

	*dst = level;

	return 1;
}

#ifdef CREATE_MODE
int l_write_level(const char* fp, l_level* level){
	/*char str_buff[512];
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
	for(int i=0;i<res.ent_count;++i){
		l_ent_res* ent = &res.ents[i];
		w1 = sprintf(type_buff, "%d,%d,%f,%f:", ent->uid, ent->gid, ent->pos[0], ent->pos[1]);
		w2 = fwrite(type_buff, sizeof(char), w1, f);	
		if(!w2){
			_e("Unable to write [%s] to %s.\n", type_buff, fp);
			continue;
		}
		memset(type_buff, 0, sizeof(char) * w2);
	}

	//object writing
	for(int i=0;i<res.obj_count;++i){
		l_obj_res* obj = &res.objs[i];
		w1 = sprintf(type_buff, "%d,%d,%f,%f:", obj->uid, obj->gid, obj->pos[0], obj->pos[1]);
		w2 = fwrite(type_buff, sizeof(char), w1, f);	
		if(!w2){
			_e("Unable to write [%s] to %s.\n", type_buff, fp);
			continue;
		}
		memset(type_buff, 0, sizeof(char) * w2);
	}

	r_resource_map* r_map = res.r_map;
	
	for(int i=0;i<r_map->shader_count;++i){
		const char* vert, *frag;
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
	*/
	return 1;
}
#endif

#include "conf.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "game.h"

#include <ini.h>
#include <ini.c>

#include "render.h"
#include "audio.h"

#define _MASTER

#define match(s,n) strcmp(section, s) == 0 && strcmp(name, n) == 0

//TODO key value checking in INI file

static int parse_handler(void* user, const char* section, const char* name, const char* value){
	c_conf* c = (c_conf*)user;

	if(strcmp(section, "render") == 0){
		if(strcmp(name, "width") == 0){	
			c->width = atoi(value);
		}else if(strcmp(name, "height") == 0){
			c->height = atoi(value);
		}else if(strcmp(name, "fullscreen") == 0){
			c->fullscreen = atoi(value);
		}else if(strcmp(name, "borderless") == 0){		
			c->borderless = atoi(value);
		}else if(strcmp(name, "refreshRate") == 0){
			c->refreshRate = atoi(value);
		}else if(strcmp(name, "vsync") == 0){
			c->vsync = atoi(value);
		}
	}else if(strcmp(section, "audio") == 0){
		if(strcmp(name, "master") == 0){
			c->master = atoi(value);	
		}else if(strcmp(name, "music") == 0){
			c->music = atoi(value);
		}else if(strcmp(name, "sfx") == 0){
			c->sfx = atoi(value);
		}
	}else if(strcmp(section, "input") == 0){
		int binding = atoi(value);	
		int type = (binding / 100) + 1;
		
		binding = binding % 100;
		i_add_binding(strdup(name), binding, type);
		_l("Adding binding: %s %i %i\n", name, binding, type);
	}else{
		return 0;
	}

	return 1;
}

c_conf c_parse_file(const char* fp, int prefs){
	if(prefs){
		c_prefs_path = fp;		
	}

	c_conf conf = (c_conf){1280, 720, 0, 60, 1, 0, 100, 75, 50};
	if(ini_parse(fp, parse_handler, &conf) < 0){
		_e("Unable to open: %s\n", fp);
		return conf;
	}
	return conf;
}

void c_parse_args(int argc, const char** argv){
	if(!argv || !argc){
		return;
	}

	conf_flags = (c_args){
		0, 1, 1, 0
	};

	while(1){
		char c = getopt(argc, argv, "srvadc:");

		if(c == -1){
			break;
		}

		switch(c){
			case 's':
				conf_flags.verbose = -1;
				break;
			case 'r':
				conf_flags.render = 0;
				break;
			case 'a':
				conf_flags.audio = 0;
				break;
			case 'v':
				conf_flags.verbose = 1;
				break;
			case 'd':
				conf_flags.debug = 1;
				break;
			case 'c':
				//TODO include optional config override
				break;
		}
	}

	for(int i=0;i<argc;++i){
		if(!strcmp(argv[i], "debug")){
			conf_flags.debug = 1;	
		}else if(!strcmp(argv[i], "no_render")){
			conf_flags.render = 0;
		}else if(!strcmp(argv[i], "no_audio")){
			conf_flags.audio = 0;
		}else if(!strcmp(argv[i], "verbose") || !strcmp(argv[i], "v")){
			conf_flags.verbose = 1;	
		}else if(!strcmp(argv[i], "silent") || !strcmp(argv[i], "s")){
			conf_flags.verbose = -1;
		}
	}
}

void c_write_pref(const char* fp, const char* section, const char* key, const char* value){
	ini_write_kv(fp, section, key, value);
}

void c_write_table(const char* fp, const char* section, const char** keys, const char** values, int count){
	ini_write_table(fp, section, keys, values, count);
}

#ifndef EXCLUDE_CREATE
//Only include in master copy.
void c_write_level(const char* fp, g_level* level){
	char str_buff[512];
	char table_buff[32];
	char type_buff[32];

	//insure the buffers are 0'd out
	memset(str_buff, 0, sizeof(char) * 512);
	memset(table_buff, 0, sizeof(char) * 32);
	memset(type_buff, 0, sizeof(char) * 32);

	static int unique_entities[128];
	static int unique_objs[128];

	int obj_count = 0;
	int ent_count = 0;

	r_resource_map* r_res = r_get_map();
	if(!r_res){
		_e("Unable to load rendering resource map.\n");
	}

	for(int i=0;i<level->chunk_count;++i){
		g_chunk chunk = level->chunks[i];	
		
		for(int j=0;j<chunk.count;++j){
			int contained = 0;
			for(int k=0;k<ent_count;++k){
				if(unique_entities[k] == chunk.entities[j].type){
					contained = 1;
					break;
				}
			}

			if(!contained && ent_count < 128){
				unique_entities[ent_count] = chunk.entities[j].type;
				++ent_count;
				sprintf(str_buff, "%i,", chunk.entities[j].type);
			}
		}

		sprintf(table_buff, "chunk_%i", i);

		c_write_pref(fp, table_buff, "ents", str_buff);
		memset(str_buff, 0, sizeof(char) * 512);

		sprintf(str_buff,"%i", ent_count);
		c_write_pref(fp, table_buff, "ent_count", str_buff);
		memset(str_buff, 0, sizeof(char) * 512);

		for(int j=0;j<chunk.obj_count;++j){
			int contained = 0;
			for(int k=0;k<obj_count;++k){
				if(unique_objs[k] == chunk.objs[j].type){
					contained = 1;
					break;
				}
			}

			if(!contained && obj_count < 128){
				unique_objs[ent_count] = chunk.objs[j].type;
				++obj_count;
				sprintf(str_buff, "%i", chunk.objs[j].type);	
			}
		}

		c_write_pref(fp, table_buff, "objs", str_buff);
		memset(str_buff, 0, sizeof(char) * 512);

		sprintf(str_buff, "%i", obj_count);
		c_write_pref(fp, table_buff, "obj_count", str_buff);
		memset(str_buff, 0, sizeof(char) * 512);

		int str_buff_pos = strlen(str_buff);
		for(int j=0;j<ent_count;++j){
			int type = unique_entities[j];
			int type_count = 0;
			sprintf("ent_%i", type_buff, type);

			for(int k=0;k<chunk.count;++k){
				if(chunk.entities[k].type == type){
					++type_count;
					int count = sprintf(str_buff, "%i,", chunk.entities[k].uid);
					str_buff_pos += count;

					if(str_buff_pos >= 506){
						c_write_pref(fp, table_buff, type_buff, str_buff);
						memset(str_buff, 0, sizeof(char) * 512);
					}	
				}

				if(str_buff_pos > 1 && type_count > 0){
					c_write_pref(fp, table_buff, type_buff, str_buff);
					
					memset(str_buff, 0, sizeof(char) * 512);
					memset(type_buff, 0, sizeof(char) * 32);
					
					int _tb_count = sprintf(type_buff, "ent_%i_count", type);
					int _sb_count = sprintf(str_buff, "%d", type_count);

					c_write_pref(fp, table_buff, type_buff, str_buff);
					
					memset(str_buff, 0, sizeof(char) * _sb_count);
					memset(type_buff, 0, sizeof(char) * _tb_count);
				}
			}	
		}

		for(int j=0;j<obj_count;++j){
			int type = unique_objs[j];
			int type_count = 0;
			sprintf("obj_%i", type_buff, type);

			for(int k=0;k<chunk.obj_count;++k){
				if(chunk.objs[k].type == type){
					++type_count;
					int count = sprintf(str_buff, "%i,", chunk.objs[k].uid);
					str_buff_pos += count;

					if(str_buff_pos >= 506){
						c_write_pref(fp, table_buff, type_buff, str_buff);
						memset(str_buff, 0, sizeof(char) * str_buff_pos);
					}	
				}

				if(str_buff_pos > 1 && type_count > 0){
					c_write_pref(fp, table_buff, type_buff, str_buff);
					
					memset(str_buff, 0, sizeof(char) * 512);
					memset(type_buff, 0, sizeof(char) * 32);
					
					int _tb_count = sprintf(type_buff, "obj_%i_count", type);
					int _sb_count = sprintf(str_buff, "%d", type_count);

					c_write_pref(fp, table_buff, type_buff, str_buff);
					
					memset(str_buff, 0, sizeof(char) * _sb_count);
					memset(type_buff, 0, sizeof(char) * _tb_count);
				}
			}	
		}
	}
	
	unsigned int _old_tex_ids[64];
	unsigned int _new_tex_ids[64];
	const char* tex_paths[64];
	unsigned int tex_id_count = 0;

	for(int i=0;i<r_res->tex_count;++i){
		if(r_res->tex_paths[i]){
			int exists = 0;
			for(int j=0;j<tex_id_count;++j){
				if(_old_tex_ids[j] == r_res->tex_ids[i]){
					exists = 1;
					break;
				}
			}
	
			if(exists){
				break;
			}

			tex_paths[tex_id_count] = r_res->tex_paths[i];
			_old_tex_ids[tex_id_count] = r_res->tex_ids[i];
			_new_tex_ids[tex_id_count] = tex_id_count;
			++tex_id_count;

			int _val_count = sprintf(str_buff, "%d,%s", _new_tex_ids[i], tex_paths[i]);
			c_write_pref(fp, "texs", "tex", str_buff);
			memset(str_buff, 0, sizeof(char) * _val_count);
		}
	}

	unsigned int _old_sheet_ids[64];
	unsigned int _new_sheet_ids[64];
	const char* sheet_paths[64];
	unsigned int _subwidths[64];
	unsigned int _subheights[64];
	unsigned int sheet_count = 0;

	for(int i=0;i<r_res->sheet_count;++i){
		if(r_res->sheet_paths[i]){
			int exists = 0;
			for(int j=0;j<sheet_count;++j){
				if(r_res->sheet_ids[i] == _old_sheet_ids[j]){
					exists = 1;
				}	
			}
			
			if(exists){
				break;
			}

			sheet_paths[sheet_count] = r_res->sheet_paths[i];
			_old_sheet_ids[sheet_count] = r_res->sheet_ids[i];
			_new_sheet_ids[sheet_count] = sheet_count;
			++sheet_count;
			
			int _val_count = sprintf(str_buff, "%d,%d,%d,%s",_new_sheet_ids[i], _subwidths[i], _subheights[i], sheet_paths[i]); 
			c_write_pref(fp, "sheets", "sheet", str_buff);
		   	memset(str_buff, 0, sizeof(char) * _val_count);	
		}
	}

	unsigned int _old_anim_ids[128];
	unsigned int _new_anim_ids[128];
	r_anim* _anims[128];
	unsigned int anim_count = 0;

	for(int i=0;i<r_res->anim_count;++i){
		int exists = 0;
		for(int j=0;j<anim_count;++j){
			if(_old_anim_ids[j] == r_res->anims[i].uid){
				exists = 1;
				break;
			}
		}

		if(exists){
			break;
		}

		if(anim_count < 128){
			r_anim* _anim = &r_res->anims[i];
			_old_anim_ids[anim_count] = r_res->anims[i].uid;
			_new_anim_ids[anim_count] = anim_count;
			_anims[anim_count] = _anim;
			++anim_count;	

			int updated_tex_id = -1;
			for(int j=0;j<sheet_count;++j){
				if(_old_sheet_ids[j] == _anim->sheet_id){
					updated_tex_id = j;
				}	
			}

			int count = 0;
			count += sprintf(str_buff,"%d,%d,%d,%d:", _new_anim_ids[i], updated_tex_id, _anim->frame_rate, _anim->frame_count); 

			for(int j=0;j<_anim->frame_count;++j){
				count += sprintf(str_buff, "%d,", _anim->frames[j]);
			}

			c_write_pref(fp, "anims", "anim", str_buff);
			memset(str_buff, 0, sizeof(char) * count);
		}	
	}
}
#endif
/*void c_write_table(const char* table, char** keys, char** values, int count){
  for(int i=0;i<count;++i){

  }
  }*/

unsigned char* c_get_file_contents(const char* fp, int* size){
	FILE* file;
	unsigned char* data = NULL;
	int count = 0;

	file = fopen(fp, "rt");
	if(!file){
		_e("Unable to open shader file: %s\n", fp);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	count = ftell(file);
	rewind(file);

	if(count > 0){
		data = malloc(sizeof(unsigned char)*(count));

		if(!data){
			_e("Unable to malloc size for shader: %d\n", (sizeof(unsigned char) * (count + 1)));
			fclose(file);
			return NULL;
		}
		count = fread(data, sizeof(unsigned char), count, file);
	}

	if(size)
		*size = count;

	fclose(file);
	return file;
}

int c_prefs_located(){
	return c_prefs_path;
}

int c_is_debug(){
	return conf_flags.debug;
}

int c_allow_render(){
	return conf_flags.render;
}

int c_allow_audio(){
	return conf_flags.audio;
}

int c_is_silent(){
	return conf_flags.verbose == -1;
}

int c_is_verbose(){
	return conf_flags.verbose;
}


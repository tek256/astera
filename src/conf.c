#include "conf.h"

#include <toml.c>
#include <string.h>
#include <unistd.h>

//tuning performance / memory usage
//#define C_MAP_SCALE_FACTOR_ADD
#define C_MAP_SCALE_FACTOR_MUL
#define C_MAP_SCALE_FACTOR 2

static const char* _raw;
static char err_buff[1024];

void c_alloc_map(c_map* map, int len){
	map->keys = malloc(sizeof(char*) * len);
	
	if(!map->keys){
		_e("Unable to allocate space for map: %d\n", (sizeof(char*) * len));
		return;
	}

	map->values = malloc(sizeof(void*) * len);	
	if(!map->values){
			_e("Unable to allocate space for map: %d\n", (sizeof(char*) * len));
			return; 
	}

	map->count = 0;
	map->capacity = len;
}

void c_parse_args(int argc, const char** argv){
	if(!argv || !argc){
		return;
	}

	conf_flags = (c_args){
		0, 1, 1, 0
	};

	while(1){
		char c;
		c = getopt(argc, argv, "srvad");

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

void c_add_to_map(c_map* map, char* key, void* value){
	if(!map){
		_e("No map passed to add to.\n");
		return;
	}	
	
	if(!key || !map){
		_e("No value/key passed to add to map.\n");
		return;
	}

	if(map->count == map->capacity){
		size_t scale;

#if  defined(C_MAP_SCALE_FACTOR_MUL)
		scale = sizeof(char*) * (map->capacity * C_MAP_SCALE_FACTOR);
#elif defined(C_MAP_SCALE_FACTOR_ADD)
		scale = sizeof(char*) * (map->capacity + C_MAP_SCALE_FACTOR); 
#else
		scale =sizeof(char*) * (map->capacity * 2);
#endif
		
		void* mem = malloc(scale);	
		void* mem2 = malloc(scale);

		memcpy(mem, map->keys, sizeof(char*) * map->capacity);
		memcpy(mem2, map->values, sizeof(char*) * map->capacity);
		
		map->capacity = scale / sizeof(char);

		free(map->keys);
		free(map->values);

		map->keys = mem;
		map->values = mem2;
	}	
	
	for(int i=0;i<map->count;++i){
		if(strcmp(map->keys[i], key) == 0){
			map->values[i] = (void*)value;
			return;
		}
	}

	map->keys[map->count] = (void*)key;
	map->values[map->count] = (void*)value;
	map->count ++;
}

//TODO yoinks, check me when sober
void c_parse_file(char* f, c_map* map){
	if(!f){
		_e("File required to parse for configurations.\n");
		return;
	}

	if(!map){
		_e("Please allocate a map to store all of the configurations in.\n");
		return;
	}

	toml_table_t* conf = toml_parse_file(f, err_buff, sizeof(char) * 1024);
	if(conf == 0){
		_e("Unable to parse toml file: %s\n", err_buff);
		memset(err_buff, 0, sizeof(char) * 1024);
		return; 
	}

	toml_table_t* curr = toml_table_in(conf, "render");

	if(curr == 0){
		_l("Unable to get table: render\n");
		return;	
	}

	int nkeys = toml_table_nkval(curr);
	for(int j=0;j<nkeys;++j){
		_raw = toml_raw_at(conf, j);
		if(_raw){
			const char* value;
			if(toml_rtos(_raw, value)){
				_e("Unable to convert to value: %s\n", _raw);
			}else{
				if(strcmp("width", value) == 0){
					int width;
					c_get_vali(_raw, 'd', &width);
					_l("Width: %d\n", width);
				}else if(strcmp("height", value) == 0){
					int height;
					c_get_vali(_raw, 'd', &height);
					_l("Height: %d\n", height);
				}else if(strcmp("windowed", value) == 0){
					int windowed;
					c_get_vali(_raw, 'd', &windowed);
					_l("Windowed: %d\n", windowed);
				}else if(strcmp("borderless", value) == 0){
					int borderless;
					c_get_vali(_raw, 'd', &borderless);
					_l("Borderless: %d\n", borderless);
				}else if(strcmp("refreshRate", value) == 0){
					int refreshRate;
					c_get_vali(_raw, 'd', &refreshRate);
					_l("Refresh rate: %d", refreshRate);
				}
			}

		}	
	}

	free(conf);
}

static void c_get_vali(const char* var, unsigned char type, void* ptr){
	int ival;
	double dval;
	char** sval;
	switch(type){
		case 'd':
				if(toml_rtoi(_raw, &ival)){
					_e("Unable to load toml int value: %s %s\n", var, err_buff);
					return;
				}
				*(int*)(ptr) = ival;
			break;
		case 'l':
				if(toml_rtod(_raw, &dval)){
					_e("Unable to load toml double value: %s %s\n", var, err_buff);
					return;
				}
				*(double*)(ptr) = dval;
			break;
		case 's':
				if(toml_rtos(_raw, &sval)){
					_e("Unable to convert toml string value: %s %s\n", var, err_buff);
					return;
				}
				*(char*)(ptr) = *sval;
			break;
	}
}

static void c_get_val(const char* var, toml_table_t* table, unsigned char type, void* ptr){
	switch(type){
		case 'd':
				if((_raw = toml_raw_in(table, var)) == 0){
					_e("Unable to load toml varabile: %s.\n", var);
					return;
				}

				int ival;
				if(toml_rtoi(_raw, &ival)){
					_e("Unable to load toml int value: %s %s\n", var, err_buff);
					return;
				}
				*(int*)(ptr) = ival;
			break;
		case 'l':
				if((_raw = toml_raw_in(table, var)) == 0){
					_e("Unable to load toml varabile: %s %s.\n", var, err_buff);
					return;
				}
				
				double dval;
				if(toml_rtod(_raw, &dval)){
					_e("Unable to load toml double value: %s %s\n", var, err_buff);
					return;
				}
				*(double*)(ptr) = dval;
			break;
		case 's':
				if((_raw = toml_raw_in(table, var)) == 0){
					_e("Unable to load toml variable: %s %s\n", var, err_buff);
					return;
				}
				
				char** sval;
				if(toml_rtos(_raw, &sval)){
					_e("Unable to convert toml string value: %s %s\n", var, err_buff);
					return;
				}
				*(char*)(ptr) = *sval;
			break;
	}
}

//TODO own memory implementation
void c_unload_map(c_map* map){
	if(!map){
		return;
	}

	free(map->keys);
	free(map->values);
	free(map);
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


#include "conf.h"

#include "debug.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <misc/ini.h>
#include <misc/ini.c>

static c_args _flags;

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
		i_add_binding(name, binding, type);
	}else{
		return 0;
	}

	return 1;
}

c_conf c_parse_file(char* fp, int prefs){
	c_conf conf = (c_conf){1280, 720, 0, 60, 1, 0, 100, 75, 50, NULL};

	if(ini_parse(fp, parse_handler, &conf) < 0){
		_e("Unable to open: %s\n", fp);
		return conf;
	}

	if(prefs){
		conf.path = fp;	
	}

	return conf;
}

c_conf c_defaults(){
	return (c_conf){1280, 720, 0, 60, 1, 0, 100, 75, 50, NULL};
}

void c_parse_args(int argc, char** argv){
	if(!argv || !argc){
		return;
	}

	_flags = (c_args){
		0, 1, 1, 0, NULL
	};

	while(1){
		char c = getopt(argc, argv, "srvadc:");

		if(c == -1){
			break;
		}

		switch(c){
			case 's':
				_flags.verbose = -1;
				break;
			case 'r':
				_flags.render = 0;
				break;
			case 'a':
				_flags.audio = 0;
				break;
			case 'v':
				_flags.verbose = 1;
				break;
			case 'd':
				_flags.debug = 1;
				break;
			case 'c':
				//TODO include optional config override
				//_flags.prefs = strdup(optarg);
				break;
		}
	}
}

void c_write_pref(const char* fp, const char* section, const char* key, const char* value){
	ini_write_kv(fp, section, key, value);
}

void c_write_table(const char* fp, const char* section, const char** keys, const char** values, int count){
	ini_write_table(fp, section, keys, values, count);
}


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

int c_has_prefs(){
	return _flags.prefs != 0;
}

char* c_get_pref_p(){
	return _flags.prefs;
}

int c_is_debug(){
	return _flags.debug;
}

int c_allow_render(){
	return _flags.render;
}

int c_allow_audio(){
	return _flags.audio;
}

int c_is_silent(){
	return _flags.verbose == -1;
}

int c_is_verbose(){
	return _flags.verbose;
}


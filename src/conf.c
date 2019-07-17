#include "conf.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ini.h>
#include <ini.c>


#define match(s,n) strcmp(section, s) == 0 && strcmp(name, n) == 0
static int parse_handler(void* user, const char* section, const char* name, const char* value){
	c_conf* c = (c_conf*)user;
	if(match("render", "width")){
		c->width = atoi(value);
	}else if(match("render", "height")){
		c->height = atoi(value);
	}else if(match("render", "fullscreen")){
		c->fullscreen = atoi(value);
	}else if(match("render", "borderless")){
		c->borderless = atoi(value);
	}else if(match("render", "refreshRate")){
		c->refreshRate = atoi(value);
	}else if(match("render", "vsync")){
		c->vsync = atoi(value);
	}

	if(match("audio", "master")){
		c->master = atoi(value);
	}else if(match("audio", "music")){
		c->music = atoi(value);	
	}else if(match("audio", "sfx")){
		c->sfx = atoi(value);	
	}else{
		return 0;
	}

	return 1;
}
/*
 *typedef struct {
	unsigned int width, height;
	unsigned int fullscreen, refreshRate;
	unsigned int vsync, borderless;
	unsigned int master;
	unsigned int music, sfx;	
} c_conf;


 */
c_conf c_parse_file(const char* fp){
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


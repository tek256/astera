#include "conf.h"

#include <unistd.h>
#include <string.h>

#include <ini.h>
#include <ini.c>

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


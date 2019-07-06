#include "conf.h"

#include <toml.h>
#include <string.h>

void c_parse_args(int argc, const char** argv){
	if(!argv || !argc){
		return;
	}
	
	//defaults
	conf_flags = (c_args){
		0, 1, 1, 0
	};

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

void c_parse_file(const char* f, c_map* map){
	if(!f){
		_e("File required to parse for configurations.\n");
		return;
	}

	if(!map){
		_e("Please allocate a map to store all of the configurations in.\n");
		return;
	}
	
	unsigned char* contents = f_get_file_contents(f);
	if(!contents){
		_e("No file contents for: %s\n", f);
		return;
	}

	//TOML impl
	toml_table_t* conf;
	
}

void c_unload_map(c_map* map){
	if(!map){
		return;
	}

	m_free(map);
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




#include "conf.h"

#include <string.h>
#include <unistd.h>

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


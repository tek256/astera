#include "fmgr.h"
#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <miniz/miniz.h>

#define READ_CHUNK_SIZE 4096

unsigned char* f_get_lib_file(f_lib* lib, const char* file_name){
	if(!lib){
		_e("No library passed to get file from.\n");
		return NULL;
	}
}

int f_lib_contains(f_lib* lib, const char* file_name){
	for(int i=0;i<lib->file_count;++i){
		if(strcmp(lib->files[i], file_name) == 0){
			return 1;
		}
	}	
	return 0;	
}

int f_open_lib(f_lib* lib){
	if(lib->open){
		return 1;
	}else{

	}
}

f_lib f_init_lib(const char* fp){
	if(!fp){
		return (f_lib){0};
	}

	f_lib lib;
	lib.file_count = 0;
	lib.path = fp;

	lib.open = 0;
	
	//TODO decompression

	return lib;
}

int f_close_lib(f_lib* lib){
	if(!lib->open){
		return 1;
	}

	return 1;
}

unsigned char* f_get_file_contents(const char* fp, size_t* size){
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
		data = malloc(sizeof(unsigned char)*(count+1));

		if(!data){
			_e("Unable to malloc size for shader: %d\n", (sizeof(unsigned char) * (count + 1)));
			fclose(file);
			return NULL;
		}
		count = fread(data, sizeof(unsigned char), count, file);
		data[count] = '\0';
	}
	
	if(size)
		*size = count+1;

	fclose(file);
	return file;
}

#include "fmgr.h"
#include "mem.h"

#include <stdio.h>
#include <stdlib.h>

#define READ_CHUNK_SIZE 4096

/*
 *FILE* file;
    unsigned char* data = NULL;
    int count = 0;
    file = fopen(filePath, "rt");
    if(file != NULL){
        fseek(file, 0, SEEK_END);
        count = ftell(file);
        rewind(file);

        if(count > 0){
            data = malloc(sizeof(unsigned char)*(count+1));
            count = fread(data, sizeof(unsigned char), count, file);
            data[count] = '\0';
        }

        fclose(file);
    }else{
      return 0;
}
 */

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

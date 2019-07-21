#include "fmgr.h"

#include <stdio.h>
#include <string.h>

unsigned char* f_get_file_contents(const char* fp, int* size){
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
		*size = count;

	fclose(file);
	return file;
}

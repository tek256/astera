#ifndef FMGR_H
#define FMGR_H

#include <unistd.h>
#include <miniz/miniz.h>

#define F_LIB_MAX 256

typedef struct {
	const char* files[F_LIB_MAX];
	unsigned int file_count;
	const char* path;	
	mz_zip_archive zip;
	
	void* _lib;
	int open: 1;
} f_lib;

unsigned char* f_get_lib_file(f_lib* lib, const char* file_name);
int f_lib_contains(f_lib* lib, const char* file_name);
int f_open_lib(f_lib* lib);
f_lib f_init_lib(const char* fp);
int f_close_lib(f_lib* lib);

unsigned char* f_get_file_contents(const char* fp, size_t* size);

#endif

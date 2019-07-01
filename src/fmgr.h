#ifndef FMGR_H
#define FMGR_H

#include <unistd.h>

typedef struct _f {
	FILE* ptr;
	size_t size;
	const char* name;
} _f;



#endif

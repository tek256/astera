#ifndef MEM_H
#define MEM_H

#define TAG_SIZE 8

#include <stddef.h>


typedef struct {
	void* blk;
	size_t size;
	char tag[BLOCK_TAG_SIZE];
} m_block;

typedef struct {
	m_block* blocks;
	int blocks;
} m_area;

#endif:

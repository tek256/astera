#ifndef MEM_H
#define MEM_H

#include <unistd.h>

//#define MEM_REUSE_ZONES
//#define MEM_SAFE

#define MEM_OFFSET_THRESHHOLD 64

typedef struct m_block m_block;
struct m_block {
	void* ptr;
	size_t size;
	int free;
	m_block* next;
	m_block* prev;
};

typedef struct {
	void* ptr;
	m_block* start;
	size_t size;
} m_zone;

typedef struct {
	m_zone* zone;
	int blocks;
	int used;
	int free;
	size_t free_amnt;
	size_t used_amnt;
} m_zoneinfo;

typedef struct {
	unsigned int size : 7;
	unsigned int free : 1;
} m_tag;

m_zone* ctx;

m_zone m_init_zone(size_t size);
void   m_free_zone(m_zone* zone);

//move to context for allocation & management 
void m_sw_ctx(m_zone* zone);

void m_update(m_zoneinfo* info);

m_block* m_alloc(size_t s);
m_block* m_free(void* ptr);

#endif

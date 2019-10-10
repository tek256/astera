#ifndef ASSET_H
#define ASSET_H

#include <stdio.h>

#include <zip/zip.h>

#include "platform.h"

#define MAX_ASSET_CACHE 256
#define MIN_ASSET_CACHE 16
#define ASSET_CACHE_GROWTH 16

//Max amount of map_t's
#define ASSET_MAX_MAPS 4

typedef struct {
	unsigned int uid;
	unsigned char* data; 
	unsigned int data_length;
	const char* name;	
} asset_t;

typedef struct {
	asset_t* assets;
	unsigned int count;	
	unsigned int capacity;
	const char* name;
} asset_map_t;

typedef struct {
	asset_t** to_save;	
	u16 save_count;
	u16 save_capacity;
} asset_flags_t;

typedef struct {
	const char* name;
	unsigned char* data;
	unsigned int length;
	asset_map_t* map_to;
	int filled : 1;
} asset_req_t;

typedef struct {
	const char* archive;
	asset_req_t* reqs;
	int count;
	int capacity;
} asset_req_stack_t; 

//leave these as pointers for when they're allocated
static asset_map_t* asset_maps[ASSET_MAX_MAPS];
static int asset_map_count = 0;

static asset_flags_t asset_flags;
static asset_req_stack_t req_stack;

void asset_init();

asset_t asset_load(const char* archive, const char* filename);
void asset_free(asset_t asset);

asset_t* asset_to_map(asset_t asset, asset_map_t* map);
asset_req_t* asset_request(const char* archive, const char* filename, asset_map_t* map_to);

void asset_pop_stack();
void asset_clear_stack();

asset_map_t* asset_create_map(const char* name, unsigned int capacity);
void asset_destroy_cache(asset_map_t* map); 

#endif
